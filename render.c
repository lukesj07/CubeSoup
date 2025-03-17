#include "render.h"

/**
 * Calculates the edge function for three vertices.
 * A positive value indicates a counter-clockwise winding.
 * A negative value indicates a clockwise winding. (change eventually to reduce redundant calcs, reuuse coefficients)
 */
static inline double edge_function(const Matrix* a, const Matrix* b, const Matrix* c) {
    double ax = matrix_get(a, 0, 0), ay = matrix_get(a, 1, 0);
    double bx = matrix_get(b, 0, 0), by = matrix_get(b, 1, 0);
    double cx = matrix_get(c, 0, 0), cy = matrix_get(c, 1, 0);
    
    return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}

/**
 * Draws a triangle with color interpolation using barycentric coordinates.
 */
void draw_triangle(SDL_Renderer* renderer, const Triangle* tri, const double light_factor) {
    const Matrix* a = tri->vertices[0];
    const Matrix* b = tri->vertices[1];
    const Matrix* c = tri->vertices[2];
    
    // Validate input colors
    for (int i = 0; i < 3; i++) {
        if (tri->colors[i] == NULL || tri->colors[i]->rows != 3 || tri->colors[i]->cols != 1) {
            fprintf(stderr, "Error: Invalid color matrix for triangle\n");
            return;
        }
    }
    
    // Calculate triangle area (using edge function)
    const double abc = edge_function(a, b, c);
    
    // Ensure triangle has clockwise winding (negative area)
    if (abc > 0) {
        fprintf(stderr, "Warning: Triangle has counter-clockwise winding, skipping\n");
        return;
    }
    
    // Cache vertex coordinates
    double ax = matrix_get(a, 0, 0), ay = matrix_get(a, 1, 0);
    double bx = matrix_get(b, 0, 0), by = matrix_get(b, 1, 0);
    double cx = matrix_get(c, 0, 0), cy = matrix_get(c, 1, 0);
    
    // Get bounding box for the triangle. PROBLEMATIC, VISUAL GLITCHES
    int min_x = (int)fmin(fmin(ax, bx), cx);
    int min_y = (int)fmin(fmin(ay, by), cy);
    int max_x = (int)fmax(fmax(ax, bx), cx);
    int max_y = (int)fmax(fmax(ay, by), cy);
    
    // Save current renderer color
    Uint8 oldr, oldg, oldb, olda;
    SDL_GetRenderDrawColor(renderer, &oldr, &oldg, &oldb, &olda);
    
    // Create point matrix for testing
    Matrix* p = matrix_new(2, 1);
    
    // Find top vertex as seed point
    int top_vertex_idx = 0;
    double top_y = ay;
    
    if (by < top_y) {
        top_y = by;
        top_vertex_idx = 1;
    }
    
    if (cy < top_y) {
        top_y = cy;
        top_vertex_idx = 2;
    }
    
    int seed_x = (int)matrix_get(tri->vertices[top_vertex_idx], 0, 0);
    
    // Scanline rendering
    for (int y = min_y; y <= max_y; y++) {
        matrix_set(p, 1, 0, y);
        
        // Start with the seed x-position
        int x = seed_x;
        matrix_set(p, 0, 0, x);
        
        // Check if seed point is inside the triangle
        double abp = edge_function(a, b, p);
        double bcp = edge_function(b, c, p);
        double cap = edge_function(c, a, p);
        
        if (!(abp <= 0 && bcp <= 0 && cap <= 0)) {
            // Seed not in triangle, search for a point inside
            for (x = min_x; x <= max_x; x++) {
                matrix_set(p, 0, 0, x);
                abp = edge_function(a, b, p);
                bcp = edge_function(b, c, p);
                cap = edge_function(c, a, p);
                if (abp <= 0 && bcp <= 0 && cap <= 0) {
                    break;
                }
            }
        }
        
        // Calculate color at current point using barycentric coordinates
        const double bc_a = bcp / abc;
        const double bc_b = cap / abc;
        const double bc_c = abp / abc;
        
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
        
        // Find leftmost point
        int left_x = x;
        while (left_x >= min_x) {
            matrix_set(p, 0, 0, left_x);
            abp = edge_function(a, b, p);
            bcp = edge_function(b, c, p);
            cap = edge_function(c, a, p);
            
            if (abp <= 0 && bcp <= 0 && cap <= 0) {
                // Calculate color using barycentric coordinates
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
        
        // Find rightmost point
        int right_x = x;
        while (right_x <= max_x) {
            matrix_set(p, 0, 0, right_x);
            abp = edge_function(a, b, p);
            bcp = edge_function(b, c, p);
            cap = edge_function(c, a, p);
            
            if (abp <= 0 && bcp <= 0 && cap <= 0) {
                // Calculate color using barycentric coordinates
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
        
        // Update seed for next scanline (middle of current span)
        seed_x = (left_x + right_x) / 2;
    }
    
    // Restore renderer color
    SDL_SetRenderDrawColor(renderer, oldr, oldg, oldb, olda);
    free_matrix(p);
}
