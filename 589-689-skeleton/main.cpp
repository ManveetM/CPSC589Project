#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <functional>
#include <unordered_map>

// Window.h `#include`s ImGui, GLFW, and glad in correct order.
#include "Window.h"

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "Renderbuffer.h"

#include "GeomLoaderForOBJ.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Surface.h"

// EXAMPLE CALLBACKS
class Callbacks3D : public CallbackInterface {

public:
	// Constructor. We use values of -1 for attributes that, at the start of
	// the program, have no meaningful/"true" value.
	Callbacks3D(ShaderProgram& shader, ShaderProgram& pickerShader, int screenWidth, int screenHeight)
		: shader(shader), pickerShader(pickerShader)
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
			if (action == GLFW_PRESS)			rightMouseDown = true;
			else if (action == GLFW_RELEASE)	rightMouseDown = false;
		}

		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (action == GLFW_PRESS)			leftMouseDown = true;
			else if (action == GLFW_RELEASE) {
				isDraggingControlPoint = false;
				leftMouseDown = false;
			}
		}

		if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
			if (action == GLFW_PRESS)			middleMouseDown = true;
			else if (action == GLFW_RELEASE)	middleMouseDown = false;
		}
    }

    virtual void cursorPosCallback(double xpos, double ypos) {
		if (rightMouseDown) {
			camera.incrementTheta(ypos - mouseOldY);
			camera.incrementPhi(xpos - mouseOldX);
		}
		if (middleMouseDown) {
			float deltaX = xpos - mouseOldX;
			float deltaY = ypos - mouseOldY;
			camera.pan(deltaX, deltaY);
		}

		if (isDraggingControlPoint) {
			Frame localFrame = camera.getFrame();

			float deltaX = xpos - mouseOldX;
			float deltaY = ypos - mouseOldY;

			float distance = glm::length(controlPointPos - camera.getPos());

			newControlPointPos = glm::translate(glm::mat4(1.0f), 0.00105f * distance * deltaX * localFrame.u) * glm::vec4(controlPointPos, 1.0f);
			newControlPointPos = glm::translate(glm::mat4(1.0f), 0.00105f * distance * -deltaY * localFrame.v) * glm::vec4(newControlPointPos, 1.0f);

			controlPointPos = newControlPointPos;
			controlPointPosUpdated = true;
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
		camera.incrementR(yoffset);
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
		glm::mat4 M = glm::mat4(1.0);
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);

		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}

	void updateShadingUniforms(
		const glm::vec3& lightPos, const glm::vec3& lightCol,
		const glm::vec3& diffuseCol, float ambientStrength, bool texExistence
	)
	{
		// Like viewPipeline(), this function assumes shader.use() was called before.
		glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(lightColLoc, lightCol.r, lightCol.g, lightCol.b);
		glUniform3f(diffuseColLoc, diffuseCol.r, diffuseCol.g, diffuseCol.b);
		glUniform1f(ambientStrengthLoc, ambientStrength);
		glUniform1i(texExistenceLoc, (int)texExistence);
	}

	// Converts the cursor position from screen coordinates to GL coordinates
	// and returns the result.
	glm::vec2 getCursorPosGL() {
		glm::vec2 screenPos(mouseOldX, mouseOldY);
		// Interpret click as at centre of pixel.
		glm::vec2 centredPos = screenPos + glm::vec2(0.5f, 0.5f);
		// Scale cursor position to [0, 1] range.
		glm::vec2 scaledToZeroOne = centredPos / glm::vec2(screenWidth, screenHeight);

		glm::vec2 flippedY = glm::vec2(scaledToZeroOne.x, 1.0f - scaledToZeroOne.y);

		// Go from [0, 1] range to [-1, 1] range.
		return 2.f * flippedY - glm::vec2(1.f, 1.f);
	}

	bool isDraggingControlPoint = false;
	bool controlPointPosUpdated = false;
	glm::vec3 controlPointPos;
	glm::vec3 newControlPointPos;

	Camera camera;
