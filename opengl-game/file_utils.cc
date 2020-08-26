#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "file_utils.hh"
#include "macros.hh"

/**
   Return the size (in bytes) of the file pointed to by the given file
   descriptor.
*/
size_t
get_file_size_in_bytes(int file_descriptor)
{
	struct stat stats;

	int result = fstat(file_descriptor, &stats);

	if (result == -1) {
		DIE_HORRIBLY("Failed to get file size of file %d.\n"
			     "This is weird. Bailing out.\n",
			     file_descriptor);
	}

	return stats.st_size;
}

/**
   Load the whole file given by `path` into RAM. Obviously this should not be
   called on enormous files.

   Returns a pointer to a calloc'd buffer containing the file text. The buffer
   ends with a null byte, so its length can be calculated with strlen().

   This buffer should be freed manually.
*/
char *
read_whole_file_into_memory(char const *path)
{
	int file_descriptor = open(path, O_RDONLY);
	if (file_descriptor == -1) {
		DIE_HORRIBLY(
		    "%s: Failed to open file! It probably doesn't exist.\n",
		    path);
	}

	// Add 1 for a null byte on the end
	size_t const buffer_size = 1 + get_file_size_in_bytes(file_descriptor);

	if (buffer_size > (500 * 1000 * 1000)) {
		DEBUGMSG("Loading a pretty big file into memory. (%zu bytes)",
			 buffer_size);
	}

	char *const text_buffer = (char *) calloc(buffer_size, sizeof(char));

	{
		int result = read(file_descriptor, text_buffer, buffer_size);

		if (result == -1) {
			DIE_HORRIBLY("%s: Error reading file!\n", path);
		}
	}

	text_buffer[buffer_size - 1] = '\0';
	close(file_descriptor);

	return text_buffer;
}

void
chdir_or_die(char const *dir)
{

	if (!dir) {
		DIE_HORRIBLY("chdir_or_die() was passed a NULL directory.\n");
	}

	errno = 0;
	if (chdir(dir) == -1) {
		perror(__FUNCTION__);
		DIE_HORRIBLY("Failed to open directory %s.", dir);
	}
}
