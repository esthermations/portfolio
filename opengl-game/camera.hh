#ifndef _VIEWER_HH_
#define _VIEWER_HH_

#include <glm/glm.hpp>

#include "mouse.hh"

/**
 The base Viewer class stores a current view transformation. It is initialised
 by providing an initial camera position. Assumes we are always looking at the
 origin. It is updated in response to user input or it can be reset. Method
 orthogonaliseViewMtx() can be used to correct drift if needed.
*/
struct Camera {
	glm::mat4 view_matrix;
	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 up;
	glm::vec2 rotation;

	Camera() {};
	Camera(glm::vec3 eye, glm::vec3 target);

	void reset(void);
	void update(void);
};

#endif // _VIEWER_HH_
