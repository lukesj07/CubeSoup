#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stdio.h>
#include <stdlib.h>
#include "linear.h"

typedef struct {
    Matrix* vertices[3];
} Triangle;

typedef struct {
    int num_triangles;
    Triangle* tris;
} Mesh;

Triangle* triangle_new(Matrix* p1, Matrix* p2, Matrix* p3);

Mesh* mesh_new(int num_triangles);
void mesh_set(Mesh* mesh, int index, Triangle* tri);
void free_mesh(Mesh* mesh);

#endif // ! GEOMETRY_H
