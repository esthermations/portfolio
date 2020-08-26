#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "file_utils.hh"
#include "macros.hh"
#include "shader_utils.hh"

/**
   Load and compile shader stored in file given by `path`. Returns true if all
   went well, false otherwise. Static, since it's only used in this file.
*/
static bool
compile_shader(char const *path, GLuint const id)
{
	char const *file_data = read_whole_file_into_memory(path);

	glShaderSource(id, 1, &file_data, NULL);
	glCompileShader(id);

	GLint result     = GL_FALSE;
	int   log_length = 0;

	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_length);

	if (log_length > 1) {
		char *log = (char *) calloc(log_length + 1, sizeof(char));
		glGetShaderInfoLog(id, log_length, NULL, &log[0]);
		DEBUGMSG("GL error log:\n%s\n----\n", log);
		free(log);
		return false;
	}

	free((char *) file_data);

	return true;
}

GLuint
load_shader(char const *vert_file_path, char const *frag_file_path)
{
	GLuint vert_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag_id = glCreateShader(GL_FRAGMENT_SHADER);

	if (!compile_shader(vert_file_path, vert_id)) {
		DIE_HORRIBLY("Vert shader %s failed to compile.",
			     vert_file_path);
	}

	if (!compile_shader(frag_file_path, frag_id)) {
		DIE_HORRIBLY("Frag shader %s failed to compile.",
			     frag_file_path);
	}

	GLuint shader_id = glCreateProgram();

	glAttachShader(shader_id, vert_id);
	glAttachShader(shader_id, frag_id);
	glLinkProgram(shader_id);

	GLint result     = GL_FALSE;
	GLint log_length = 0;

	glGetProgramiv(shader_id, GL_LINK_STATUS, &result);
	glGetProgramiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

	if (log_length > 5) {
		char *  log = (char *) calloc(log_length + 1, sizeof(char));
		GLsizei new_log_length = 0;
		glGetShaderInfoLog(shader_id, log_length, &new_log_length, log);

		DEBUGMSG("---- GL log:\n%s\n----", log);

		if (result == GL_FALSE) {
			DIE_HORRIBLY("Failed to link shader!");
		}
	}

	glDeleteShader(vert_id);
	glDeleteShader(frag_id);

	return shader_id;
}
