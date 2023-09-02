#pragma once
#include "sycl.h"
#include "Scene.h"
#include "kernel.h"

void copy_scene(dev_Scene* scene, dev_Scene* dev_scene, sycl::queue& q);

