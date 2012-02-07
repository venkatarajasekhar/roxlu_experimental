#include "Texture.h"
#include "Image.h"
#include "Error.h"

namespace roxlu {

Texture::Texture() {
	glGenTextures(1, &texture_id); eglGetError();
}

Texture::~Texture() {
}

void Texture::setParams() {
	bind();
		glEnable(GL_TEXTURE_2D); eglGetError();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); eglGetError(); // or GL_CLAMP (=> clamp give artifacts with uvsphere) ?
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); eglGetError();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); eglGetError();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); eglGetError();
	unbind();
}

/*
void Texture::loadImage(string fileName, GLint imageFormat) {
	// load image.
	image_file = fileName;
	printf("Loaded texture path: '%s'\n", image_file.c_str());
	if(!img.loadImage(fileName)) {
		ofLog(OF_LOG_ERROR, "Cannot load file: %s", fileName.c_str());
		return;
	}
	setParams();
	
	// upload texture.
	bind();
		glTexImage2D(
			 GL_TEXTURE_2D, 0
			,GL_RGBA, img.getWidth(), img.getHeight(), 0
			,imageFormat, GL_UNSIGNED_BYTE, img.getPixels());
		eglGetError();
	unbind();
}
*/

void Texture::setPixels(unsigned char* pixels, int width, int height, GLenum format) {
	bind();
	glTexImage2D(
		 GL_TEXTURE_2D
		,0
		,GL_RGBA 
		,width
		,height
		,0
		,format
		,GL_UNSIGNED_BYTE
		,pixels
	);
	setParams();
	eglGetError();
	unbind();
}

string Texture::getImageFilePath() {
	return File::toDataPath(image_file);
}

void Texture::bind() {
	glEnable(GL_TEXTURE_2D);	
	glBindTexture(GL_TEXTURE_2D, texture_id); 
}

void Texture::unbind() {
	glBindTexture(GL_TEXTURE_2D,0); 
}


GLuint Texture::getTextureID() {
	return texture_id;
}

}; // roxlu