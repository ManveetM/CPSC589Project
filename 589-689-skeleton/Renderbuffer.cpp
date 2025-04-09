#include "Renderbuffer.h"

Renderbuffer::Renderbuffer()
	: renderbufferID()
{
}


void Renderbuffer::setStorage(GLenum internalStorage, GLsizei width, GLsizei height)
{
	bind();
	glRenderbufferStorage(GL_RENDERBUFFER, internalStorage, width, height);
	unbind();
}