#pragma once

#include "Window.h"
#include "Camera.h"
#include "ShaderProgram.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "PointsData.h"

class Callbacks3D : public CallbackInterface {

public:
	// Constructor. We use values of -1 for attributes that, at the start of
	// the program, have no meaningful/"true" value.
	Callbacks3D(ShaderProgram& shader, ShaderProgram& pickerShader, int screenWidth, int screenHeight)
		: shader(shader)
		, pickerShader(pickerShader)
		, camera(glm::radians(0.f), glm::radians(0.f), 3.0)
		, aspect(1.0f)
		, rightMouseDown(false)
		, leftMouseDown(false)
		, middleMouseDown(false)
		, mouseOldX(-1.0)
		, mouseOldY(-1.0)
		, screenWidth(screenWidth)
		, screenHeight(screenHeight)
	{
		updateUniformLocations();
	}

	glm::ivec2 getMousePos() {
		return glm::ivec2(mouseOldX, mouseOldY);
	}

	bool isLeftMouseDown() {
		return leftMouseDown;
	}

	bool isRightMouseDown() {
		return rightMouseDown;
	}

	bool isMiddleMouseDown() {
		return middleMouseDown;
	}

	glm::vec3 getDragOffset() {
		glm::vec3 temp = dragOffset;
		dragOffset = glm::vec3(0.0f);
		return temp;
	}

	void setIs3D(bool is3D_) {
		is3D = is3D_;
	}

