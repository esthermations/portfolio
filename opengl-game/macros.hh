#ifndef _MACROS_HH_
#define _MACROS_HH_

/*
  Global macros used for debugging.
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

//#define M_PI 3.14159265358979323846
//#define M_PI_2 1.57079632679489661923 // pi/2

#define DEG2RAD(x) ((x) * ((M_PI) / 180))
#define RAD2DEG(x) ((x) * (180 / (M_PI)))

#define ABS(a) ((a) < 0 ? -(a) : (a))
#define MAX(a, b) ((a) < (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (b) : (a))
#define CLAMP(min, val, max) MIN(min, MAX(max, val))

#define ARRAY_LENGTH(a) (sizeof((a)) / sizeof((a)[0]))

#define STRINGIFY(str) #str

#define DEBUGMSG(...)                                                          \
	do {                                                                   \
		fprintf(stderr, __VA_ARGS__);                                  \
	} while (0)

#define DIE_HORRIBLY(...)                                                      \
	do {                                                                   \
		DEBUGMSG(__VA_ARGS__);                                         \
		DEBUGMSG("\n");                                                \
		DEBUGMSG("Dying horribly in %s().\n", __func__);               \
		exit(1);                                                       \
	} while (0)

#define DEBUG_PRINT_ARRAY(arr, len, fmt)                                       \
	do {                                                                   \
		DEBUGMSG(#arr "[%zu]: { ", (size_t) len);                      \
		for (size_t _I = 0; _I < (size_t) len; ++_I)                   \
			DEBUGMSG(fmt ", ", arr[_I]);                           \
		DEBUGMSG("}\n");                                               \
	} while (0)

#define DEBUG_PRINT_MAT4(mat)                                                  \
	do {                                                                   \
		puts(#mat " = { ");                                            \
		for (int i = 0; i < 4; ++i) {                                  \
			for (int j = 0; j < 4; ++j) {                          \
				/* Column-major. */                            \
				printf(" %+.1f ", mat[j][i]);                  \
			}                                                      \
			puts("");                                              \
		}                                                              \
		puts("}");                                                     \
	} while (0)

#endif // macros.hh