private:
	// Uniform locations do not, ordinarily, change between frames.
	// However, we may need to update them if the shader is changed and recompiled.
	void updateUniformLocations() {
		mLoc = glGetUniformLocation(shader, "M");
		vLoc = glGetUniformLocation(shader, "V");
		pLoc = glGetUniformLocation(shader, "P");;
		lightPosLoc = glGetUniformLocation(shader, "lightPos");;
		lightColLoc = glGetUniformLocation(shader, "lightCol");;
		diffuseColLoc = glGetUniformLocation(shader, "diffuseCol");;
		ambientStrengthLoc = glGetUniformLocation(shader, "ambientStrength");;
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

};

// You may want to make your own class to replace this one.
class ModelInfo {
public:
	ModelInfo(std::string fileName)
		: fileName(fileName)
	{
		// Uses our .obj loader (relying on the tinyobjloader library).
		cpuGeom = GeomLoaderForOBJ::loadIntoCPUGeometry(fileName);
		gpuGeom.bind();
		gpuGeom.setVerts(cpuGeom.verts);
		gpuGeom.setNormals(cpuGeom.normals);
		gpuGeom.setUVs(cpuGeom.uvs);
	}

	void bind() { gpuGeom.bind(); }

	size_t numVerts() { return cpuGeom.verts.size(); }

