#include <assert.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "file_utils.hh"
#include "globals.hh"
#include "macros.hh"
#include "obj_utils.hh"

#define VALUES_PER_VERTEX 3
#define VALUES_PER_NORMAL 3
#define VALUES_PER_TEXCOORD 2

#define TINYOBJLOADER_IMPLEMENTATION
#include "external_files/tiny_obj_loader.h"

/**
   Populate an Obj_Data struct from the given obj file.
 */
Obj_Data::Obj_Data(char const *path_to_obj_file)
{
	/* Set up callbacks. I'm defining them as lambdas since they're not
	   meant to be used anywhere else. */

	tinyobj::callback_t cb;

	cb.vertex_cb = [](void *user_data, float x, float y, float z, float w) {
		Obj_Data *obj = (Obj_Data *) user_data;
		obj->unique_vertices.push_back(x);
		obj->unique_vertices.push_back(y);
		obj->unique_vertices.push_back(z);
		// We don't care about the w-coord
	};

	cb.normal_cb = [](void *user_data, float x, float y, float z) {
		Obj_Data *obj = (Obj_Data *) user_data;
		obj->unique_normals.push_back(x);
		obj->unique_normals.push_back(y);
		obj->unique_normals.push_back(z);
	};

	cb.texcoord_cb = [](void *user_data, float x, float y, float z) {
		Obj_Data *obj = (Obj_Data *) user_data;
		obj->unique_texcoords.push_back(x);
		obj->unique_texcoords.push_back(y);
		// We don't care about the Z-coord -- our textures are 2D.
	};

	cb.index_cb = [](void *user_data, tinyobj::index_t indices[], //
			 int num_indices) {

		/*
		  Our strategy with indices is just to reconstruct the full list
		  of vertices, normals and texcoords. This is inefficient in
		  many ways, but skips any issues related to ensuring
		  vertex/normal/texcoord indices are all identical.

		  Until performance is a concern, I'm just sticking with the
		  simplest thing that could possibly work.
		*/

		Obj_Data *obj = (Obj_Data *) user_data;

		for (int i = 0; i < num_indices; i++) {
			auto vi = indices[i].vertex_index;
			auto ni = indices[i].normal_index;
			auto ti = indices[i].texcoord_index;

			if (vi < 0 || ni < 0 || ti < 0)
				DIE_HORRIBLY("The program tried to load an obj "
					     "file which used relative "
					     "indices. Those aren't "
					     "implemented yet.");
			/*
			  The vertex index comes straight from the OBJ file. OBJ
			  uses 1-based indexing, so we need to subtract 1. After
			  that, since v is a vector of floats and not a vector
			  of vertices, we need to multiply the index by 3 to get
			  the start of the appropriate vertex. (Or multiply by 2
			  for texcoords).
			*/

			if (vi != 0) {
				auto v  = &obj->vertices;
				auto uv = &obj->unique_vertices;
				v->push_back(uv->at(3 * (vi - 1) + 0));
				v->push_back(uv->at(3 * (vi - 1) + 1));
				v->push_back(uv->at(3 * (vi - 1) + 2));
			}

			if (ni != 0) {
				auto n  = &obj->normals;
				auto un = &obj->unique_normals;
				n->push_back(un->at(3 * (ni - 1) + 0));
				n->push_back(un->at(3 * (ni - 1) + 1));
				n->push_back(un->at(3 * (ni - 1) + 2));
			}

			if (ti != 0) {
				auto t  = &obj->texcoords;
				auto ut = &obj->unique_texcoords;
				t->push_back(ut->at(2 * (ti - 1) + 0));
				t->push_back(ut->at(2 * (ti - 1) + 1));
			}
		}
	};

	cb.mtllib_cb = [](void *user_data, tinyobj::material_t const *materials,
			  int num_materials) {
		Obj_Data *obj = (Obj_Data *) user_data;
		for (int i = 0; i < num_materials; i++) {
			if (materials[i].diffuse_texname.empty()) continue;

			obj->tinyobj_materials.push_back(materials[i]);
		}
	};

	cb.usemtl_cb = [](void *user_data, char const *name, int material_id) {
		Obj_Data *obj = (Obj_Data *) user_data;

		Material mtl;

		mtl.name = (std::string) name;

		mtl.first_vertex_index = (obj->vertices.size() / 3);

		mtl.num_vertices = -1; // Set later.

		if (material_id >= obj->tinyobj_materials.size()) {
			DEBUGMSG("Received a material ID for a material we "
				 "don't have. Doing nothing.\n");
			return;
		}

		// Brevity
		tinyobj::material_t &tmtl = obj->tinyobj_materials[material_id];

		mtl.diffuse
		    = tmtl.diffuse_texname.empty()
			  ? g_default_texture
			  : create_texture(tmtl.diffuse_texname.c_str());

		mtl.specular
		    = tmtl.specular_texname.empty()
			  ? g_default_texture
			  : create_texture(tmtl.specular_texname.c_str());

		for (int i = 0; i < 3; ++i) {
			mtl.ambient_colour[i]  = tmtl.ambient[i];
			mtl.diffuse_colour[i]  = tmtl.diffuse[i];
			mtl.specular_colour[i] = tmtl.specular[i];
		}

		mtl.shininess = tmtl.shininess;

		size_t last_mtl_index = obj->materials.size();
		if (last_mtl_index > 0) last_mtl_index -= 1;
		Material *last_mtl = &obj->materials[last_mtl_index];

		// Set the previous material's num_vertices, since we apparently
		// just finished with it.
		if (last_mtl != NULL) {
			last_mtl->num_vertices = (obj->vertices.size() / 3)
						 - last_mtl->first_vertex_index;
		}

		obj->materials.push_back(mtl);
	};

	cb.group_cb = [](void *user_data, char const *names[], int num_names) {
		if (!names) return;
		// DEBUGMSG("Ignoring `group %s` in mtl file.\n", names[0]);
	};

	cb.object_cb = [](void *user_data, char const *name) {
		if (!name) return;
		// DEBUGMSG("Ignoring `object %s` statement in mtl file.\n", name);
	};

	{ /* Construct obj_data from library calls */
		using namespace std;
		using namespace tinyobj;

		string   errstr;
		ifstream ifs(path_to_obj_file);
		if (ifs.fail())
			DIE_HORRIBLY("Couldn't open OBJ file %s.\n",
				     path_to_obj_file);

		MaterialFileReader mtl_reader("./");

		bool ok
		    = LoadObjWithCallback(ifs, cb, this, &mtl_reader, &errstr);

		if (!errstr.empty())
			DEBUGMSG("Error loading OBJ file:\n%s\n",
				 errstr.c_str());

		if (!ok) DIE_HORRIBLY("Failed to parse OBJ file.\n");
	}
}
