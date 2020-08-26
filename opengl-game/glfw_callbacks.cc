#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glfw_callbacks.hh"
#include "gameplay.hh"
#include "globals.hh"
#include "macros.hh"
#include "mouse.hh"

/*
   These are namespaced so they have nice names and won't be used accidentally.
*/

namespace glfw_callbacks
{

void
key_pressed(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS) return;

	switch (mods) {

	case GLFW_MOD_SHIFT: {

		switch (key) {
		default: break;
		}

		break;
	}

	/* No modifiers. */
	default: {
		switch (key) {

		case GLFW_KEY_Q:
		case GLFW_KEY_ESCAPE: {
			glfwSetWindowShouldClose(g_window, true);
			break;
		}

		case GLFW_KEY_RIGHT: {
			g_car->move_right();
			break;
		}

		case GLFW_KEY_LEFT: {
			g_car->move_left();
			break;
		}

		default: break;
		}

		break;
	}
	}

}

void
window_resized(GLFWwindow *window, int x, int y)
{
	// Window resize event
	g_window_width  = x;
	g_window_height = y;
	reset_projection();
	glViewport(0, 0, x, y);
}

void
mouse_moved(GLFWwindow *window, double x, double y)
{
	g_mouse->update_position(glm::vec2((float) x, (float) y));
}

void
mouse_button_pressed(GLFWwindow *window, int button, int action, int mods)
{
	switch (button) {
	case GLFW_MOUSE_BUTTON_RIGHT:
		if (action == GLFW_PRESS) g_mouse->right_mouse_pressed = true;
		if (action == GLFW_RELEASE)
			g_mouse->right_mouse_pressed = false;
		break;
	case GLFW_MOUSE_BUTTON_LEFT:
		if (action == GLFW_PRESS) g_mouse->left_mouse_pressed   = true;
		if (action == GLFW_RELEASE) g_mouse->left_mouse_pressed = false;
		break;
	default: break;
	}
}

void
error(int error, char const *description)
{
	DEBUGMSG("GLFW error %d: %s\n", error, description);
}
}
