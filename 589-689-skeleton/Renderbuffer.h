#pragma once

#include "GLHandles.h"
#include <glad/glad.h>

class Renderbuffer {
public:
	// Constructor
	Renderbuffer();

	// Because we're using the RenderbufferHandle to do RAII for the renderbuffer for us
	// and our other types are trivial or provide their own RAII
	// we don't have to provide any specialized functions here. Rule of zero
	//
	// https://en.cppreference.com/w/cpp/language/rule_of_three
	// https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Rc-zero

	void bind() { glBindRenderbuffer(GL_RENDERBUFFER, renderbufferID); }
	void unbind() { glBindRenderbuffer(GL_RENDERBUFFER, 0); }

	// "internalStorage" is to specify whether the buffer is for depth, 
	// RGBA colour, whether to use floats or ints, etc.
	// For more info: https://registry.khronos.org/OpenGL-Refpages/gl4/html/glRenderbufferStorage.xhtml
	void setStorage(GLenum internalStorage, GLsizei width, GLsizei height);

	operator GLuint() const {
		return renderbufferID;
	}

private:
	RenderbufferHandle renderbufferID;

};