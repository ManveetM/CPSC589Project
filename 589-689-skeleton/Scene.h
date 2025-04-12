#pragma once

#include "Geometry.h"
#include "Texture.h"
#include "Window.h"
#include "Log.h"

#include "RenderBuffer.h"
#include "FrameBuffer.h"

#include <unordered_map>
#include <iostream>


class Scene {
public:
	enum class ShaderType {
		DEFAULT,
		PICKER,
		CONTROL_POINTS,
	};

	Scene(Window& window_, std::shared_ptr<Callback3D> callbacks,
		std::unordered_map<std::string, ShaderProgram*>& shaders_)
		: window(window_), panel(panel_), shaders(shaders_), skybox(skyBoxData.faces, GL_LINEAR)
		, activeShader(shaders_.at("default")), callbacks(callbacks),
		pickerTex(0, GL_R32I, window_.getFramebufferSize().x, window_.getFramebufferSize().y, GL_RED_INTEGER, GL_INT, GL_NEAREST)
	{
		skybox_cpu.verts = skyBoxData.skyboxVertices;
		grid.generateAxes();
		grid.generateGridPlanes();

		initializeGpuPicking();
		initializeControlPoints();
	}

	void refresh() {
		//updateGrid();
		//draw();
		handleGpuPicking();
	}

private:
	Window& window;
	PreferencesPanel& panel;
	Grid grid;
	SkyBox skyBoxData;
	CPU_Geometry skybox_cpu;
	Texture skybox;

	ShaderProgram* activeShader;
	std::unordered_map<std::string, ShaderProgram*>& shaders;

	std::shared_ptr<Callback> callbacks;

	Texture pickerTex;
	Renderbuffer pickerRB;
	Framebuffer pickerFB;
	int pickerClearValue[4] = { 0, 0, 0, 0 };


	// __________________________________________________________________

	// B-spline surface control
	std::vector<std::vector<glm::vec3>> controlPoints;         // 2D grid of control points
	std::vector<std::vector<glm::vec3>> controlPointColors;    // Unique color per control point for picking
	std::vector<float> knotU, knotV;                           // Knot vectors for tensor product surface
	int degreeU = 3;                                            // Degree in U direction (default cubic)
	int degreeV = 3;                                            // Degree in V direction

	int numCtrlPtsU = 10;                                       // Default number of control points in U
	int numCtrlPtsV = 10;                                       // Default number of control points in V

	// GPU buffer for rendering control points
	CPU_Geometry cp_cpu;
	GPU_Geometry cp_gpu;

	// Selection state
	int selectedRow = -1;
	int selectedCol = -1;
	glm::vec3 selectedLocalFrameU = glm::vec3(1, 0, 0);
	glm::vec3 selectedLocalFrameV = glm::vec3(0, 1, 0);
	glm::vec3 selectedNormal = glm::vec3(0, 0, 1);              // Z-up by default

	// Influence radius & falloff
	float influenceRadius = 2.0f;                               // In control point grid units
	float falloffPower = 2.0f;                                  // Controls how fast influence decays

	// Geometry for surface mesh (optional: build later)
	GPU_Geometry surfaceGeom;
	bool surfaceNeedsUpdate = true;                            // Dirty flag for surface regeneration

	// __________________________________________________________________

	void updateGrid() {
		if (panel.getGridDensityChanged()) {
			grid.clear();
			grid.density = panel.getGridDensity();
			grid.generateGridPlanes();
			panel.resetGridDensityChanged();
		}

		grid.areAxesVisible = panel.shouldShowAxes();
		grid.isXYGridPlaneVisible = panel.shouldShowXYPlane();
		grid.isXZGridPlaneVisible = panel.shouldShowXZPlane();
		grid.isYZGridPlaneVisible = panel.shouldShowYZPlane();
	}

	void setShader(ShaderType type)
	{
		switch (type) {
		case ShaderType::DEFAULT:
			if (shaders.count("default")) {
				activeShader = shaders.at("default");
			}
			break;
		case ShaderType::SKYBOX:
			if (shaders.count("skybox")) {
				activeShader = shaders.at("skybox");
			}
			break;
		case ShaderType::PICKER:
			if (shaders.count("picker")) {
				activeShader = shaders.at("picker");
			}
			break;
		}
	}

	void useShader() {
		activeShader->use();
	}

	void draw() {

		glDepthMask(GL_FALSE);

		// SKY BOX DRAW
		setShader(ShaderType::SKYBOX);
		useShader();
		callbacks->viewPipeline(*activeShader, skyBoxData.model);

		skybox.bind();
		GPU_Geometry skybox_gpu;
		skybox_gpu.setVerts(skybox_cpu.verts);
		skybox_gpu.bind();

		glDrawArrays(GL_TRIANGLES, 0, skybox_cpu.verts.size());

		// DRAW REST OF THE SCENE
		setShader(ShaderType::DEFAULT);
		useShader();
		callbacks->viewPipeline(*activeShader);
		grid.drawGridPlanes();
		grid.drawAxes();

		glDepthMask(GL_TRUE);

		// DRAW CONTROL POINTS
		if (!controlPoints.empty()) {
			updateControlPointGeom(false); // normal rendering
			cp_gpu.bind();
			glPointSize(10.0f); // for visibility
			glDrawArrays(GL_POINTS, 0, cp_cpu.verts.size());
		}
	}

	void initializeControlPoints();
	void updateControlPointGeom(bool forPicking = false);
	void handleGpuPicking();
	void initializeGpuPicking();
};
