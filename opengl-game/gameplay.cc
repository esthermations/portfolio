#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>

#include "gameplay.hh"
#include "globals.hh"
#include "macros.hh"
#include "model.hh"

#define LANE_WIDTH (g_car->dimensions.x * g_car->scale.x * 2 + 0.2)
#define LEFTMOST_LANE_POSITION (GAME_LEFTMOST_LANE_NUMBER * LANE_WIDTH)
#define RIGHTMOST_LANE_POSITION (GAME_RIGHTMOST_LANE_NUMBER * LANE_WIDTH)

void
Entity::move_left(void)
{
	float lane_width = LANE_WIDTH;
	float newx       = this->position.x - lane_width;
	if (newx < LEFTMOST_LANE_POSITION) return;
	this->position.x -= lane_width;
	g_car_moved = true;
}

void
Entity::move_right(void)
{
	float lane_width = LANE_WIDTH;
	float newx       = this->position.x + lane_width;
	if (newx > RIGHTMOST_LANE_POSITION) return;
	this->position.x += lane_width;
	g_car_moved = true;
}

/**
   Return the actual dimensions of this entity in the game world, based on its
   @dimensions and @scale members.
 */
glm::vec3
Entity::get_scaled_dimensions(void)
{
	return this->dimensions * this->scale;
}

void
spawn_obstacle(int lane)
{
	assert(IS_VALID_LANE(lane));

	for (int i = 0; i < GAME_MAX_OBSTACLES; ++i) {
		if (g_obstacle_is_live[i]) continue;

		float lane_pos          = lane * LANE_WIDTH;
		g_obstacles[i]          = *g_obstacle;
		g_obstacles[i].position = glm::vec3(lane_pos, g_car->position.y,
						    -10 * g_car->dimensions.z);
		g_obstacle_is_live[i] = true;
		break; // We only spawn a single obstacle per call to this
		       // function.
	}
}

void
advance_entity(Entity &e, float amount)
{
	e.position.z += amount;
}

bool
entity_is_off_screen(Entity &e)
{
	glm::vec3 dim      = e.get_scaled_dimensions();
	float     z_extent = e.position.z - dim.z;
	return (z_extent > g_camera->position.z);
}

void
clean_up_invisible_obstacles(void)
{
	for (int i = 0; i < GAME_MAX_OBSTACLES; ++i) {
		if (g_obstacle_is_live[i]
		    && entity_is_off_screen(g_obstacles[i])) {
			g_obstacle_is_live[i] = false;
		}
	}
}

bool
entities_collide(Entity &m1, Entity &m2)
{
	using namespace glm;

	/* If they're not in the same lane, they didn't collide. */
	if (m1.position.x != m2.position.x) return false;

	/* All entities should be the same height. */
	if (m1.position.y != m2.position.y) return false;

	auto between = [](float near, float middle, float far) {
		return (near < middle && middle < far);
	};

	float m1_scaled_z = m1.scale.z * (m1.dimensions.z / 2);
	float m2_scaled_z = m2.scale.z * (m2.dimensions.z / 2);

	float m1_near = m1.position.z - m1_scaled_z;
	float m1_far  = m1.position.z + m1_scaled_z;
	float m2_near = m2.position.z - m2_scaled_z;
	float m2_far  = m2.position.z + m2_scaled_z;

	return (between(m2_near, m1_near, m2_far)
		|| between(m2_near, m1_far, m2_near)
		|| between(m1_near, m2_near, m1_far)
		|| between(m1_near, m2_far, m1_far));
}
