#pragma once

//------------------------------------------------------------------------------
// This file contains an implementation of a spherical camera
//------------------------------------------------------------------------------

//#include <GL/glew.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

struct Frame {
	glm::vec3 n;
	glm::vec3 u;
	glm::vec3 v;
};

class Camera {
public:

	Camera(float t, float p, float r);

	glm::mat4 getView();
	glm::vec3 getPos();

	void incrementTheta(float dt);
	void incrementPhi(float dp);
	void incrementR(float dr);
	void pan(float dx, float dy, int width, int height);

	Frame getFrame() {
		return generateFrameVectors();
	}

private:

	glm::vec3 lookat;
	float theta;
	float phi;
	float radius;

	Frame generateFrameVectors();
};
