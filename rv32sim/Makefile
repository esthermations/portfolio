CC=clang
CXX=clang++
RM=rm -f
CPPFLAGS=-g -std=c++11 -Wall -Wextra
# -Wpedantic complains about designated initialisers so it can just go away and
# leave me alone.
LDFLAGS=-g
LDLIBS=

SRCS=$(wildcard *.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))

all: rv32sim

rv32sim: $(OBJS)
	$(CXX) $(LDFLAGS) -o rv32sim $(OBJS) $(LDLIBS) 

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

dist-clean: clean
	$(RM) *~ .dependtool

include .depend
