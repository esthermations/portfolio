#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "file_utils.hh"
#include "globals.hh"
#include "macros.hh"
#include "model.hh"
#include "obj_utils.hh"
#include "texture_utils.hh"

/**
   Determine the bounding box of the given vertices. This is simply the vector
   from the most extreme negative coords in the model to the most positive.
*/
glm::vec3
find_dimensions(std::vector<float> v)
{

	glm::vec3 maxdim(0, 0, 0);
	glm::vec3 mindim(0, 0, 0);

	// Find maximum XYZ values
	for (size_t dim = 0; dim < 3; ++dim) {
		for (size_t i = dim; i < v.size(); i += 3) {
			if (v[i] > maxdim[dim]) maxdim[dim] = v[i];
		}
	}

	// Find minimum XYZ values
	for (size_t dim = 0; dim < 3; ++dim) {
		for (size_t i = dim; i < v.size(); i += 3) {
			if (v[i] > maxdim[dim]) maxdim[dim] = v[i];
		}
	}

	return maxdim - mindim;
}

/**
   Create a Model from the given Obj_Data.
*/
Model::Model(Obj_Data &obj)
{
	this->vao        = Vao(obj.vertices, obj.normals, obj.texcoords);
	this->dimensions = find_dimensions(obj.unique_vertices);
	this->materials  = obj.materials;
}

/**
   Render the model on-screen.
*/
void
Model::render()
{
	if (vao.id == 0) DIE_HORRIBLY("Invalid vao id.");
	if (materials.size() == 0) {
		DIE_HORRIBLY("Can't render a model with no materials.\n"
			     "Provide at least the default materials.\n");
	}

	using namespace glm;

	mat4 modelview(1.0f);
	modelview = modelview * g_camera->view_matrix;
	modelview = translate(modelview, this->position);
	modelview = glm::scale(modelview, this->scale);

	if (this->rotation.x != 0 || this->rotation.y != 0
	    || this->rotation.z != 0) {
		modelview = glm::rotate(modelview, (float) DEG2RAD(180.0f),
					this->rotation);
	}

	mat3 normal_xform = glm::transpose(glm::inverse(mat3(modelview)));

	// Send transformations for model and normals
	glUniformMatrix4fv(u_modelview, 1, false, value_ptr(modelview));
	glUniformMatrix3fv(u_normal_xform, 1, false, value_ptr(normal_xform));

	// Send texture info
	glUniform1i(u_diffuse_map, 0);
	glUniform1i(u_specular_map, 1);

	// Render vertices for each material separately.
	for (Material &mtl : this->materials) {
		// Send material uniform values
		glUniform3fv(u_mtl_ambient, 1,
			     glm::value_ptr(mtl.ambient_colour));
		glUniform3fv(u_mtl_diffuse, 1,
			     glm::value_ptr(mtl.diffuse_colour));
		glUniform3fv(u_mtl_specular, 1,
			     glm::value_ptr(mtl.specular_colour));
		glUniform1f(u_mtl_shininess, mtl.shininess);

		// Specify diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mtl.diffuse);

		// Specify texture map
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mtl.specular);

		if (mtl.num_vertices == -1) {
			mtl.num_vertices
			    = this->vao.num_indices - mtl.first_vertex_index;
		}

		// Draw the vertices associated with this material
		glBindVertexArray(vao.id);
		glDrawArrays(GL_TRIANGLES, mtl.first_vertex_index,
			     mtl.num_vertices);
	}

	// Unbind
	glBindVertexArray(0);
}

/**
   Create a VAO from the given vectors.
 */
Vao::Vao(std::vector<float> &vertices, std::vector<float> &normals,
	 std::vector<float> &texcoords)
{
	assert(vertices.size() != 0);
	assert(vertices.size() % 3 == 0);  // 3 floats per vertex
	assert(normals.size() % 3 == 0);   // 3 floats per normal
	assert(texcoords.size() % 2 == 0); // 2 floats per texcoord

	glGenVertexArrays(1, &id);
	glBindVertexArray(id);

	this->num_indices = vertices.size() / 3;

	GLuint buffers[3];
	glGenBuffers(3, buffers);

	auto buffer_to_gpu
	    = [](std::vector<float> &vec, GLuint buf, GLuint loc, GLuint vpi) {
		      /* buf = buffer,
			 loc = layout(location=X),
			 vpi = values per item */
		      if (vec.size() == 0) return;
		      glBindBuffer(GL_ARRAY_BUFFER, buf);
		      glBufferData(GL_ARRAY_BUFFER, vec.size() * sizeof(float),
				   &vec.at(0), GL_STATIC_DRAW);
		      glEnableVertexAttribArray(loc);
		      glVertexAttribPointer(loc, vpi, GL_FLOAT, false, 0, 0);
	      };

	buffer_to_gpu(vertices, buffers[0], 0, 3);
	buffer_to_gpu(normals, buffers[1], 1, 3);
	buffer_to_gpu(texcoords, buffers[2], 2, 2);

	// Unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