	void resetCamera() {
		camera.setValues(0.0, 0.0, 1.0);
	}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			shader.recompile();
			updateUniformLocations();
		}
	}

	virtual void mouseButtonCallback(int button, int action, int mods) {

		auto& io = ImGui::GetIO();
		if (io.WantCaptureMouse && action == GLFW_PRESS) return;

		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS) {
				rightMouseDown = true;
			}
			else if (action == GLFW_RELEASE) {
				rightMouseDown = false;
			}
		}

		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (action == GLFW_PRESS) {
				leftMouseDown = true;
			}
			else if (action == GLFW_RELEASE) {
				leftMouseDown = false;
			}
		}

		if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
			if (action == GLFW_PRESS) {
				middleMouseDown = true;
			}
			else if (action == GLFW_RELEASE) {
				middleMouseDown = false;
			}
		}
	}

	virtual void cursorPosCallback(double xpos, double ypos) {
		if (is3D && rightMouseDown) {
			camera.incrementTheta(ypos - mouseOldY);
			camera.incrementPhi(xpos - mouseOldX);
		}
		if (middleMouseDown) {
			float deltaX = xpos - mouseOldX;
			float deltaY = ypos - mouseOldY;
			camera.pan(deltaX, deltaY, screenWidth, screenHeight);
		}

		if (leftMouseDown) {
			Frame localFrame = camera.getFrame();

			float deltaX = xpos - mouseOldX;
			float deltaY = ypos - mouseOldY;

			float ndx = deltaX / (float) screenWidth;
			float ndy = deltaY / (float) screenHeight;

			float distance = glm::length(camera.getLookAt() - camera.getPos());
			float scale = 2.0f * distance * tan(glm::radians(45.0f) * 0.5f);

			dragOffset = ndx * scale * aspect * localFrame.u - ndy * scale * localFrame.v;
		}

		mouseOldX = xpos;
		mouseOldY = ypos;
	}

	// Updates the screen width and height, in screen coordinates
	// (not necessarily the same as pixels)
	virtual void windowSizeCallback(int width, int height) {
		screenWidth = width;
		screenHeight = height;
		aspect = float(width) / float(height);
	}

	virtual void scrollCallback(double xoffset, double yoffset) {
		if (is3D) {
			camera.incrementR(yoffset);
		}
	}

	void viewPipeline() {
		glm::mat4 M = glm::mat4(1.0);
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);
		glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(M));
		glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(V));
		glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(P));
	}

	void viewPipelinePicker() {
		glm::mat4 M = glm::mat4(1.0);
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);
		glUniformMatrix4fv(mLocPicker, 1, GL_FALSE, glm::value_ptr(M));
		glUniformMatrix4fv(vLocPicker, 1, GL_FALSE, glm::value_ptr(V));
		glUniformMatrix4fv(pLocPicker, 1, GL_FALSE, glm::value_ptr(P));
	}

	void viewPipelineControlPoints(ShaderProgram& sp) {
		glm::mat4 M = glm::mat4(1.0f);
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);

		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}

	void viewPipelinePlantPreview(const glm::mat4& modelMatrix) {
		glm::mat4 M = modelMatrix;
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);

		GLint uniMat = glGetUniformLocation(shader, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(shader, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(shader, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}

	void viewPipelineEditing(ShaderProgram& sp) {
		glm::mat4 M = glm::mat4(1.0f);
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -1.0f, 10.0f);

		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}

	void updateShadingUniforms(
		const glm::vec3& lightPos, const glm::vec3& lightCol, glm::vec3& diffuseCol, float ambientStrength, bool texExistence
	)
	{
		// Like viewPipeline(), this function assumes shader.use() was called before.
		glUniform3f(diffuseColLoc, diffuseCol.r, diffuseCol.g, diffuseCol.b);
		glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(lightColLoc, lightCol.r, lightCol.g, lightCol.b);
		glUniform1f(ambientStrengthLoc, ambientStrength);
		glUniform1i(texExistenceLoc, (int)texExistence);
	}

	glm::vec2 getCursorPosGL() {
		glm::vec2 screenPos(mouseOldX, mouseOldY);
		// Interpret click as at centre of pixel.
		glm::vec2 centredPos = screenPos + glm::vec2(0.5f, 0.5f);
		// Scale cursor position to [0, 1] range.
		glm::vec2 scaledToZeroOne = centredPos / glm::vec2(screenWidth, screenHeight);

		glm::vec2 flippedY = glm::vec2(scaledToZeroOne.x, 1.0f - scaledToZeroOne.y);

		glm::vec2 adjustedForAspect = glm::vec2(flippedY.x, flippedY.y);

		// Go from [0, 1] range to [-1, 1] range.

		glm::vec2 temp = 2.f * adjustedForAspect - glm::vec2(1.f, 1.f);
		temp.x *= aspect;

		return temp + glm::vec2(camera.getLookAt());
	}

	Camera getCamera() {
		return camera;
	}

private:
	// Uniform locations do not, ordinarily, change between frames.
	// However, we may need to update them if the shader is changed and recompiled.
	void updateUniformLocations() {
		mLoc = glGetUniformLocation(shader, "M");
		vLoc = glGetUniformLocation(shader, "V");
		pLoc = glGetUniformLocation(shader, "P");
		lightPosLoc = glGetUniformLocation(shader, "lightPos");
		lightColLoc = glGetUniformLocation(shader, "lightCol");
		ambientStrengthLoc = glGetUniformLocation(shader, "ambientStrength");
		diffuseColLoc = glGetUniformLocation(shader, "diffuseCol");
		texExistenceLoc = glGetUniformLocation(shader, "texExistence");

		mLocPicker = glGetUniformLocation(pickerShader, "M");
		vLocPicker = glGetUniformLocation(pickerShader, "V");
		pLocPicker = glGetUniformLocation(pickerShader, "P");
	}

	int screenWidth;
	int screenHeight;

	bool rightMouseDown;
	bool leftMouseDown;
	bool middleMouseDown;
	float aspect;
	double mouseOldX;
	double mouseOldY;

	// Uniform locations
	GLint mLoc;
	GLint vLoc;
	GLint pLoc;
	GLint lightPosLoc;
	GLint lightColLoc;
	GLint diffuseColLoc;
	GLint ambientStrengthLoc;
	GLint texExistenceLoc;

	// Uniform locations for Picker Shader
	GLint mLocPicker;
	GLint vLocPicker;
	GLint pLocPicker;

	ShaderProgram& shader;
	ShaderProgram& pickerShader;

	Camera camera;

	bool is3D = false;
	glm::vec3 dragOffset = glm::vec3(0.0f, 0.0f, 0.0f);
};
