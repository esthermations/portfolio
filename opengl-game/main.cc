#include <float.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.hh"
#include "file_utils.hh"
#include "gameplay.hh"
#include "glfw_callbacks.hh"
#include "globals.hh"
#include "macros.hh"
#include "model.hh"
#include "mouse.hh"
#include "obj_utils.hh"
#include "shader_utils.hh"
#include "terrain.hh"

int
main(int argc, char *argv[])
{

	if (argc != 1) DEBUGMSG("This program takes no arguments. Ignoring.\n");

	{
		char *argv0 = strdup(argv[0]);
		char *dir   = dirname(argv0);
		chdir_or_die(dir);
		free(argv0);
	}

	srand(time(NULL));

	using namespace glm;

	g_shader              = -1;
	g_camera              = NULL;
	g_mouse               = NULL;
	g_window_width        = 1280;
	g_window_height       = 720;
	g_fovy                = (float) M_PI / 3.0;
	g_player_speed        = 0.01;
	g_player_speed_record = 0.01;

	for (int i = 0; i < GAME_MAX_OBSTACLES; ++i) {
		g_obstacle_is_live[i] = false;
	}

	glfwSetErrorCallback(glfw_callbacks::error);
	if (!glfwInit()) DIE_HORRIBLY("Failed to initialise GLFW.");

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	g_window = glfwCreateWindow(
	    g_window_width, g_window_height,
	    "a1667700 Assignment 3 Part 2: Oncoming Traffic Simulator", NULL,
	    NULL);

	if (!g_window) DIE_HORRIBLY("Failed to create a window.");

	glfwMakeContextCurrent(g_window);
	glfwSwapInterval(1);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) DIE_HORRIBLY("Failed to initialize GLEW.");

	g_shader = load_shader("./vert.glsl", "./frag.glsl");
	if (g_shader == 0) DIE_HORRIBLY("Failed to load shader.");

	glClearColor(0.1, 0.1, 0.1, 1.0);

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glEnable(GL_DEPTH_TEST);

	// glEnable(GL_FOG);
	// glFogi(GL_FOG_MODE, GL_EXP);
	// glFogf(GL_FOG_START, 2);
	// glFogf(GL_FOG_END, 3);

	glUseProgram(g_shader);

	// Populate uniforms. These IDs won't change, so we get them upfront.

	// Vertex shader
	u_modelview    = glGetUniformLocation(g_shader, "u_modelview");
	u_normal_xform = glGetUniformLocation(g_shader, "u_normal_xform");
	u_projection   = glGetUniformLocation(g_shader, "u_projection");
	u_time         = glGetUniformLocation(g_shader, "u_time");
	u_procedural_height
	    = glGetUniformLocation(g_shader, "u_procedural_height");

	// Fragment shader
	u_diffuse_map     = glGetUniformLocation(g_shader, "u_diffuse_map");
	u_specular_map    = glGetUniformLocation(g_shader, "u_specular_map");
	u_mtl_ambient     = glGetUniformLocation(g_shader, "u_mtl_ambient");
	u_mtl_diffuse     = glGetUniformLocation(g_shader, "u_mtl_diffuse");
	u_mtl_specular    = glGetUniformLocation(g_shader, "u_mtl_specular");
	u_mtl_shininess   = glGetUniformLocation(g_shader, "u_mtl_shininess");
	u_light_position  = glGetUniformLocation(g_shader, "u_light_position");
	u_light_ambient   = glGetUniformLocation(g_shader, "u_light_ambient");
	u_light_diffuse   = glGetUniformLocation(g_shader, "u_light_diffuse");
	u_light_specular  = glGetUniformLocation(g_shader, "u_light_specular");

	{ // Set GLFW callbacks
		using namespace glfw_callbacks;
		glfwSetKeyCallback(g_window, key_pressed);
		glfwSetCursorPosCallback(g_window, mouse_moved);
		glfwSetMouseButtonCallback(g_window, mouse_button_pressed);
		glfwSetFramebufferSizeCallback(g_window, window_resized);
	}

	Mouse  mouse;
	Camera camera;

	g_mouse  = &mouse;
	g_camera = &camera;
	g_default_texture
	    = create_texture("./external_files/models/default_texture.png");
	g_program_t0 = glfwGetTime();

	Model car;
	Model obstacle;

	/* Create models from obj data. */
	chdir_or_die("external_files/models");

	{ // Open new scopes so the obj data gets freed ASAP
		Obj_Data car_obj("car-n.obj");
		car = Model(car_obj);

		car.position = vec3(+0.0, +0.0, -1.5);
		car.scale    = vec3(+0.1, +0.1, +0.1);
		car.rotation = vec3(+0.0, +1.0, +0.0);
		g_car        = &car;
	}

	{
		Obj_Data obstacle_obj("Barrel02.obj");
		obstacle = Model(obstacle_obj);

		obstacle.position = vec3(0);
		obstacle.scale    = vec3(6);
		obstacle.rotation = vec3(0);
		g_obstacle        = &obstacle;
	}

	chdir_or_die("../..");

	g_camera->target   = g_car->position;
	g_camera->position = vec3(0, 0.8, 0);
	camera.update();

	reset_projection();

	Model terrain[3];

	{
		terrain[0]         = generate_terrain_model(64);
		terrain[0].scale   = vec3(5);
		terrain[0].scale.z = 10;
		vec3 dim           = terrain[0].get_scaled_dimensions();

		terrain[0].position = g_car->position;
		terrain[0].position.x -= dim.x / 2;
		terrain[0].position.y = -0.05 - (car.scale.y * car.dimensions.y);
		terrain[0].position.z -= 5;
	}

	Material terrain_mtl;
	terrain_mtl.diffuse = create_texture("external_files/models/green.jpg");
	terrain_mtl.specular           = g_default_texture;
	terrain_mtl.first_vertex_index = 0;
	terrain_mtl.num_vertices       = terrain[0].vao.num_indices;
	terrain[0].materials.push_back(terrain_mtl);

	for (int i = 1; i < 3; ++i) {
		vec3 dim             = terrain[0].get_scaled_dimensions();
		terrain[i].vao       = terrain[0].vao;
		terrain[i].materials = terrain[0].materials;
		terrain[i].scale     = terrain[0].scale;

		terrain[i].position = terrain[i - 1].position;
		terrain[i].position.z -= dim.z;
	}

	Light headlight;
	headlight.position = car.position;
	headlight.position.z += car.dimensions.z * car.scale.z;
	// headlight.direction       = vec3(0, 1, 0); // Away from the screen.
	headlight.ambient_colour  = vec3(0.1, 0.1, 0.1); // Yellow.
	headlight.diffuse_colour  = vec3(0.7, 0.7, 0.5); // White.
	headlight.specular_colour = vec3(1, 1, 1);     // White.

	auto update_light_position = [](Light &l) {
		glUniform3fv(u_light_position, 1, value_ptr(l.position));
		glUniform3fv(u_light_ambient, 1, value_ptr(l.ambient_colour));
		glUniform3fv(u_light_diffuse, 1, value_ptr(l.diffuse_colour));
		glUniform3fv(u_light_specular, 1, value_ptr(l.specular_colour));
	};

	update_light_position(headlight);

	/*
	  Main program loop
	*/

	int previous_lane;

	while (!glfwWindowShouldClose(g_window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUniform1f(u_time, (float) glfwGetTime());

		clean_up_invisible_obstacles();

		g_player_speed += 0.001;

		if (g_player_speed > g_player_speed_record) {
			g_player_speed_record = g_player_speed;
		}

		// @Speedup: Slow!
		glUniform1i(u_procedural_height, true);

		for (int i = 0; i < 3; ++i) {
			if (entity_is_off_screen(terrain[i])) {
				int prev;
				if (i == 0) {
					prev = 2;
				} else {
					prev = (i - 1);
				}

				terrain[i].position.z
				    = terrain[prev].position.z - 10.0f;
			}

			advance_entity(terrain[i], g_player_speed);
			terrain[i].render();
		}

		glUniform1i(u_procedural_height, false);

		if (g_car_moved) {
			g_car_moved        = false;
			headlight.position = car.position;
			update_light_position(headlight);
		}

		// Only spawn obstacles sometimes
		int lane
		    = rand() % (25 * GAME_NUM_LANES) - (10 * GAME_NUM_LANES);
		if (IS_VALID_LANE(lane) && lane != previous_lane) {
			previous_lane = lane;
			spawn_obstacle(lane);
		}

		/* The player doesn't actually move -- everything else does. */
		for (int i = 0; i < GAME_MAX_OBSTACLES; ++i) {
			if (g_obstacle_is_live[i]) {
				advance_entity(g_obstacles[i], g_player_speed);
				g_obstacles[i].render();

				if (!entities_collide(*g_car, g_obstacles[i]))
					continue;

				g_obstacle_is_live[i] = false;
				g_player_speed        = 0.01;
				printf("Ouch!\n");
			}
		}

		g_car->render();

		glFlush();
		glfwSwapBuffers(g_window);
		glfwPollEvents();
	}

	// Multiply by 100 to make it sound more impressive...
	printf("Thanks for playing! Your top speed was %.1f.\n",
	       g_player_speed_record * 100);

	glfwDestroyWindow(g_window);
	glfwTerminate();

	return 0;
}
