all:
	g++ -ansi -pedantic -Wno-deprecated src/*.cpp src/*.cc -DGL_GLEXT_PROTOTYPES -lglut -lGL -lGLU -o universe

osx:
	g++ -ansi -pedantic -Wno-deprecated src/*.cpp src/*.cc -framework GLUT -framework OpenGL -o universe

clean:
	rm -f -r *~ *.o universe
