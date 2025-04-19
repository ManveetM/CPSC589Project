#pragma once

#include "GLHandles.h"

#include <glad/glad.h>


class ElementBuffer {

public:
	ElementBuffer(GLuint index, GLint size, GLenum dataType);

	// Public interface
	void bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID); }

	void uploadData(GLsizeiptr size, const void* data, GLenum usage);

private:
	ElementBufferHandle bufferID;
};
