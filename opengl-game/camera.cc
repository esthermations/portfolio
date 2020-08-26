#include <math.h>
#include <stdio.h>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.hh"
#include "globals.hh"
#include "macros.hh"

/**
   Ensure the camera is pointed towards the car.
 */
void
Camera::update()
{
	using namespace glm;

	this->target = g_car->position;
	this->view_matrix = lookAt(position, target, vec3(0, 1, 0));
}
