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
	glm::vec3 getLookAt() { return lookat; }

	void incrementTheta(float dt);
	void incrementPhi(float dp);
	void incrementR(float dr);
	void pan(float dx, float dy, int width, int height);

	void setValues(float t_, float p_, float r_) {
		theta = t_;
		phi = p_;
		radius = r_;
	}

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
