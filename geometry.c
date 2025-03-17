#include "geometry.h"

// points should be going clockwise. 3x1 matrices for vectors.
// freeing triangle does not free points (might be shared).
Triangle* triangle_new(Matrix* vertices[], Matrix* colors[]) {
    Triangle* tri = (Triangle*)malloc(sizeof(Triangle));
    if (tri == NULL) {
        fprintf(stderr, "Error allocating memory for triangle\n");
        return NULL;
    }

    for (int i = 0; i < 3; i++) {
        tri->vertices[i] = vertices[i];
        tri->colors[i] = colors[i];
    }
    return tri;
}

Mesh* mesh_new(int num_triangles) {
    Mesh* mesh = (Mesh*)malloc(sizeof(Mesh));
    if (mesh == NULL) {
        fprintf(stderr, "Error allocating memory for mesh\n");
        return NULL;
    }

    mesh->tris = (Triangle*)malloc(sizeof(Triangle) * num_triangles);
    if (mesh->tris == NULL) {
        fprintf(stderr, "Error allocating memory for triangles in mesh\n");
        free(mesh);
        return NULL;
    }

    mesh->num_triangles = num_triangles;

    return mesh;
}

inline void mesh_set(Mesh* mesh, int index, Triangle* tri) {
    if (mesh == NULL || tri == NULL) {
        fprintf(stderr, "Cannot call mesh_set on a null mesh/triangle\n");
        return;
    }
    if (index < 0 || index >= mesh->num_triangles) {
        fprintf(stderr, "Mesh index out of bounds: %d in an array of size %d\n", index, mesh->num_triangles);
        return;
    }

    mesh->tris[index] = *tri;
}

inline void free_mesh(Mesh* mesh) {
    free(mesh->tris);
    free(mesh);
}

// TODO: add color to triangle struct
// create a triangle normal function?
