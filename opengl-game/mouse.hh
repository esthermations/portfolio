#ifndef _MOUSE_HH_
#define _MOUSE_HH_

#include <glm/glm.hpp>

struct Mouse {
	bool  left_mouse_pressed = false;
	bool right_mouse_pressed = false;
	glm::vec2 previous;
	glm::vec2 delta;

	void update_position(glm::vec2 new_pos);
	glm::vec2 read_delta_and_reset(void);
};

#endif // mouse.hh
