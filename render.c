#include "render.h"

double edge_function(const Matrix* a, const Matrix* b, const Matrix* c) {
    return ((matrix_get(b, 0, 0) - matrix_get(a, 0, 0))
        * (matrix_get(c, 1, 0) - matrix_get(a, 1, 0)))
        - ((matrix_get(b, 1, 0) - matrix_get(a, 1, 0))
        * (matrix_get(c, 0, 0) - matrix_get(a, 0, 0)));
}


void draw_triangle(SDL_Renderer* renderer, const Triangle* tri, const double light_factor) {
    const Matrix* a = tri->vertices[0];
    const Matrix* b = tri->vertices[1];
    const Matrix* c = tri->vertices[2];
    
    for (int i = 0; i < 3; i++) {
        if (tri->colors[i] == NULL || 
            tri->colors[i]->rows != 3 || 
            tri->colors[i]->cols != 1) {
            fprintf(stderr, "Error: Invalid color matrix for triangle\n");
            return;
        }
    }
    
    const double abc = edge_function(a, b, c);
    
    if (abc > 0) {
        fprintf(stderr, "Warning: Triangle has counter-clockwise winding, skipping\n");
        return;
    }
    
    int min_x = (int)fmin(fmin(matrix_get(a, 0, 0), matrix_get(b, 0, 0)), matrix_get(c, 0, 0));
    int min_y = (int)fmin(fmin(matrix_get(a, 1, 0), matrix_get(b, 1, 0)), matrix_get(c, 1, 0));
    int max_x = (int)fmax(fmax(matrix_get(a, 0, 0), matrix_get(b, 0, 0)), matrix_get(c, 0, 0));
    int max_y = (int)fmax(fmax(matrix_get(a, 1, 0), matrix_get(b, 1, 0)), matrix_get(c, 1, 0));
    
    Uint8 oldr, oldg, oldb, olda;
    SDL_GetRenderDrawColor(renderer, &oldr, &oldg, &oldb, &olda);
    
    Matrix* p = matrix_new(2, 1);
    
    int top_vertex_idx = 0;
    double top_y = matrix_get(a, 1, 0);
    
    if (matrix_get(b, 1, 0) < top_y) {
        top_y = matrix_get(b, 1, 0);
        top_vertex_idx = 1;
    }
    
    if (matrix_get(c, 1, 0) < top_y) {
        top_y = matrix_get(c, 1, 0);
        top_vertex_idx = 2;
    }
    
    int seed_x = (int)matrix_get(tri->vertices[top_vertex_idx], 0, 0);
    
    for (int y = min_y; y <= max_y; y++) {
        matrix_set(p, 1, 0, y);
        
        int x = seed_x;
        matrix_set(p, 0, 0, x);
        
        double abp = edge_function(a, b, p);
        double bcp = edge_function(b, c, p);
        double cap = edge_function(c, a, p);
        int in_tri = abp <= 0 && bcp <= 0 && cap <= 0;
        
        if (!in_tri) {
            // Ssed not in triangle, search for point inside
            for (x = min_x; x <= max_x; x++) {
                matrix_set(p, 0, 0, x);
                abp = edge_function(a, b, p);
                bcp = edge_function(b, c, p);
                cap = edge_function(c, a, p);
                if (abp <= 0 && bcp <= 0 && cap <= 0) {
                    in_tri = 1;
                    break;
                }
            }
            
            if (!in_tri) continue;  // no points in triangle for this scanline (should be impossible..)
        }
        
        // draw
        const double bc_a = bcp / abc;
        const double bc_b = cap / abc;
        const double bc_c = abp / abc;
        
        // barycentric coordinates
        Uint8 red = (Uint8)((matrix_get(tri->colors[0], 0, 0) * bc_a
                       + matrix_get(tri->colors[1], 0, 0) * bc_b
                       + matrix_get(tri->colors[2], 0, 0) * bc_c) * light_factor);
        Uint8 green = (Uint8)((matrix_get(tri->colors[0], 1, 0) * bc_a
                       + matrix_get(tri->colors[1], 1, 0) * bc_b
                       + matrix_get(tri->colors[2], 1, 0) * bc_c) * light_factor);
        Uint8 blue = (Uint8)((matrix_get(tri->colors[0], 2, 0) * bc_a
                       + matrix_get(tri->colors[1], 2, 0) * bc_b
                       + matrix_get(tri->colors[2], 2, 0) * bc_c) * light_factor);
        
        SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
        SDL_RenderDrawPoint(renderer, x, y);
        
        // find left
        int left_x = x - 1;
        while (left_x >= min_x) {
            matrix_set(p, 0, 0, left_x);
            abp = edge_function(a, b, p);
            bcp = edge_function(b, c, p);
            cap = edge_function(c, a, p);
            
            if (abp <= 0 && bcp <= 0 && cap <= 0) {
                red = (Uint8)((matrix_get(tri->colors[0], 0, 0) * bcp / abc
                           + matrix_get(tri->colors[1], 0, 0) * cap / abc
                           + matrix_get(tri->colors[2], 0, 0) * abp / abc) * light_factor);
                green = (Uint8)((matrix_get(tri->colors[0], 1, 0) * bcp / abc
                           + matrix_get(tri->colors[1], 1, 0) * cap / abc
                           + matrix_get(tri->colors[2], 1, 0) * abp / abc) * light_factor);
                blue = (Uint8)((matrix_get(tri->colors[0], 2, 0) * bcp / abc
                           + matrix_get(tri->colors[1], 2, 0) * cap / abc
                           + matrix_get(tri->colors[2], 2, 0) * abp / abc) * light_factor);
                
                SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
                SDL_RenderDrawPoint(renderer, left_x, y);
                left_x--;
            } else {
                break;
            }
        }
        
        // find right
        int right_x = x + 1;
        while (right_x <= max_x) {
            matrix_set(p, 0, 0, right_x);
            abp = edge_function(a, b, p);
            bcp = edge_function(b, c, p);
            cap = edge_function(c, a, p);
            
            if (abp <= 0 && bcp <= 0 && cap <= 0) {
                red = (Uint8)((matrix_get(tri->colors[0], 0, 0) * bcp / abc
                           + matrix_get(tri->colors[1], 0, 0) * cap / abc
                           + matrix_get(tri->colors[2], 0, 0) * abp / abc) * light_factor);
                green = (Uint8)((matrix_get(tri->colors[0], 1, 0) * bcp / abc
                           + matrix_get(tri->colors[1], 1, 0) * cap / abc
                           + matrix_get(tri->colors[2], 1, 0) * abp / abc) * light_factor);
                blue = (Uint8)((matrix_get(tri->colors[0], 2, 0) * bcp / abc
                           + matrix_get(tri->colors[1], 2, 0) * cap / abc
                           + matrix_get(tri->colors[2], 2, 0) * abp / abc) * light_factor);
                
                SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
                SDL_RenderDrawPoint(renderer, right_x, y);
                right_x++;
            } else {
                break;
            }
        }
        
        seed_x = (left_x + 1 + right_x - 1) / 2;
    }
    
    SDL_SetRenderDrawColor(renderer, oldr, oldg, oldb, olda);
    free_matrix(p);
}
