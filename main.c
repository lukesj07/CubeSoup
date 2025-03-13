#include "geometry.h"
#include "linear.h"
#include "video.h"
#include "render.h"

int main() {

    // init matrices
    Matrix* rot_matrix = matrix_new(3, 3);
    const double rot_arr[] = {
    cos(0.01), -sin(0.01), 0,
        cos(0.01)*sin(0.01), cos(0.01)*cos(0.01), -sin(0.01),
        sin(0.01)*sin(0.01), cos(0.01)*sin(0.01), cos(0.01)
    };
    matrix_init(rot_matrix, rot_arr);


    Matrix* model_matrix = matrix_new(3, 3);
    const double identity3x3[] = {
        1, 0, 0,
        0, 1, 0,
        0, 0, 1,
    };
    matrix_init(model_matrix, identity3x3);


    // make projection matrix
    const double f = 1/tan(FOV_DEG * M_PI / 360.0);
    const double q = ZFAR / (ZFAR - ZNEAR);
    Matrix* proj_matrix = matrix_new(3, 4);
    const double proj_arr[] = {
        ASPECT_RATIO * f, 0, 0, 0,
                       0, f, 0, 0,
                       0, 0, q, -ZNEAR * q
    };
    matrix_init(proj_matrix, proj_arr);

    Matrix* camera_pos = matrix_new(3, 1); // malloc auto initializes to 0

    // make vertices
    Matrix* coord_vecs[8];
    for (int i = 0; i < 8; i++) {
        const double coord[] = {i & 1, (i & 2) >> 1, (i & 4) >> 2};
        coord_vecs[i] = matrix_new(3, 1);
        matrix_init(coord_vecs[i], coord);
    }

    // make mesh
    const int triangle_indices[][3] = {
        // south
        {0, 2, 3}, {0, 3, 1},
        // east
        {1, 3, 7}, {1, 7, 5},
        // north
        {5, 7, 6}, {5, 6, 4},
        // wes
        {4, 6, 2}, {4, 2, 0},
        // top
        {2, 6, 7}, {2, 7, 3},
        // bottom
        {1, 5, 4}, {1, 4, 0}
    };
    
    Matrix* rgb[] = {matrix_new(3, 1), matrix_new(3, 1), matrix_new(3, 1)};
    const double rgb_arr[][3] = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}};
    for (int i = 0; i < 3; i++) {
        matrix_init(rgb[i], rgb_arr[i]);
    }

    Mesh* cube_mesh = mesh_new(12);
    for (int i = 0; i < cube_mesh->num_triangles; i++) {
        Matrix* vertices[3];
        for (int j = 0; j < 3; j++) {
            vertices[j] = coord_vecs[triangle_indices[i][j]];
        }
        //Triangle* tri = triangle_new(vertices, (Matrix*[]){rgb[2], rgb[2], rgb[2]});
        Triangle* tri = triangle_new(vertices, rgb);
        mesh_set(cube_mesh, i, tri);
        free(tri); // triangle gets copied so must be freed
    }


    VideoHandler handler = {
        .window = NULL,
        .renderer = NULL
    };

    if (video_init(&handler)) {
        video_cleanup(&handler);
        return 1;
    }

    SDL_SetRenderDrawColor(handler.renderer, 0, 0, 0, 255);
    int running = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_ESCAPE:
                            running = 0;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
        SDL_RenderClear(handler.renderer);
        // draw
        
        Matrix* new_model = matrix_mult(rot_matrix, model_matrix);
        free_matrix(model_matrix);
        model_matrix = new_model;

        for (int i = 0; i < cube_mesh->num_triangles; i++) {
            Matrix* rotated_verts[3];
            for (int j = 0; j < 3; j++) {
                rotated_verts[j] = matrix_mult(model_matrix, cube_mesh->tris[i].vertices[j]);
                matrix_set(rotated_verts[j], 2, 0, matrix_get(rotated_verts[j], 2, 0) + 3.0);
            }

            Matrix* edge_a = matrix_subtract(rotated_verts[0], rotated_verts[1]);
            Matrix* edge_b = matrix_subtract(rotated_verts[2], rotated_verts[1]);
            Matrix* cross_prod = cross_mult(edge_a, edge_b);

            Matrix* camera_to_point = matrix_subtract(rotated_verts[0], camera_pos);
            if (dot_mult(cross_prod, camera_to_point) <= 0.0) {
                free_matrices(rotated_verts, 3);
                free_matrices((Matrix*[]) {edge_a, edge_b, cross_prod, camera_to_point}, 4);
                continue;
            }

            Matrix* light_vec = matrix_new(3, 1);
            const double light_arr[] = {0, 0, 1};
            matrix_init(light_vec, light_arr);
            matrix_normalize(light_vec);
            matrix_normalize(cross_prod);
            
            const double light_factor = dot_mult(cross_prod, light_vec);
            
            Matrix* proj_results[3];
            for (int j = 0; j < 3; j++) {
                Matrix* homog = matrix_new(4, 1);
                matrix_set(homog, 0, 0, matrix_get(rotated_verts[j], 0, 0));
                matrix_set(homog, 1, 0, matrix_get(rotated_verts[j], 1, 0));
                matrix_set(homog, 2, 0, matrix_get(rotated_verts[j], 2, 0));
                matrix_set(homog, 3, 0, 1);


                const double z = matrix_get(homog, 2, 0);
                proj_results[j] = matrix_mult(proj_matrix, homog);
                if (z != 0) {
                    matrix_set(proj_results[j], 0, 0, matrix_get(proj_results[j], 0, 0) / z);
                    matrix_set(proj_results[j], 1, 0, matrix_get(proj_results[j], 1, 0) / z);
                    matrix_set(proj_results[j], 2, 0, matrix_get(proj_results[j], 2, 0) / z);
                }

                matrix_set(proj_results[j], 0, 0, (matrix_get(proj_results[j], 0, 0) + 1) * 0.5 * SCREEN_WIDTH);
                matrix_set(proj_results[j], 1, 0, (matrix_get(proj_results[j], 1, 0) + 1) * 0.5 * SCREEN_HEIGHT);

                free_matrix(homog);
            }
            /* SDL_RenderDrawLine(handler.renderer, matrix_get(proj_results[0], 0, 0), matrix_get(proj_results[0], 1, 0),
                               matrix_get(proj_results[1], 0, 0), matrix_get(proj_results[1], 1, 0));
            SDL_RenderDrawLine(handler.renderer, matrix_get(proj_results[1], 0, 0), matrix_get(proj_results[1], 1, 0),
                               matrix_get(proj_results[2], 0, 0), matrix_get(proj_results[2], 1, 0));
            SDL_RenderDrawLine(handler.renderer, matrix_get(proj_results[2], 0, 0), matrix_get(proj_results[2], 1, 0),
                               matrix_get(proj_results[0], 0, 0), matrix_get(proj_results[0], 1, 0));
            */

            Triangle* proj_tri = triangle_new(proj_results, cube_mesh->tris[i].colors);
            draw_triangle(handler.renderer, proj_tri, light_factor);
            free(proj_tri);

            free_matrices(proj_results, 3);
            free_matrices(rotated_verts, 3);
            free_matrices((Matrix*[]) {edge_a, edge_b, cross_prod, camera_to_point, light_vec}, 5);
        }
        SDL_RenderPresent(handler.renderer);
        SDL_Delay(16);
    }

    free_mesh(cube_mesh);
    free_matrices(coord_vecs, 8);
    free_matrices(rgb, 3);
    free_matrix(rot_matrix);
    free_matrix(proj_matrix);
    free_matrix(camera_pos);
    video_cleanup(&handler);
    return 0;
}
// TODO: add lighting
