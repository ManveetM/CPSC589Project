#pragma once

#include "GLHandles.h"
#include "Renderbuffer.h"
#include "Texture.h"
#include <glad/glad.h>

class Framebuffer {
public:
	// Constructor
	Framebuffer();

	// Because we're using the FramebufferHandle to do RAII for the framebuffer for us
	// and our other types are trivial or provide their own RAII
	// we don't have to provide any specialized functions here. Rule of zero
	//
	// https://en.cppreference.com/w/cpp/language/rule_of_three
	// https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Rc-zero

	void bind() { glBindFramebuffer(GL_FRAMEBUFFER, framebufferID); }
	void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

	// "attachmentType" tells OpenGL whether the texture/buffer is for depth
	// data, color data, stencil data, etc.
	void addTextureAttachment(GLenum attachmentType, Texture& tex);
	void addRenderbufferAttachment(GLenum attachmentType, Renderbuffer& rb);

private:
	FramebufferHandle framebufferID;

};