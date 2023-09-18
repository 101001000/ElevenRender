#include "sycl.h"
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>

#define MAX_SHADERS 4

#define ASL_SHADE_DEC(z, n, text) \
    SYCL_EXTERNAL void asl_shade##n##_(float Px, float Py, float Pz, float Ix, float Iy, float Iz, float Nx, float Ny, float Nz, float Ngx, float Ngy, float Ngz, float u, float v, float f, float fps, float& Cr, float& Cg, float& Cb);

#define ASL_SHADE_DEF(z, n, text) \
    SYCL_EXTERNAL void asl_shade##n##_(float Px, float Py, float Pz, float Ix, float Iy, float Iz, float Nx, float Ny, float Nz, float Ngx, float Ngy, float Ngz, float u, float v, float f, float fps, float& Cr, float& Cg, float& Cb){Cr = 1; Cg = 1; Cb = 0;}

#define ASL_SHADE_NAME(z, n, text) \
    asl_shade##n##_,

BOOST_PP_REPEAT(MAX_SHADERS, ASL_SHADE_DEC, ~)

SYCL_EXTERNAL void asl_shade(int id, float Px, float Py, float Pz, float Ix, float Iy, float Iz, float Nx, float Ny, float Nz, float Ngx, float Ngy, float Ngz, float u, float v, float f, float fps, float& Cr, float& Cg, float& Cb);
