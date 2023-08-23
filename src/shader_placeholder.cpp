#include <CL/sycl.hpp>
#include "shader.h"

BOOST_PP_REPEAT(MAX_SHADERS, ASL_SHADE_DEF, ~)