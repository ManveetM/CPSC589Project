#include "Camera.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <iostream>

#include "glm/gtc/matrix_transform.hpp"

Camera::Camera(float t, float p, float r) : theta(t), phi(p), radius(r) {
	lookat = glm::vec3(0.0f, 0.0f, 0.0f);
}

glm::mat4 Camera::getView() {
	glm::vec3 offset = radius * glm::vec3(std::cos(theta) * std::sin(phi), std::sin(theta), std::cos(theta) * std::cos(phi));
	glm::vec3 eye = lookat + offset;
	glm::vec3 at = lookat;
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	return glm::lookAt(eye, at, up);
}

glm::vec3 Camera::getPos() {
	glm::vec3 offset = radius * glm::vec3(std::cos(theta) * std::sin(phi), std::sin(theta), std::cos(theta) * std::cos(phi));
	return lookat + offset;
}

void Camera::incrementTheta(float dt) {
	if (theta + (dt / 100.0f) < M_PI_2 && theta + (dt / 100.0f) > -M_PI_2) {
		theta += dt / 100.0f;
	}
}

void Camera::incrementPhi(float dp) {
	phi -= dp / 100.0f;
	if (phi > 2.0 * M_PI) {
		phi -= 2.0 * M_PI;
	} else if (phi < 0.0f) {
		phi += 2.0 * M_PI;
	}
}

void Camera::incrementR(float dr) {
	radius -= dr;
	if (radius <= 0.3f) {
		radius = 0.3f;
	}
}

Frame Camera::generateFrameVectors() {
	glm::vec3 offset = radius * glm::vec3(std::cos(theta) * std::sin(phi), std::sin(theta), std::cos(theta) * std::cos(phi));
	glm::vec3 eye = lookat + offset;
	glm::vec3 n = glm::normalize(eye - lookat);

	glm::vec3 upApprox = glm::vec3(
        -std::sin(phi) * std::cos(theta),
        std::cos(theta),
        -std::cos(phi) * std::cos(theta)
    );

	glm::vec3 v = glm::normalize(upApprox - glm::dot(upApprox, n) * n);

	glm::vec3 u = glm::normalize(glm::cross(v, n));

	return Frame{ n, u, v };
}

void Camera::pan(float dx, float dy) {
	Frame f = generateFrameVectors();
	glm::vec3 u = f.u;
	glm::vec3 v = f.v;

	lookat = glm::translate(glm::mat4(1.0f), 0.00105f * radius * -dx * u) * glm::vec4(lookat, 1.0f);
	lookat = glm::translate(glm::mat4(1.0f), 0.00105f * radius * dy * v) * glm::vec4(lookat, 1.0f);
}
