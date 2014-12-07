#include "Texture.h"

void loadTexture(GLuint* textureId, const char* fileName)
{
   Image *image1 = (Image *)malloc(sizeof(Image));
   if(image1 == NULL) {
      printf("Error allocating space for image");
   }
   if(!ImageLoad(fileName, image1)) {
      printf("Error loading texture image\n");
   }
   glActiveTexture(GL_TEXTURE1);
   glGenTextures(1, textureId);
   glBindTexture(GL_TEXTURE_2D, *textureId);
   glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY,
             0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);
   glGenerateMipmap(GL_TEXTURE_2D);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glBindTexture(GL_TEXTURE_2D, 0);
   free(image1);  
}



void loadTextures (GLuint** textures, unsigned int* numFiles, std::string path)
{
   DIR *dp;
   struct dirent *dirp;
   if((dp  = opendir(path.c_str())) == NULL) {
      fprintf(stderr, "Could not open path: %s\n", path.c_str());
      exit(BAD_PATH_ERROR);
   }

   glob_t gl;
   if(glob((path + "*.bmp").c_str(), GLOB_NOSORT, NULL, &gl) == 0)
      *numFiles = gl.gl_pathc;
   globfree(&gl);

   *textures = (GLuint*) malloc((*numFiles) * sizeof(GLuint));

   for (unsigned int i = 0; i < *numFiles;){
      dirp = readdir(dp);
      std::string fileName = std::string(dirp->d_name);
      if(fileName.substr(fileName.find_last_of(".")) == ".bmp")
         loadTexture(*textures + i++, (path + fileName).c_str());
   }
   closedir(dp);
}