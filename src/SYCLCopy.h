#pragma once
#include "sycl.h"
#include "OslMaterial.hpp"
#include "Scene.h"
#include "kernel.h"

void copy_exp(Exp* exp, Exp* dev_exp, sycl::queue& q);
void copy_statement(Statement* sta, Statement* dev_sta, sycl::queue& q);
void copy_osl_material(OslMaterial* mat, OslMaterial* dev_mat, sycl::queue& q);
void copy_scene(dev_Scene* scene, dev_Scene* dev_scene, sycl::queue& q);

