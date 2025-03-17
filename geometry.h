#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stdio.h>
#include <stdlib.h>
#include "linear.h"

typedef struct {
    Matrix* vertices[3];
    Matrix* colors[3];
} Triangle;

typedef struct {
    int num_triangles;
    Triangle* tris;
} Mesh;

Triangle* triangle_new(Matrix* vertices[], Matrix* colors[]);

Mesh* mesh_new(int num_triangles);
inline void mesh_set(Mesh* mesh, int index, Triangle* tri);
inline void free_mesh(Mesh* mesh);

#endif // ! GEOMETRY_H
