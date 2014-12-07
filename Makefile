SOURCES=BoxList.cpp Camera.cpp GLSL.cpp Image.cpp main.cpp Mass.cpp MatrixStack.cpp Octree.cpp planet.cpp Shape.cpp Simulator.cpp Texture.cpp tiny_obj_loader.cc
SRCFILES=$(addprefix src/, $(SOURCES))
CCFLAGS=-O3 -ansi -pedantic -Wno-deprecated -g -Wall

all:
	g++ $(CCFLAGS) $(SRCFILES) -DGL_GLEXT_PROTOTYPES -lglut -lGL -lGLU -o universe -pthread

osx:
	g++ $(CCFLAGS) $(SRCFILES) src/pthread-barrier.c -framework GLUT -framework OpenGL -o universe -pthread

ot: src/OctreeTester.cpp src/Octree.cpp src/Octree.h src/Mass.cpp src/Mass.h
	g++ $(CCFLAGS) src/OctreeTester.cpp src/Octree.cpp src/Mass.cpp -o ot 

clean:
	rm -f -r *~ *.o universe
