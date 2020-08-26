#ifndef _GLOBALS_HH_
#define _GLOBALS_HH_

#include <GLFW/glfw3.h>

#include "camera.hh"
#include "gameplay.hh"
#include "model.hh"
#include "mouse.hh"

void reset_projection(void);

/*
  Global variables. Prefixed with g_ to make that fact clear.
*/

extern GLint       g_shader;
extern GLenum      g_rendering_mode;
extern Camera *    g_camera;
extern Mouse *     g_mouse;
extern GLuint      g_default_texture;
extern int         g_window_width;
extern int         g_window_height;
extern float       g_fovy;
extern GLFWwindow *g_window;
extern double      g_program_t0;
extern Model *     g_car;
extern Model *     g_obstacle;
extern Model *     g_tree;
extern bool        g_car_moved;
extern float       g_player_speed;
extern float       g_player_speed_record;

extern bool  g_obstacle_is_live[GAME_MAX_OBSTACLES];
extern Model g_obstacles[GAME_MAX_OBSTACLES];

/*
  Global uniform IDs. Prefixed with u_ to make that fact clear.
*/

extern GLint u_modelview;
extern GLint u_normal_xform;
extern GLint u_projection;
extern GLint u_time;
extern GLint u_procedural_height;

extern GLint u_diffuse_map;
extern GLint u_specular_map;
extern GLint u_mtl_ambient;
extern GLint u_mtl_diffuse;
extern GLint u_mtl_specular;
extern GLint u_mtl_shininess;
extern GLint u_light_position;
extern GLint u_light_ambient;
extern GLint u_light_diffuse;
extern GLint u_light_specular;

#endif // _GLOBALS_HH_
