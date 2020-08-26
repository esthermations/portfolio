LDLIBS=-lGLEW -framework OpenGL -lglfw -lm `pkg-config --libs glfw3`
CPPFLAGS=-std=c++11 -Wall -Wpedantic `pkg-config --cflags glfw3`

CC=clang++
CC_CALL=$(CC) $(LDLIBS) $(CPPFLAGS)
Out=assign3_part2

Sources=glfw_callbacks.cc file_utils.cc main.cc mouse.cc shader_utils.cc	\
	texture_utils.cc model.cc camera.cc globals.cc obj_utils.cc gameplay.cc \
	terrain.cc

Headers=glfw_callbacks.hh file_utils.hh macros.hh mouse.hh shader_utils.hh	\
	stb_image.h texture_utils.hh tiny_obj_loader.h model.hh camera.hh	\
	globals.hh obj_utils.hh gameplay.hh terrain.hh

Objects=$(subst .cc,.o,$(Sources))

all: $(Objects)
	$(CC_CALL) $(Objects) -o $(Out) 

obj: $(Sources) $(Headers)
	$(CC_CALL) $(Sources) -c -g

# AddressSanitizer for debugging segfaults and memory leaks. Be sure to run
# `make clean` after this, as the object files will be invalid for usual builds.
asan: $(Sources) $(Headers)
	clang++ $(CPPFLAGS) $(Sources) -c -g -fsanitize=address
	clang++ $(LDLIBS) $(Objects) -o $(Out) -g -fsanitize=address

clean:
	rm $(Objects)
