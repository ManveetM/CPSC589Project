#include "Framebuffer.h"

Framebuffer::Framebuffer()
	: framebufferID()
{
}


void Framebuffer::addTextureAttachment(GLenum attachmentType, Texture& tex)
{
	bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D, (GLuint) tex, 0);
	unbind();
}

void Framebuffer::addRenderbufferAttachment(GLenum attachmentType, Renderbuffer& rb)
{
	bind();
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, (GLuint)rb);
	unbind();
}
