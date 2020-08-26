#ifndef _OBJ_UTILS_HH_
#define _OBJ_UTILS_HH_

#include "texture_utils.hh"
#include <glm/glm.hpp>

#include "external_files/tiny_obj_loader.h"

struct Material {

        std::string name;

	glm::vec3 ambient_colour  = glm::vec3(0, 0, 0);
	glm::vec3 diffuse_colour  = glm::vec3(1, 0, 1);
	glm::vec3 specular_colour = glm::vec3(1, 1, 1);

	int first_vertex_index = -1;
	int num_vertices       = -1;

	GLuint diffuse  = 0;
	GLuint specular = 0;

	float shininess = 8.0;

	Material(){};
};

/**
   A struct containing all relevant data for constructing a model. Since we
   probably want this data on the GPU rather than in RAM, this struct should be
   used to construct a model as soon as possible.
*/
struct Obj_Data {
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> texcoords;

	std::vector<float> unique_vertices;
	std::vector<float> unique_normals;
	std::vector<float> unique_texcoords;

	std::vector<tinyobj::material_t> tinyobj_materials;
	std::vector<Material>            materials;

	GLuint vao_id;

	/* The only reasonable way to construct this is with an obj file. */
	Obj_Data(){};
	Obj_Data(char const *path_to_obj_file);
};

#endif // _OBJ_UTILS_HH_
