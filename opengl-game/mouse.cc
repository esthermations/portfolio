#include <glm/glm.hpp>

#include "mouse.hh"
#include "macros.hh"

void
Mouse::update_position(glm::vec2 new_pos)
{
	this->delta += new_pos - this->previous;
	this->previous = new_pos;
}

glm::vec2
Mouse::read_delta_and_reset()
{
	glm::vec2 out = this->delta;
	this->delta = { 0, 0 };
	return out;
}
