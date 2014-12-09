#include "DrawableRandomizer.h"

GLuint DrawableRandomizer::uniformKsTexture;
GLuint DrawableRandomizer::blackTexture;
Shape* DrawableRandomizer::rockShapes;
unsigned int DrawableRandomizer::numRockShapes;
Shape* DrawableRandomizer::planetShapes;
unsigned int DrawableRandomizer::numPlanetShapes;
Shape* DrawableRandomizer::starShapes;
unsigned int DrawableRandomizer::numStarShapes;

DrawableRandomizer::DrawableRandomizer(std::string rockPath, std::string planetPath, std::string starPath, std::string atmosPath)
{
   loadTextures(&rockTextures, &numRockTextures, rockPath);
   loadTextures(&planetTextures, &numPlanetTextures, planetPath);
   loadTextures(&starTextures, &numStarTextures, starPath);
   loadTextures(&atmosTextures, &numAtmosTextures, atmosPath);	
}

void DrawableRandomizer::initSharedRefs(Shape* rockShapes, unsigned int numRockShapes,
                    					Shape* planetShapes, unsigned int numPlanetShapes, 
                    					Shape* starShapes, unsigned int numStarShapes, 
                    					GLuint uniformKsTexture, GLuint blackTexture)
{
	//TODO: Copy rather than setting reference
	DrawableRandomizer::rockShapes = rockShapes;
	DrawableRandomizer::numRockShapes = numRockShapes;

	DrawableRandomizer::planetShapes = planetShapes;	
	DrawableRandomizer::numPlanetShapes = numPlanetShapes;

	DrawableRandomizer::starShapes = starShapes;	
	DrawableRandomizer::numStarShapes = numStarShapes;

	DrawableRandomizer::uniformKsTexture = uniformKsTexture;
	DrawableRandomizer::blackTexture = blackTexture;	
}

Drawable DrawableRandomizer::randomDrawable(DrawableType type)
{
	Shape* shape;
	GLuint *kd, *ks, *atmos;
	float rSpeed = 0.0f;
	switch(type)
	{
		case DrawableType::ROCK:
			shape = DrawableRandomizer::rockShapes + rand() % DrawableRandomizer::numRockShapes;
			kd = rockTextures + rand() % DrawableRandomizer::numRockTextures;
			ks = atmos = &blackTexture;
			break;
		case DrawableType::PLANET:
			shape = DrawableRandomizer::planetShapes + rand() % DrawableRandomizer::numPlanetShapes;
			kd = planetTextures + rand() % DrawableRandomizer::numPlanetTextures;
			ks = &blackTexture;
			atmos = DrawableRandomizer::atmosTextures + rand() % DrawableRandomizer::numAtmosTextures;
			rSpeed = (!(rand() % 1) ? -1 : 1) * rand() % 10 * 0.00001f;
			break;
		case DrawableType::STAR:
			shape = DrawableRandomizer::starShapes + rand() % DrawableRandomizer::numStarShapes;
			kd = starTextures + rand() % DrawableRandomizer::numStarTextures;
			ks = atmos = &blackTexture;
			break;
		default:
			fprintf(stderr, "Black hole unsupported\n");
			exit(UNKNOWN_PLANET_TYPE);
	}

	return Drawable(type, shape, kd, ks, atmos, rSpeed);
}

