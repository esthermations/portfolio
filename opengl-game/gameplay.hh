#ifndef _GAMEPLAY_HH_
#define _GAMEPLAY_HH_

#include <glm/glm.hpp>

#define GAME_MAX_OBSTACLES 3

#define GAME_LEFTMOST_LANE_NUMBER -1
#define GAME_RIGHTMOST_LANE_NUMBER 1
#define GAME_NUM_LANES                                                         \
	(1 + GAME_RIGHTMOST_LANE_NUMBER - GAME_LEFTMOST_LANE_NUMBER)

#define IS_VALID_LANE(number)                                                  \
	(number >= GAME_LEFTMOST_LANE_NUMBER                                   \
	 && number <= GAME_RIGHTMOST_LANE_NUMBER)


class Entity {
public:
	glm::vec3 position   = glm::vec3(0, 0, 0);
	glm::vec3 scale      = glm::vec3(1, 1, 1);
	glm::vec3 rotation   = glm::vec3(0, 0, 0);
	glm::vec3 dimensions = glm::vec3(0, 0, 0);

	void move_left(void);
	void move_right(void);
	glm::vec3 get_scaled_dimensions(void);
};


/**

   Both cars and the obstacles must spawn in a lane.

   The lane should be at least as wide as the widest thing that will be in it.

   There will be GAME_NUM_LANES lanes.

   We probably want to remember which lane something is in, or we could just use
   the Model.position.x variable.

 */

void spawn_obstacle(int lane);
void advance_entity(Entity &e, float amount);
bool entities_collide(Entity &m1, Entity &m2);
void clean_up_invisible_obstacles(void);
bool entity_is_off_screen(Entity &e);

#endif // _GAMEPLAY_HH_
