#pragma once

#include "Geometry.h"
#include "Texture.h"
#include "Window.h"
#include "Log.h"

#include "RenderBuffer.h"
#include "FrameBuffer.h"
#include "Callback.h"
#include "ShaderProgram.h"
#include "Surface.h"
#include "Plant.h"
#include "PlantPart.h"

#include <unordered_map>
#include <iostream>
#include <string.h>


class Scene {
public:
	enum class ShaderType {
		DEFAULT,
		PICKER,
		CONTROL_POINTS,
	};

	Scene(Window& window_, std::shared_ptr<Callbacks3D> callbacks,
		std::unordered_map<std::string, ShaderProgram*>& shaders_)
		: window(window_)
		, shaders(shaders_)
		, activeShader(shaders_.at("default"))
		, cb(callbacks)
		, pickerTex(0, GL_R32I, window_.getFramebufferSize().x, window_.getFramebufferSize().y, GL_RED_INTEGER, GL_INT, GL_NEAREST)
		, landscape(5, 3, 3, 20, 20)
	{
		initialize();
		initializeGpuPicking();
	}

	void updateScene();

	void draw();


private:
	Window& window;

	ShaderProgram* activeShader;
	std::unordered_map<std::string, ShaderProgram*>& shaders;

	std::shared_ptr<Callbacks3D> cb;

	Texture pickerTex;
	Renderbuffer pickerRB;
	Framebuffer pickerFB;
	int pickerClearValue[4] = { 0, 0, 0, 0 };

	Surface landscape;
	std::vector<Plant> plants;
	// __________________________________________________________________
	// __________________________________________________________________

	void setShader(ShaderType type);
	void initializeGpuPicking();
	void drawImGui();
	void initialize();
	void initializeLandscape();
	void handleGPUPickingLandscape();
	void drawLandscape();
	void drawAxes(const char* shaderType);
	void drawLandscapeControlPoints();
	void updateLandscapeState();
	void drawEditingImGui();
	void previewPlants();
	std::vector<glm::vec3> updateBSpline(PointsData& controlPoints);
	glm::vec3 E_delta_1(const std::vector<glm::vec3>& ctrlPts, const std::vector<float>& weights, const std::vector<double>& U, float u, int k, int m);
	std::vector<double> getKnotSequence(int k, int m);
	void drawCurves();
	void drawControlPoints();
	void handleEditingControlPointUpdate(PointsData& cp);
	void applyBrushDeformation();

	// __________________________________________________________________
	// STATES

	int comboSelection = 0;
	bool modeChanged = false;
	bool isDrawingLandscape = false;
	bool lightingChange = false;
	bool show3DAxes = false;
	int controlPointIndex = -1;

	float brushRadius = 1.5f;
	float brushStrength = 0.1f;
	float noiseScale = 0.5f;
	float noiseAmplitude = 0.2f;
	bool brushRaise = true;
	bool brushEnabled = false;

	// editing
	int selectedPlantIndex = -1;
	int selectedPartIndex = -1;
	bool previewingPart = false;
	bool previewingPlant = false;

	bool showLeftCurve = false;
	bool showRightCurve = false;
	bool showCrossSection = false;

	// __________________________________________________________________
	// IMGUI

	const char* options[2] = { "Default", "Editing" };

	// __________________________________________________________________
	// LIGHTING
	glm::vec3 lightPos = glm::vec3(0.f, 5.f, 8.f);
	glm::vec3 lightCol = glm::vec3(1.f);
	glm::vec3 diffuseCol = glm::vec3(1.0f, 1.1f, 1.1f);
	float ambientStrength = 0.035f;
	bool simpleWireframe = false;
};
