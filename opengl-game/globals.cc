#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.hh"
#include "gameplay.hh"
#include "model.hh"
#include "mouse.hh"

/*
  Global variables. Prefixed with g_ to make that fact clear.
*/

GLint       g_shader;
Camera *    g_camera;
Mouse *     g_mouse;
GLuint      g_default_texture;
GLFWwindow *g_window;
int         g_window_width;
int         g_window_height;
float       g_fovy;
double      g_program_t0;
Model *     g_car;
Model *     g_obstacle;
Model *     g_tree;
bool        g_car_moved;
float       g_player_speed;
float       g_player_speed_record;

bool  g_obstacle_is_live[GAME_MAX_OBSTACLES];
Model g_obstacles[GAME_MAX_OBSTACLES];

/*
  Global uniform IDs. Prefixed with u_ to make that fact clear.
*/

GLint u_modelview;    /// Transform for VAO vertices
GLint u_normal_xform; /// Transform for VAO normals
GLint u_time;
GLint u_procedural_height;

GLint u_diffuse_map;   /// Texture ID of diffuse map for VAO.
GLint u_specular_map;  /// Texture ID of specular map for VAO.
GLint u_projection;    /// Projection of screen-space onto screen.
GLint u_mtl_ambient;   /// Material ambient colour
GLint u_mtl_diffuse;   /// Material diffuse colour
GLint u_mtl_specular;  /// Material specular colour
GLint u_mtl_shininess; /// Shininess amount for VAO.
GLint u_light_position;
GLint u_light_ambient;
GLint u_light_diffuse;
GLint u_light_specular;

/**
   Set the projection matrix based on the dimensions of the program window. When
   the window is resized, this should be called.
*/
void
reset_projection(void)
{
	using namespace glm;
	float ratio = g_window_width / (float) g_window_height;
	mat4  proj  = perspective(g_fovy, ratio, 1.0f, 30.0f);
	glUniformMatrix4fv(u_projection, 1, false, value_ptr(proj));
}