	bool hasUVs() { return (cpuGeom.uvs.size() > 0); }

private:
	std::string fileName;
	CPU_Geometry cpuGeom;
	GPU_Geometry gpuGeom;
};

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 589/689"); // could set callbacks at construction if desired

	//GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");
	ShaderProgram cpShader("shaders/controlPoints.vert", "shaders/controlPoints.frag");
	ShaderProgram pickerShader("shaders/test.vert", "shaders/picker.frag");

	auto cb = std::make_shared<Callbacks3D>(shader, pickerShader, window.getWidth(), window.getHeight());
	// CALLBACKS
	window.setCallbacks(cb);

	window.setupImGui(); // Make sure this call comes AFTER GLFW callbacks set.

	// Tensor surface
	Surface splineSurface(5, 3, 3, 20, 20); // 5x5 grid, degree 3, resolution 50x50
	splineSurface.generateSurface();
	splineSurface.bind();

	// Control points
	cpShader.use();
	cb->viewPipelineControlPoints(cpShader);
	CPU_Geometry controlPointsCPU;
	std::vector<glm::vec3> flatControlPoints;

	const auto& grid = splineSurface.getControlGrid();
	for (const auto& row : grid) {
		for (const auto& pt : row) {
			flatControlPoints.push_back(pt);
		}
	}
	// Index to keep track of which control point to move
	int index = -1;
	controlPointsCPU.verts = flatControlPoints;

	GPU_Geometry controlPointsGPU;
	controlPointsGPU.setVerts(controlPointsCPU.verts);
	controlPointsGPU.setCols(std::vector<glm::vec3>(controlPointsCPU.verts.size(), glm::vec3(1.0f, 0.0f, 0.0f)));
	controlPointsGPU.bind();

	// Some variables for shading that ImGui may alter.
	glm::vec3 lightPos(0.f, 35.f, -35.f);
	glm::vec3 lightCol(1.f);
	glm::vec3 diffuseCol(1.f, 0.f, 0.f);
	float ambientStrength = 0.035f;
	bool simpleWireframe = false;

	bool texExistence = false;

	// Set the initial, default values of the shading uniforms.
	shader.use();
	cb->updateShadingUniforms(lightPos, lightCol, diffuseCol, ambientStrength, texExistence);

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();


		// Three functions that must be called each new frame.
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Sample window.");

		bool change = false;

		// If a texture is not in use, the user can pick the diffuse colour.
		if (!texExistence) change |= ImGui::ColorEdit3("Diffuse colour", glm::value_ptr(diffuseCol));

		// The rest of our ImGui widgets.
		change |= ImGui::DragFloat3("Light's position", glm::value_ptr(lightPos));
		change |= ImGui::ColorEdit3("Light's colour", glm::value_ptr(lightCol));
		change |= ImGui::SliderFloat("Ambient strength", &ambientStrength, 0.0f, 1.f);
		change |= ImGui::Checkbox("Simple wireframe", &simpleWireframe);

		// Framerate display, in case you need to debug performance.
		ImGui::Text("Average %.1f ms/frame (%.1f fps)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
		ImGui::Render();

		// __________________________________________________________________
		// Handle mouse input for control point dragging
		if (cb->isLeftMouseDown()) {
			if (!cb->isDraggingControlPoint) {
				const glm::ivec2 fbSize = window.getFramebufferSize();

				Texture pickerTex(0, GL_R32I, fbSize.x, fbSize.y, GL_RED_INTEGER, GL_INT, GL_NEAREST);
				int pickerClearValue[4] = { 0, 0, 0, 0 };

				Renderbuffer pickerRB;
				pickerRB.setStorage(GL_DEPTH_COMPONENT24, fbSize.x, fbSize.y);

				Framebuffer pickerFB;
				pickerFB.addTextureAttachment(GL_COLOR_ATTACHMENT0, pickerTex);
				pickerFB.addRenderbufferAttachment(GL_DEPTH_ATTACHMENT, pickerRB);

				auto fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
				if (fbStatus != GL_FRAMEBUFFER_COMPLETE)
				{
					Log::error("Error creating framebuffer : {}", fbStatus);
					throw std::runtime_error("Framebuffer creation error!");
				}

				glEnable(GL_LINE_SMOOTH);
				glEnable(GL_FRAMEBUFFER_SRGB);
				glEnable(GL_DEPTH_TEST);
				glDisable(GL_DITHER);

				glm::ivec2 pickPos = cb->getMousePos();
				pickPos.y = window.getHeight() - pickPos.y;

				// We'll draw each point one at a time with a unique ID
				for (size_t i = 0; i < controlPointsCPU.verts.size(); ++i) {
					pickerFB.bind();
					glClearBufferiv(GL_COLOR, 0, pickerClearValue);
					glClear(GL_DEPTH_BUFFER_BIT);

					glEnable(GL_SCISSOR_TEST);
					glScissor(pickPos.x, pickPos.y, 1, 1);

					pickerShader.use();

					cb->viewPipelinePicker();
					controlPointsGPU.bind();

					glPointSize(15);
					glDrawArrays(GL_POINTS, i, 1);

					GLint pickTexCPU[1];
					pickerTex.bind();
					glReadPixels(pickPos.x, pickPos.y, 1, 1, GL_RED_INTEGER, GL_INT, pickTexCPU);

					pickerFB.unbind();
					glDisable(GL_SCISSOR_TEST);

					if (pickTexCPU[0] == 1) {
						index = i;

						cb->isDraggingControlPoint = true;
						cb->controlPointPos = controlPointsCPU.verts[i];
						break;
					}
				}
				glEnable(GL_DITHER);
			}
		}

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, (simpleWireframe ? GL_LINE : GL_FILL));

		// __________________________________________________________________
		// Update the control point position if the user is dragging it

		if (cb->controlPointPosUpdated) {
			splineSurface.setControlPoint(index, cb->newControlPointPos);

			flatControlPoints.clear();
			const auto& grid = splineSurface.getControlGrid();
			for (const auto& row : grid) {
				for (const auto& pt : row) {
					flatControlPoints.push_back(pt);
				}
			}

			controlPointsCPU.verts = flatControlPoints;
			controlPointsGPU.setVerts(controlPointsCPU.verts);
			controlPointsGPU.bind();

			splineSurface.generateSurface();
		}

		// __________________________________________________________________
		// Draw Control Points
		cpShader.use();
		cb->viewPipelineControlPoints(cpShader);
		controlPointsGPU.bind();
		glPointSize(10);
		glDrawArrays(GL_POINTS, 0, controlPointsCPU.verts.size());

		// __________________________________________________________________
		// Draw the surface
		shader.use();
		if (change)
		{
			// If any of our shading values was updated, we need to update the
			// respective GLSL uniforms.
			cb->updateShadingUniforms(lightPos, lightCol, diffuseCol, ambientStrength, texExistence);
		}
		cb->viewPipeline();

		splineSurface.bind();
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLES, 0, splineSurface.numVerts());


		// __________________________________________________________________
		// __________________________________________________________________

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		window.swapBuffers();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}
