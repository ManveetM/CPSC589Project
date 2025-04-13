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
#include "Scene.h"
#include "Callback.h"
#include "Plant.h"
#include "PlantPart.h"

#include "GeomLoaderForOBJ.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Surface.h"
#include "PointsData.h"

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(1200, 1200, "CPSC 589/689"); // could set callbacks at construction if desired

	//GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");
	ShaderProgram cpShader("shaders/controlPoints.vert", "shaders/controlPoints.frag");
	ShaderProgram editingShader("shaders/editing.vert", "shaders/editing.frag");
	ShaderProgram pickerShader("shaders/test.vert", "shaders/picker.frag");

	auto cb = std::make_shared<Callbacks3D>(shader, pickerShader, window.getWidth(), window.getHeight());
	// CALLBACKS
	window.setCallbacks(cb);

	window.setupImGui(); // Make sure this call comes AFTER GLFW callbacks set.

	std::unordered_map<std::string, ShaderProgram*> shaders = {
		{"default", &shader},
		{"controlPoint", &cpShader},
		{"picker", &pickerShader},
		{"editing", &editingShader}
	};

	Scene scene(window, cb, shaders);

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		scene.updateScene();
		scene.draw();

		window.swapBuffers();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}
