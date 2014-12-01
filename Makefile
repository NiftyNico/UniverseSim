all:
	g++ -ansi -pedantic -Wno-deprecated src/*.cpp src/*.cc -DGL_GLEXT_PROTOTYPES -lglut -lGL -lGLU -o universe -pthread

osx:
	g++ -ansi -pedantic -Wno-deprecated src/*.cpp src/*.cc src/pthread-barrier.c -framework GLUT -framework OpenGL -o universe -pthread

clean:
	rm -f -r *~ *.o universe
