#ifndef _MODEL_HH_
#define _MODEL_HH_

#include <OpenGL/gl.h>
#include <glm/glm.hpp>

#include "obj_utils.hh"
#include "gameplay.hh"

struct Vao {
	GLuint id          = 0;
	GLuint num_indices = 0;

	Vao(){};
	Vao(std::vector<float> &vertices, std::vector<float> &normals,
	    std::vector<float> &texcoords);
};

class Light : public Entity {
public:
	glm::vec3 direction;
	glm::vec3 ambient_colour;
	glm::vec3 diffuse_colour;
	glm::vec3 specular_colour;
};

class Model : public Entity {
public:
	Vao vao;

	std::vector<Material> materials;

	Model(){};
	Model(Obj_Data &obj);
	void render(void);
};

glm::vec3 find_dimensions(std::vector<float> vertices);

#endif // model.hh
