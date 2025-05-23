#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "Log.h"

Texture::Texture(std::string path, GLint interpolation)
	: textureID(), path(path), interpolation(interpolation)
{
	int numComponents;
	stbi_set_flip_vertically_on_load(true);
	const char* pathData = path.c_str();
	unsigned char* data = stbi_load(pathData, &width, &height, &numComponents, 0);
	if (data != nullptr)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);		//Set alignment to be 1

		bind();

		//Set number of components by format of the texture
		GLuint format = GL_RGB;
		switch (numComponents)
		{
		case 4:
			format = GL_RGBA;
			break;
		case 3:
			format = GL_RGB;
			break;
		case 2:
			format = GL_RG;
			break;
		case 1:
			format = GL_RED;
			break;
		default:
			Log::error("Invalid number of components {} for {}!", numComponents, path);
			throw std::runtime_error("Invalid number of componenets!");
			break;
		};
		//Loads texture data into bound texture
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interpolation);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interpolation);

		// Clean up
		unbind();
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);	//Return to default alignment
		stbi_image_free(data);

	}
	else {
		throw std::runtime_error("Failed to read texture data from file!");
	}
}

Texture::Texture(int level, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, GLint interpolation)
	: textureID(), interpolation(interpolation)
{

	bind();

	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
	glTexImage2D(GL_TEXTURE_2D, level, internalFormat, width, height, 0, format, type, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interpolation);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interpolation);

	unbind();
}

