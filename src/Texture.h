#ifndef TEXTURE_H
#define TEXTURE_H
#include <string>
#include "GLSL.h"
#include <sys/types.h>
#include <dirent.h>
#include <glob.h>
#include "Image.h"
#include <stdio.h>

#define BAD_PATH_ERROR 1

void loadTexture(GLuint* textureId, const char* fileName);
void loadTextures (GLuint** textures, unsigned int* numFiles, std::string path);
#endif