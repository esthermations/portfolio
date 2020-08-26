#ifndef _GLFW_CALLBACKS_HH_
#define _GLFW_CALLBACKS_HH_

#include <GLFW/glfw3.h>

namespace glfw_callbacks
{

void key_pressed(GLFWwindow *window, int key, int scancode, int action, int mods);
void window_resized(GLFWwindow *window, int x, int y);
void mouse_moved(GLFWwindow *window, double x, double y);
void mouse_button_pressed(GLFWwindow *window, int button, int action, int mods);
void error(int error, char const *description);

}

#endif // callbacks.hh
