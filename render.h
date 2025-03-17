#ifndef RENDER_H
#define RENDER_H

#include "geometry.h"
#include "linear.h"
#include <SDL2/SDL.h>

static inline double edge_function(const Matrix* a, const Matrix* b, const Matrix* c);
void draw_triangle(SDL_Renderer* renderer, const Triangle* tri, const double light_factor);

#endif // !RENDER_H
