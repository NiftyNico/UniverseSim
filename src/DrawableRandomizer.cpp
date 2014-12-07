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
	int mSize;
	DrawableRandomizer::rockShapes = (Shape*) malloc(mSize = numRockShapes * sizeof(Shape));
	memcpy(DrawableRandomizer::rockShapes, rockShapes, mSize);
	DrawableRandomizer::numRockShapes = numRockShapes;

	DrawableRandomizer::planetShapes = (Shape*) malloc(mSize = numPlanetShapes * sizeof(Shape));
	memcpy(DrawableRandomizer::planetShapes, planetShapes, mSize);
	DrawableRandomizer::numPlanetShapes = numPlanetShapes;

	DrawableRandomizer::starShapes = (Shape*) malloc(mSize = numStarShapes * sizeof(Shape));
	memcpy(DrawableRandomizer::starShapes, starShapes, mSize);
	DrawableRandomizer::numStarShapes = numStarShapes;

	DrawableRandomizer::uniformKsTexture = uniformKsTexture;
	DrawableRandomizer::blackTexture = blackTexture;	
}

Drawable DrawableRandomizer::randomDrawable(DrawableRandomizer::DrawableType type)
{
	Shape* shape;
	GLuint *kd, *ks, *atmos;
	float rSpeed = 0.0f;
	switch(type)
	{
		case DrawableRandomizer::DrawableType::ROCK:
			shape = DrawableRandomizer::rockShapes + rand() % DrawableRandomizer::numRockShapes;
			kd = ks = atmos = rockTextures + rand() % DrawableRandomizer::numRockTextures;
			break;
		case DrawableRandomizer::DrawableType::PLANET:
			shape = DrawableRandomizer::planetShapes + rand() % DrawableRandomizer::numPlanetShapes;
			kd = planetTextures + rand() % DrawableRandomizer::numPlanetTextures;
			ks = &blackTexture;
			atmos = DrawableRandomizer::atmosTextures + rand() % DrawableRandomizer::numAtmosTextures;
			rSpeed = (!(rand() % 1) ? -1 : 1) * rand() % 10 * 0.00001f;
			break;
		case DrawableRandomizer::DrawableType::STAR:
			shape = DrawableRandomizer::planetShapes + rand() % DrawableRandomizer::numPlanetShapes;
			kd = ks = atmos = planetTextures + rand() % DrawableRandomizer::numPlanetTextures;
			break;
		default:
			fprintf(stderr, "Unknown planet type\n");
			exit(UNKNOWN_PLANET_TYPE);
	}

	return Drawable(shape, kd, ks, atmos, rSpeed);
}

