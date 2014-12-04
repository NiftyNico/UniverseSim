CCFLAGS=-O3 -ansi -pedantic -Wno-deprecated

all:
	g++ $(CCFLAGS) src/*.cpp src/*.cc -DGL_GLEXT_PROTOTYPES -lglut -lGL -lGLU -o universe -pthread

osx:
	g++ $(CCFLAGS) src/*.cpp src/*.cc src/pthread-barrier.c -framework GLUT -framework OpenGL -o universe -pthread

ot: src/OctreeTester.cpp src/Octree.cpp src/Octree.h src/Mass.cpp src/Mass.h
	g++ $(CCFLAGS) src/OctreeTester.cpp src/Octree.cpp src/Mass.cpp -o ot 

clean:
	rm -f -r *~ *.o universe
