#include "shader.h"

#define ASL_SHADE_CASE(z, n, text) \
    case n: asl_shade##n##_(Px, Py, Pz, Ix, Iy, Iz, Nx, Ny, Nz, Ngx, Ngy, Ngz, u, v, f, fps, Cr, Cg, Cb); break;

void asl_shade(int id, float Px, float Py, float Pz, float Ix, float Iy, float Iz, float Nx, float Ny, float Nz, float Ngx, float Ngy, float Ngz, float u, float v, float f, float fps, float& Cr, float& Cg, float& Cb) {
	switch (id) {
		BOOST_PP_REPEAT(MAX_SHADERS, ASL_SHADE_CASE, ~)
	}
}
