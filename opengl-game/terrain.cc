#include "terrain.hh"
#include "model.hh"
#include "macros.hh"

/**
   Generate a plane VAO with N points on it.
 */
Model
generate_terrain_model(int N)
{
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> texcoords;

	size_t size = N * N * 3 /* vals per vert */ * 6 /* verts per quad */;
	vertices.reserve(size);
	normals.reserve(size);
	// texcoords.reserve(size);

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {

			float fi = (float) i;
			float fj = (float) j;

			vertices.push_back(fi / N);
			vertices.push_back(0);
			vertices.push_back(fj / N);

			vertices.push_back(fi / N);
			vertices.push_back(0);
			vertices.push_back((fj + 1) / N);

			vertices.push_back((fi + 1) / N);
			vertices.push_back(0);
			vertices.push_back((fj + 1) / N);

			vertices.push_back((fi + 1) / N);
			vertices.push_back(0);
			vertices.push_back((fj + 1) / N);

			vertices.push_back((fi + 1) / N);
			vertices.push_back(0);
			vertices.push_back(fj / N);

			vertices.push_back(fi / N);
			vertices.push_back(0);
			vertices.push_back(fj / N);
		}
	}

	for (size_t i = 0; i < size / 3; ++i) {
		normals.push_back(0);
		normals.push_back(1);
		normals.push_back(0);
	}

	Model terrain;
	terrain.vao        = Vao(vertices, normals, texcoords);
	terrain.position   = glm::vec3(0);
	terrain.scale      = glm::vec3(1);
	terrain.rotation   = glm::vec3(0);
	terrain.dimensions = find_dimensions(vertices);

	return terrain;
}
