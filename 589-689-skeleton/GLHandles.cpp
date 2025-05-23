#include "GLHandles.h"

#include <algorithm> // For std::swap

ShaderHandle::ShaderHandle(GLenum type)
	: shaderID(glCreateShader(type))
{
}


ShaderHandle::ShaderHandle(ShaderHandle&& other) noexcept
	: shaderID(std::move(other.shaderID))
{
	other.shaderID = 0;
}


ShaderHandle& ShaderHandle::operator=(ShaderHandle&& other) noexcept {
	// Note that swap is implemented with move internally
	std::swap(shaderID, other.shaderID);
	return *this;
}


ShaderHandle::~ShaderHandle() {
	glDeleteShader(shaderID);
}


ShaderHandle::operator GLuint() const {
	return shaderID;
}

GLuint ShaderHandle::value() const {
	return shaderID;
}


//------------------------------------------------------------------------------


ShaderProgramHandle::ShaderProgramHandle()
	: programID(glCreateProgram())
{
}


ShaderProgramHandle::ShaderProgramHandle(ShaderProgramHandle&& other) noexcept
	: programID(std::move(other.programID))
{
	other.programID = 0;
}


ShaderProgramHandle& ShaderProgramHandle::operator=(ShaderProgramHandle&& other) noexcept {
	std::swap(programID, other.programID);
	return *this;
}


ShaderProgramHandle::~ShaderProgramHandle() {
	glDeleteProgram(programID);
}


ShaderProgramHandle::operator GLuint() const {
	return programID;
}


GLuint ShaderProgramHandle::value() const {
	return programID;
}

//------------------------------------------------------------------------------


VertexArrayHandle::VertexArrayHandle()
	: vaoID(0) // Due to OpenGL syntax, we can't initial directly here, like we want.
{
	glGenVertexArrays(1, &vaoID);
}


VertexArrayHandle::VertexArrayHandle(VertexArrayHandle&& other) noexcept
	: vaoID(std::move(other.vaoID))
{
	other.vaoID = 0;
}


VertexArrayHandle& VertexArrayHandle::operator=(VertexArrayHandle&& other) noexcept {
	std::swap(vaoID, other.vaoID);
	return *this;
}


VertexArrayHandle::~VertexArrayHandle() {
	glDeleteVertexArrays(1, &vaoID);
}


VertexArrayHandle::operator GLuint() const {
	return vaoID;
}


GLuint VertexArrayHandle::value() const {
	return vaoID;
}

//------------------------------------------------------------------------------


VertexBufferHandle::VertexBufferHandle()
	: vboID(0) // Due to OpenGL syntax, we can't initial directly here, like we want.
{
	glGenBuffers(1, &vboID);
}


VertexBufferHandle::VertexBufferHandle(VertexBufferHandle&& other) noexcept
	: vboID(std::move(other.vboID))
{
	other.vboID = 0;
}


VertexBufferHandle& VertexBufferHandle::operator=(VertexBufferHandle&& other) noexcept {
	std::swap(vboID, other.vboID);
	return *this;
}


VertexBufferHandle::~VertexBufferHandle() {
	glDeleteBuffers(1, &vboID);
}


VertexBufferHandle::operator GLuint() const {
	return vboID;
}


GLuint VertexBufferHandle::value() const {
	return vboID;
}


//------------------------------------------------------------------------------

TextureHandle::TextureHandle()
	: textureID(0) // Due to OpenGL syntax, we can't initial directly here, like we want.
{
	glGenTextures(1, &textureID);
}


TextureHandle::TextureHandle(TextureHandle&& other) noexcept
	: textureID(std::move(other.textureID))
{
	other.textureID = 0;
}

TextureHandle& TextureHandle::operator=(TextureHandle&& other) noexcept {
	std::swap(textureID, other.textureID);
	return *this;
}


TextureHandle::~TextureHandle() {
	glDeleteTextures(1, &textureID);
}


TextureHandle::operator GLuint() const {
	return textureID;
}


GLuint TextureHandle::value() const {
	return textureID;
}

//------------------------------------------------------------------------------

FramebufferHandle::FramebufferHandle()
	: framebufferID(0) // Due to OpenGL syntax, we can't initial directly here, like we want.
{
	glGenFramebuffers(1, &framebufferID);
}


FramebufferHandle::FramebufferHandle(FramebufferHandle&& other) noexcept
	: framebufferID(std::move(other.framebufferID))
{
	other.framebufferID = 0;
}

FramebufferHandle& FramebufferHandle::operator=(FramebufferHandle&& other) noexcept {
	std::swap(framebufferID, other.framebufferID);
	return *this;
}


FramebufferHandle::~FramebufferHandle() {
	glDeleteFramebuffers(1, &framebufferID);
}


FramebufferHandle::operator GLuint() const {
	return framebufferID;
}


GLuint FramebufferHandle::value() const {
	return framebufferID;
}

//------------------------------------------------------------------------------

RenderbufferHandle::RenderbufferHandle()
	: renderbufferID(0) // Due to OpenGL syntax, we can't initial directly here, like we want.
{
	glGenRenderbuffers(1, &renderbufferID);
}


RenderbufferHandle::RenderbufferHandle(RenderbufferHandle&& other) noexcept
	: renderbufferID(std::move(other.renderbufferID))
{
	other.renderbufferID = 0;
}

RenderbufferHandle& RenderbufferHandle::operator=(RenderbufferHandle&& other) noexcept {
	std::swap(renderbufferID, other.renderbufferID);
	return *this;
}


RenderbufferHandle::~RenderbufferHandle() {
	glDeleteRenderbuffers(1, &renderbufferID);
}


RenderbufferHandle::operator GLuint() const {
	return renderbufferID;
}


GLuint RenderbufferHandle::value() const {
	return renderbufferID;
}


//------------------------------------------------------------------------------


ElementBufferHandle::ElementBufferHandle()
	: eboID(0) // Due to OpenGL syntax, we can't initial directly here, like we want.
{
	glGenBuffers(1, &eboID);
}


ElementBufferHandle::ElementBufferHandle(ElementBufferHandle&& other) noexcept
	: eboID(std::move(other.eboID))
{
	other.eboID = 0;
}


ElementBufferHandle& ElementBufferHandle::operator=(ElementBufferHandle&& other) noexcept {
	std::swap(eboID, other.eboID);
	return *this;
}


ElementBufferHandle::~ElementBufferHandle() {
	glDeleteBuffers(1, &eboID);
}


ElementBufferHandle::operator GLuint() const {
	return eboID;
}


GLuint ElementBufferHandle::value() const {
	return eboID;
}
