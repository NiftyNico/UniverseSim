#ifndef DRAWABLERANDOMIZER_H
#define DRAWABLERANDOMIZER_H

#include "Texture.h"
#include "Drawable.h"
#include <time.h> 
#include <stdio.h>
#include "Shape.h"
#include "GLSL.h"

#define UNKNOWN_PLANET_TYPE 1

class DrawableRandomizer {

private:
   unsigned int numRockTextures;
   GLuint* rockTextures;
	
   unsigned int numPlanetTextures;
   GLuint* planetTextures;
	
   unsigned int numStarTextures;
   GLuint* starTextures;
	
   unsigned int numAtmosTextures;
   GLuint* atmosTextures;

   static Shape* rockShapes;
   static unsigned int numRockShapes;
   static Shape* planetShapes;
   static unsigned int numPlanetShapes;
   static Shape* starShapes;
   static unsigned int numStarShapes;
   static GLuint uniformKsTexture;
   static GLuint blackTexture;

public: 
   DrawableRandomizer(std::string rockPath, std::string planetPath, std::string starPath, std::string atmosPath);

   static void initSharedRefs(Shape* rockShapes, unsigned int numRockShapes,
                              Shape* planetShapes, unsigned int numPlanetShapes, 
                              Shape* starShapes, unsigned int numStarShapes, 
                              GLuint uniformKsTexture, GLuint blackTexture);

   enum class DrawableType 
   { 
      ROCK, 
      PLANET, 
      STAR
   };

   Drawable randomDrawable(DrawableType type);
};
#endif