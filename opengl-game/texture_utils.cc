#include <GL/glew.h>

#include "macros.hh"
#include "texture_utils.hh"

#define STB_IMAGE_IMPLEMENTATION
#include "external_files/stb_image.h"

/**
   Create a texture from the given filename.
*/
GLuint
create_texture(char const *image_path)
{
	GLuint id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	int width, height, nc; // nc = num channels. Unused.

	unsigned char *data = stbi_load(image_path, &width, &height, &nc, 3);
	if (!data) DIE_HORRIBLY("Failed to load image file %s.", image_path);

	// DEBUGMSG("Image: width = %d, height = %d, channels = %d.\n", width,
	//	 height, nc);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
		     GL_UNSIGNED_BYTE, data);

	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);

	// Clean up
	stbi_image_free(data);

	return id;
}
