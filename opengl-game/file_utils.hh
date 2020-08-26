#ifndef _FILE_UTILS_HH_
#define _FILE_UTILS_HH_

size_t get_file_size_in_bytes(int file_descriptor);
char *read_whole_file_into_memory(char const *path);
void chdir_or_die(char const *dir);

#endif // file_utils.hh
