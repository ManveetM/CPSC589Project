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
//#include "Plant.h"
//#include "PlantPart.h"

#include "GeomLoaderForOBJ.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Surface.h"

 struct PointsData {
 	CPU_Geometry cpuGeom;

 	std::vector<bool> selected;
 	std::vector<float> weights;
 	bool needsUpdate = true;

 	void clear() {
 		cpuGeom.verts.clear();
 		cpuGeom.cols.clear();
 		selected.clear();
 		weights.clear();
 	}
 };

 struct PlantPart {
 	std::string name;

 	PointsData leftControlPoints;
 	PointsData rightControlPoints;
 	PointsData crossSectionControlPoints;

 	std::vector<glm::vec3> leftCurve;
 	std::vector<glm::vec3> rightCurve;
 	std::vector<glm::vec3> crossSectionCurve;

 	bool needsUpdate = false;
 	std::vector<glm::vec3> surface;

 	glm::vec3 scale;
 	glm::vec3 translation;
 	glm::vec3 rotation;

 	glm::mat4 partMatrix;

 	PlantPart(std::string name)
 		: name(name)
 		, scale(1.0f, 1.0f, 1.0f)
 		, translation(0.0f, 0.0f, 0.0f)
 		, rotation(0.0f, 0.0f, 0.0f)
 		, partMatrix(glm::mat4(1.0f))
 	{
 	}
 };

 struct Plant {
 	std::string name;
 	std::vector<PlantPart> parts;
 	glm::mat4 modelMatrix;

 	Plant(const std::string& name)
 		: name(name), parts(parts), modelMatrix(modelMatrix) {
 	}

 	void addPart(PlantPart part) {
 		parts.push_back(part);
 	}

 	void removePart(int index) {
 		if (index >= 0 && index < parts.size()) {
 			parts.erase(parts.begin() + index);
 		}
 	}
 };

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
		, cp(nullptr)
	{
		updateUniformLocations();
	}

	glm::ivec2 getMousePos() {
		return glm::ivec2(mouseOldX, mouseOldY);
	}

	bool isLeftMouseDown() {
		return leftMouseDown;
	}

	void setMode(int mode_) {
		mode = mode_;
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

		float screenX = ((mouseOldX / screenWidth) * 2.0) - 1.0;
		float screenY = 1.0 - ((mouseOldY / screenHeight) * 2.0);

		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS) {
				if (mode == 1 && !previewingPart && !previewingPlant) {
					//Remove a point
					int indexToDelete = -1;

					if (cp == nullptr) return;

					for (int i = 0; i < cp->cpuGeom.verts.size(); i++) {
						glm::vec3 delta = (cp->cpuGeom.verts[i] - glm::vec3(screenX, screenY, 0.f));
						if (glm::length(delta) < 0.025f) {
							indexToDelete = i;
							break;
						}
					}

					if (indexToDelete != -1) {
						cp->cpuGeom.verts.erase(cp->cpuGeom.verts.begin() + indexToDelete);
						cp->cpuGeom.cols.erase(cp->cpuGeom.cols.begin() + indexToDelete);
						cp->selected.erase(cp->selected.begin() + indexToDelete);
						cp->weights.erase(cp->weights.begin() + indexToDelete);
						cp->needsUpdate = true;
					}
				}
				rightMouseDown = true;
			}
			else if (action == GLFW_RELEASE) {
				rightMouseDown = false;
			}
		}

		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (action == GLFW_PRESS) {
				leftMouseDown = true;

				if (cp == nullptr) return;

				int indexToMove = -1;
				for (int i = 0; i < cp->cpuGeom.verts.size(); i++) {
					glm::vec3 delta = (cp->cpuGeom.verts[i] - glm::vec3(screenX, screenY, 0.f));
					if (glm::length(delta) < 0.05f) {
						indexToMove = i;

						std::fill(cp->cpuGeom.cols.begin(), cp->cpuGeom.cols.end(), glm::vec3(1.0f, 0.0f, 0.0f));
						std::fill(cp->selected.begin(), cp->selected.end(), false);

						cp->cpuGeom.cols.at(i) = glm::vec3(0.0f, 1.0f, 0.0f);
						cp->selected.at(i) = true;

						break;
					}
				}

				if (indexToMove == -1) {
					// Add a new point
					std::fill(cp->cpuGeom.cols.begin(), cp->cpuGeom.cols.end(), glm::vec3(1.0f, 0.0f, 0.0f));
					std::fill(cp->selected.begin(), cp->selected.end(), false);

					cp->cpuGeom.verts.push_back(glm::vec3(screenX, screenY, 0.f));
					cp->cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f));
					cp->selected.push_back(true);
					cp->weights.push_back(1.0f);
				}
				else {
					// Move existing point
					movingPointIndex = indexToMove;
				}
				cp->needsUpdate = true;
			}
			else if (action == GLFW_RELEASE) {
				isDraggingControlPoint = false;
				leftMouseDown = false;
				movingPointIndex = -1;
			}
		}

		if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
			if (action == GLFW_PRESS)			middleMouseDown = true;
			else if (action == GLFW_RELEASE)	middleMouseDown = false;
		}
	}

	virtual void cursorPosCallback(double xpos, double ypos) {
		float screenX = ((mouseOldX / screenWidth) * 2.0) - 1.0;
		float screenY = 1.0 - ((mouseOldY / screenHeight) * 2.0);

		if (rightMouseDown) {
			camera.incrementTheta(ypos - mouseOldY);
			camera.incrementPhi(xpos - mouseOldX);
		}
		if (middleMouseDown) {
			float deltaX = xpos - mouseOldX;
			float deltaY = ypos - mouseOldY;
			camera.pan(deltaX, deltaY, screenWidth, screenHeight);
		}

		if (isDraggingControlPoint) {
			Frame localFrame = camera.getFrame();

			float deltaX = xpos - mouseOldX;
			float deltaY = ypos - mouseOldY;

			float ndx = deltaX / (float)screenWidth;
			float ndy = deltaY / (float)screenHeight;

			float distance = glm::length(controlPointPos - camera.getPos());

			float scale = 2.0f * distance * tan(glm::radians(45.0f) * 0.5f); // Use actual camera FOV here if available

			glm::vec3 offset = ndx * scale * aspect * localFrame.u - ndy * scale * localFrame.v;

			controlPointPos += offset;
			controlPointPosUpdated = true;
		}
		// }
		// else if (mode == 1) {
		if (leftMouseDown && movingPointIndex != -1) {
			//Update point at index i
			cp->cpuGeom.verts[movingPointIndex] = glm::vec3(screenX, screenY, 0.f);
			cp->needsUpdate = true;
			// }
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

	void viewPipelinePlantPreview(ShaderProgram& sp, glm::mat4& modelMatrix) {
		glm::mat4 M = modelMatrix;
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);

		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}

	void viewPipelineEditing(ShaderProgram& sp) {
		glm::mat4 M = glm::mat4(1.0f);
		glm::mat4 V = glm::mat4(1.0f);
		glm::mat4 P = glm::mat4(1.0f);
	
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

	void setEditingControlPoint(PointsData* cp_) {
		cp = cp_;
	}

	bool isDraggingControlPoint = false;
	bool controlPointPosUpdated = false;
	bool previewingPart = false;
	bool previewingPlant = false;
	glm::vec3 controlPointPos;

	int mode;

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

	PointsData* cp;
	int movingPointIndex = -1;

};

std::vector<double> getKnotSequence(int k, int m);
void previewPlantPart(std::shared_ptr<Callbacks3D> cb, PlantPart& part, ShaderProgram& cpShader);
void previewPlant(std::shared_ptr<Callbacks3D> cb, Plant& plant, ShaderProgram& cpShader);

glm::vec3 E_delta_1(const std::vector<glm::vec3>& ctrlPts, const std::vector<float>& weights, const std::vector<double>& U, float u, int k, int m);

void updateBSpline(PointsData& controlPoints, std::vector<glm::vec3>& bSpline) {
	int size = controlPoints.cpuGeom.verts.size();
	if (size > 1) {
		int k = size == 2 ? 2 : 3;
		int m = size - 1;
		float uStep = 0.02f;

		std::vector<double> knotSequence = getKnotSequence(k, m);

		bSpline.clear();

		for (float u = 0.0f; u < 1.0f - 1e-4; u += uStep) {
			glm::vec3 point = E_delta_1(controlPoints.cpuGeom.verts, controlPoints.weights, knotSequence, u, k, m);
			bSpline.push_back(point);
		}
		glm::vec3 point = E_delta_1(controlPoints.cpuGeom.verts, controlPoints.weights, knotSequence, 1.0f, k, m);
		bSpline.push_back(point);
	}
}

void handleEditingMode(std::shared_ptr<Callbacks3D> cb, std::vector<Plant>& plants, ShaderProgram& editingShader, ShaderProgram& cpShader) {

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	static int selectedPlantIndex = -1;
	static int selectedPartIndex = -1;
	static bool previewingPart = false;
	static bool previewingPlant = false;

	if (!previewingPlant && !previewingPart) {
		// Draw X, Y axes with different colors
		std::vector<glm::vec3> axisVerts = {
			glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), // X-axis
			glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), // Y-axis
		};

		std::vector<glm::vec3> axisColors = {
			glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), // Red for X-axis
			glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), // Green for Y-axis
		};

		editingShader.use();
		cb->viewPipelineEditing(editingShader);

		GPU_Geometry axisGeom;
		axisGeom.setVerts(axisVerts);
		axisGeom.setCols(axisColors);
		axisGeom.bind();
		glLineWidth(2.0f);
		glDrawArrays(GL_LINES, 0, axisVerts.size());


		if (selectedPlantIndex >= 0 && selectedPartIndex >= 0) {
			auto& selectedPart = plants[selectedPlantIndex].parts[selectedPartIndex];

			GPU_Geometry gpuGeom;

			gpuGeom.setVerts(selectedPart.leftCurve);
			gpuGeom.setCols(std::vector<glm::vec3>(selectedPart.leftCurve.size(), glm::vec3(1.0f, 0.0f, 0.0f)));
			gpuGeom.bind();
			glDrawArrays(GL_LINE_STRIP, 0, selectedPart.leftCurve.size());

			gpuGeom.setVerts(selectedPart.rightCurve);
			gpuGeom.setCols(std::vector<glm::vec3>(selectedPart.rightCurve.size(), glm::vec3(0.0f, 0.0f, 1.0f)));
			gpuGeom.bind();
			glDrawArrays(GL_LINE_STRIP, 0, selectedPart.rightCurve.size());

			gpuGeom.setVerts(selectedPart.crossSectionCurve);
			gpuGeom.setCols(std::vector<glm::vec3>(selectedPart.crossSectionCurve.size(), glm::vec3(0.0f, 0.0f, 1.0f)));
			gpuGeom.bind();
			glDrawArrays(GL_LINE_STRIP, 0, selectedPart.crossSectionCurve.size());

			glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
		}
		
	}

	if (ImGui::BeginCombo("Plants", selectedPlantIndex >= 0 ? plants[selectedPlantIndex].name.c_str() : "Select a Plant")) {
		for (int i = 0; i < plants.size(); ++i) {
			bool isSelected = (selectedPlantIndex == i);
			if (ImGui::Selectable(plants[i].name.c_str(), isSelected)) {
				selectedPlantIndex = i;
				selectedPartIndex = -1;
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	if (ImGui::Button("Delete Plant")) {
		if (!plants.empty() && selectedPlantIndex < plants.size()) {
			plants.erase(plants.begin() + selectedPlantIndex);
			if (selectedPlantIndex >= plants.size()) {
				selectedPlantIndex = -1;
			}
			selectedPartIndex = -1;
		}
	}

	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	if (selectedPlantIndex >= 0) {
		const auto& selectedPlant = plants[selectedPlantIndex];
		if (ImGui::BeginCombo("Parts", selectedPartIndex >= 0 ? selectedPlant.parts[selectedPartIndex].name.c_str() : "Select a Part")) {
			for (int i = 0; i < selectedPlant.parts.size(); ++i) {
				bool isSelected = (selectedPartIndex == i);
				if (ImGui::Selectable(selectedPlant.parts[i].name.c_str(), isSelected)) {
					cb->setEditingControlPoint(nullptr);
					selectedPartIndex = i;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::Button("Delete Part")) {
			std::cout << "Attempting to delete part..." << std::endl;

			cb->setEditingControlPoint(nullptr);

			if (!plants.empty() && selectedPlantIndex < plants.size()) {
				auto& parts = plants[selectedPlantIndex].parts;

				std::cout << "Selected plant: " << plants[selectedPlantIndex].name << std::endl;

				if (!parts.empty() && selectedPartIndex < parts.size()) {
					std::cout << "Deleting part: " << parts[selectedPartIndex].name << std::endl;

					parts.erase(parts.begin() + selectedPartIndex);
					selectedPartIndex = -1;

					std::cout << "Part deleted successfully." << std::endl;
				}
			}
		}

		ImGui::Dummy(ImVec2(0.0f, 5.0f));
		static char partName[64] = "";
		ImGui::InputText("Part Name", partName, sizeof(partName));
		if (ImGui::Button("Add New Part")) {
			std::cout << "Adding new part: " << partName << std::endl;

			if (strlen(partName) > 0 && selectedPlantIndex < plants.size()) {
				auto& parts = plants[selectedPlantIndex].parts;

				std::cout << "Checking for existing part names..." << std::endl;
				for (auto& part : parts) {
					if (part.name == partName) {
						std::cout << "Another part with the same name already exists" << std::endl;
						return;
					}
				}

				PlantPart newPart(partName);
				plants[selectedPlantIndex].addPart(newPart);

				std::cout << "New part added: " << partName << std::endl;
			}
		}
	}

	if (selectedPlantIndex >= 0 && selectedPartIndex >= 0 && !previewingPlant && !previewingPart) {
		auto& selectedPart = plants[selectedPlantIndex].parts[selectedPartIndex];

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::Text("-------------------------------");
		ImGui::Text("Editing Part: %s", plants[selectedPlantIndex].parts[selectedPartIndex].name.c_str());
		if (ImGui::Button("Clear")) {
			previewingPlant = false;
			previewingPart = false;
			cb->setEditingControlPoint(nullptr);

			selectedPart.leftControlPoints.clear();
			selectedPart.rightControlPoints.clear();
			selectedPart.crossSectionControlPoints.clear();
			selectedPart.surface.clear();

			selectedPart.leftCurve.clear();
			selectedPart.rightCurve.clear();
			selectedPart.crossSectionCurve.clear();

			selectedPart.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
			selectedPart.scale = glm::vec3(1.0f, 1.0f, 1.0f);
			selectedPart.translation = glm::vec3(0.0f, 0.0f, 0.0f);
		}

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		static bool showLeftCurve = false;
		static bool showRightCurve = false;
		static bool showCrossSection = false;

		ImGui::Checkbox("Left Curve", &showLeftCurve);
		if (showLeftCurve && !previewingPart && !previewingPlant) {
			cb->setEditingControlPoint(&selectedPart.leftControlPoints);

			int index = -1;
			for (int i = 0; i < selectedPart.leftControlPoints.selected.size(); ++i) {
				if (selectedPart.leftControlPoints.selected.at(i)) {
					index = i;
				}
			}

			if (index != -1) {
				float weight = selectedPart.leftControlPoints.weights.at(index);
				bool weightChanged = ImGui::SliderFloat("Weight", &weight, 0.0f, 20.0f);

				if (weightChanged) {
					selectedPart.leftControlPoints.weights.at(index) = weight;
					updateBSpline(selectedPart.leftControlPoints, selectedPart.leftCurve);
				}
			}

			if (selectedPart.leftControlPoints.needsUpdate) {
				updateBSpline(selectedPart.leftControlPoints, selectedPart.leftCurve);
				selectedPart.leftControlPoints.needsUpdate = false;
			}

			showRightCurve = false;
			showCrossSection = false;

			GPU_Geometry gpuGeom;
			gpuGeom.setVerts(selectedPart.leftControlPoints.cpuGeom.verts);
			gpuGeom.setCols(selectedPart.leftControlPoints.cpuGeom.cols);
			gpuGeom.bind();

			editingShader.use();
			cb->viewPipelineEditing(editingShader);
			gpuGeom.bind();
			glPointSize(10);
			glDrawArrays(GL_POINTS, 0, selectedPart.leftControlPoints.cpuGeom.verts.size());

			glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
		}

		ImGui::Checkbox("Right Curve", &showRightCurve);
		if (showRightCurve && !previewingPart && !previewingPlant) {
			cb->setEditingControlPoint(&selectedPart.rightControlPoints);

			int index = -1;
			for (int i = 0; i < selectedPart.rightControlPoints.selected.size(); ++i) {
				if (selectedPart.rightControlPoints.selected.at(i)) {
					index = i;
				}
			}

			if (index != -1) {
				float weight = selectedPart.rightControlPoints.weights.at(index);
				bool weightChanged = ImGui::SliderFloat("Weight", &weight, 0.0f, 20.0f);

				if (weightChanged) {
					selectedPart.rightControlPoints.weights.at(index) = weight;
					updateBSpline(selectedPart.rightControlPoints, selectedPart.rightCurve);
				}
			}

			if (selectedPart.rightControlPoints.needsUpdate) {
				updateBSpline(selectedPart.rightControlPoints, selectedPart.rightCurve);
				selectedPart.rightControlPoints.needsUpdate = false;
			}

			showLeftCurve = false;
			showCrossSection = false;

			GPU_Geometry gpuGeom;
			gpuGeom.setVerts(selectedPart.rightControlPoints.cpuGeom.verts);
			gpuGeom.setCols(selectedPart.rightControlPoints.cpuGeom.cols);
			gpuGeom.bind();

			editingShader.use();
			cb->viewPipelineEditing(editingShader);
			gpuGeom.bind();
			glPointSize(10);
			glDrawArrays(GL_POINTS, 0, selectedPart.rightControlPoints.cpuGeom.verts.size());

			glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
		}

		ImGui::Checkbox("Cross Section", &showCrossSection);
		if (showCrossSection && !previewingPart && !previewingPlant) {
			cb->setEditingControlPoint(&selectedPart.crossSectionControlPoints);

			int index = -1;
			for (int i = 0; i < selectedPart.crossSectionControlPoints.selected.size(); ++i) {
				if (selectedPart.crossSectionControlPoints.selected.at(i)) {
					index = i;
				}
			}

			if (index != -1) {
				float weight = selectedPart.crossSectionControlPoints.weights.at(index);
				bool weightChanged = ImGui::SliderFloat("Weight", &weight, 0.0f, 20.0f);

				if (weightChanged) {
					selectedPart.crossSectionControlPoints.weights.at(index) = weight;
					updateBSpline(selectedPart.crossSectionControlPoints, selectedPart.crossSectionCurve);
				}
			}

			if (selectedPart.crossSectionControlPoints.needsUpdate) {
				updateBSpline(selectedPart.crossSectionControlPoints, selectedPart.crossSectionCurve);
				selectedPart.crossSectionControlPoints.needsUpdate = false;
			}

			showLeftCurve = false;
			showRightCurve = false;

			GPU_Geometry gpuGeom;
			gpuGeom.setVerts(selectedPart.crossSectionControlPoints.cpuGeom.verts);
			gpuGeom.setCols(selectedPart.crossSectionControlPoints.cpuGeom.cols);
			gpuGeom.bind();

			editingShader.use();
			cb->viewPipelineEditing(editingShader);
			gpuGeom.bind();
			glPointSize(10);
			glDrawArrays(GL_POINTS, 0, selectedPart.crossSectionControlPoints.cpuGeom.verts.size());

			glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
		}

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::Text("Transformations");
		ImGui::DragFloat3("Scale", glm::value_ptr(plants[selectedPlantIndex].parts[selectedPartIndex].scale), 0.1f, 0.1f, 10.0f);
		ImGui::DragFloat3("Translation", glm::value_ptr(plants[selectedPlantIndex].parts[selectedPartIndex].translation), 0.1f, -10.0f, 10.0f);
		ImGui::DragFloat3("Rotation (degrees)", glm::value_ptr(plants[selectedPlantIndex].parts[selectedPartIndex].rotation), 0.1f, -180.0f, 180.0f);

		if (ImGui::Button("Preview Part")) {
			// Error checking: Ensure all three curves are set
			if (selectedPart.leftCurve.empty() || selectedPart.rightCurve.empty() || selectedPart.crossSectionCurve.empty()) {
				std::cout << "Error: All three curves (left, right, cross-section) must be set before calculating the surface." << std::endl;
			}
			else {
				if (selectedPart.leftCurve.size() != selectedPart.rightCurve.size()) {
					std::cout << "Error: Left and Right curves must have the same number of points." << std::endl;
				}
				else {
					std::cout << "Part previewing..." << selectedPart.name << std::endl;

					showLeftCurve = false;
					showRightCurve = false;
					showCrossSection = false;

					previewingPart = true;
					cb->previewingPart = true;
					cb->setEditingControlPoint(nullptr);
				}
			}
		}

		if (ImGui::Button("Preview Plant")) {

			for (auto& part : plants[selectedPlantIndex].parts) {
				if (part.leftCurve.empty() || part.rightCurve.empty() || part.crossSectionCurve.empty()) {
					std::cout << "Error: All three curves (left, right, cross-section) must be set before calculating the surface." << std::endl;
					return;
				}
				if (part.leftCurve.size() != part.rightCurve.size()) {
					std::cout << "Error: All left and right curve sizes must be equal" << std::endl;
				}
			}
			
			std::cout << "Plant previewing..." << selectedPart.name << std::endl;

			showLeftCurve = false;
			showRightCurve = false;
			showCrossSection = false;

			previewingPlant = true;
			cb->previewingPart = true;
			cb->setEditingControlPoint(nullptr);
		}
	}

	if (previewingPart) {
		previewPlantPart(cb, plants[selectedPlantIndex].parts[selectedPartIndex], cpShader);
		if (ImGui::Button("Edit Surface")) {
			previewingPart = false;
			cb->previewingPart = false;
		}
	} else if (previewingPlant) {
		previewPlant(cb, plants[selectedPlantIndex], cpShader);
		if (ImGui::Button("Edit Surface")) {
			previewingPlant = false;
			cb->previewingPlant = false;
		}
	}
}

std::vector<glm::vec3> generatePlantPart(PlantPart& part) {
    std::vector<glm::vec3> surface;

    std::vector<glm::dvec3> transformedCurve(part.crossSectionCurve.begin(), part.crossSectionCurve.end());
    glm::dvec3 startPoint = transformedCurve.front();
    glm::dvec3 endPoint = transformedCurve.back();
    glm::dvec3 midpoint = (startPoint + endPoint) * 0.5;

    glm::dmat4 translationMatrix = glm::translate(glm::dmat4(1.0), -midpoint);
    for (auto& point : transformedCurve) {
        glm::dvec4 translatedPoint = translationMatrix * glm::dvec4(point, 1.0);
        point = glm::dvec3(translatedPoint);
    }

    // Recalculate start and end points after translation
    startPoint = transformedCurve.front();
    endPoint = transformedCurve.back();

    glm::dvec3 direction = glm::normalize(endPoint - startPoint);
    glm::dvec3 xAxis(1.0, 0.0, 0.0);
    double angle = acos(glm::clamp(glm::dot(direction, xAxis), -1.0, 1.0));

    // Rotate the curve around the z-axis and x-axis
    glm::dmat4 rotationMatrix = glm::rotate(glm::dmat4(1.0), angle, glm::dvec3(0.0, 0.0, 1.0));
    rotationMatrix = glm::rotate(glm::dmat4(1.0), glm::radians(-90.0), xAxis) * rotationMatrix;
    for (auto& point : transformedCurve) {
        glm::dvec4 rotatedPoint = rotationMatrix * glm::dvec4(point, 1.0);
        point = glm::dvec3(rotatedPoint);
    }

    startPoint = transformedCurve.front();
    endPoint = transformedCurve.back();
    double length = glm::length(endPoint - startPoint);
    if (length > 1e-4) {
        double scaleFactor = 1.0 / length;

        glm::dmat4 scaleMatrix = glm::scale(glm::dmat4(1.0), glm::dvec3(scaleFactor));
        for (auto& point : transformedCurve) {
            glm::dvec4 scaledPoint = scaleMatrix * glm::dvec4(point, 1.0);
            point = glm::dvec3(scaledPoint);
        }

        int index = transformedCurve.size() - 1;
        std::vector<glm::dvec3> reflectedPoints;
        for (size_t i = 1; i < transformedCurve.size() - 1; ++i) {
            glm::dvec3 reflectedPoint = transformedCurve[i];
            reflectedPoint.z = -reflectedPoint.z;
            reflectedPoints.push_back(reflectedPoint);
        }
        transformedCurve.insert(transformedCurve.end(), reflectedPoints.rbegin(), reflectedPoints.rend());

        std::vector<std::vector<glm::dvec3>> surfaceStrips;

        for (int i = 0; i < part.leftCurve.size(); ++i) {
            glm::dvec3 ql = part.leftCurve[i];
            glm::dvec3 qr = part.rightCurve[i];
            glm::dvec3 axis = qr - ql;
            glm::dvec3 mid = (ql + qr) * 0.5;

            glm::dvec3 axisDir = glm::normalize(axis);
            double angle = acos(glm::clamp(glm::dot(glm::dvec3(1.0, 0.0, 0.0), axisDir), -1.0, 1.0));

            glm::dmat4 transform = glm::translate(glm::dmat4(1.0), mid)
                * glm::rotate(glm::dmat4(1.0), -angle, glm::dvec3(0.0, 0.0, 1.0))
                * glm::scale(glm::dmat4(1.0), glm::dvec3(glm::length(axis)));

            std::vector<glm::dvec3> strip;

            for (const auto& pt : transformedCurve) {
                glm::dvec4 transformed = transform * glm::dvec4(pt, 1.0);
                strip.push_back(glm::dvec3(transformed));
            }

            surfaceStrips.push_back(strip);
        }

        int N = surfaceStrips.size();
        int M = transformedCurve.size();

        for (int i = 0; i < N - 1; ++i) {
            for (int j = 0; j < M; ++j) {
                glm::dvec3 v0 = surfaceStrips[i][j];
                glm::dvec3 v1 = surfaceStrips[i + 1][j];
                glm::dvec3 v2 = surfaceStrips[i][(j + 1) % M];
                glm::dvec3 v3 = surfaceStrips[i + 1][(j + 1) % M];

                surface.push_back(glm::vec3(v0));
                surface.push_back(glm::vec3(v1));
                surface.push_back(glm::vec3(v2));
                surface.push_back(glm::vec3(v2));
                surface.push_back(glm::vec3(v1));
                surface.push_back(glm::vec3(v3));
            }
        }
        return surface;
    }
    std::cout << "Error: Cross section not set up correctly" << std::endl;

    return surface;
}

void previewPlantPart(std::shared_ptr<Callbacks3D> cb, PlantPart& part, ShaderProgram& cpShader) {
	
	part.surface = generatePlantPart(part);

	std::vector<glm::vec3> axisVerts = {
	glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
	glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)
	};

	std::vector<glm::vec3> axisColors = {
		glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)
	};

	GPU_Geometry axisGeom;
	axisGeom.setVerts(axisVerts);
	axisGeom.setCols(axisColors);
	axisGeom.bind();

	cpShader.use();
	cb->viewPipelineControlPoints(cpShader);
	glLineWidth(2.0f);
	glDrawArrays(GL_LINES, 0, axisVerts.size());

	GPU_Geometry gpuGeom;
	gpuGeom.setVerts(part.surface);
	gpuGeom.setCols(std::vector<glm::vec3>(part.surface.size(), glm::vec3(0.0f)));
	gpuGeom.bind();

	cpShader.use();
	cb->viewPipelineControlPoints(cpShader);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_LINE_STRIP, 0, part.surface.size());
}

void previewPlant(std::shared_ptr<Callbacks3D> cb, Plant& plant, ShaderProgram& cpShader) {

	std::vector<glm::vec3> axisVerts = {
		glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)
	};

	std::vector<glm::vec3> axisColors = {
		glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)
	};

	GPU_Geometry axisGeom;
	axisGeom.setVerts(axisVerts);
	axisGeom.setCols(axisColors);
	axisGeom.bind();

	cpShader.use();
	cb->viewPipelineControlPoints(cpShader);
	glLineWidth(2.0f);
	glDrawArrays(GL_LINES, 0, axisVerts.size());

	for (auto& part : plant.parts) {
		part.surface = generatePlantPart(part);

		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), part.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
			* glm::rotate(glm::mat4(1.0f), part.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f))
			* glm::rotate(glm::mat4(1.0f), part.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));

		part.partMatrix = glm::translate(glm::mat4(1.0f), part.translation)
			* rotationMatrix
			* glm::scale(glm::mat4(1.0f), glm::vec3(part.scale));;

		GPU_Geometry gpuGeom;
		gpuGeom.setVerts(part.surface);
		gpuGeom.setCols(std::vector<glm::vec3>(part.surface.size(), glm::vec3(0.0f)));
		gpuGeom.bind();
		
		cpShader.use();
		cb->viewPipelinePlantPreview(cpShader, part.partMatrix);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_LINE_STRIP, 0, part.surface.size());
	}
}

void handleDraggingControlPoint(std::shared_ptr<Callbacks3D> cb, int& index, CPU_Geometry& controlPointsCPU, GPU_Geometry& controlPointsGPU, ShaderProgram& pickerShader, Window& window, Surface& splineSurface, std::vector<glm::vec3>& flatControlPoints) {
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

	// __________________________________________________________________
	// Update the control point position if the user is dragging it

	if (cb->controlPointPosUpdated) {
		splineSurface.setControlPoint(index, cb->controlPointPos);

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
}

std::vector<double> getKnotSequence(int k, int m) {
	std::vector<double> U;

	double step = 1.0 / (m - k + 2);
	for (int i = 0; i <= m + k; ++i) {
		if (i < k) {
			U.push_back(0.0);
		}
		else if (i > m) {
			U.push_back(1.0);
		}
		else {
			U.push_back(double(i - k + 1) / (m - k + 2));
		}
	}
	return U;
}

glm::vec3 E_delta_1(const std::vector<glm::vec3>& ctrlPts, const std::vector<float>& weights, const std::vector<double>& U, float u, int k, int m) {

	int d = -1;
	for (int i = 0; i < m + k; ++i) {
		if (u >= U[i] && u < U[i + 1]) {
			d = i;
			break;
		}
	}
	if (d == -1) d = m;

	std::vector<glm::vec3> C;
	std::vector<float> w;

	for (int j = 0; j < k; ++j) {
		C.push_back(weights[d - j] * ctrlPts[d - j]);
		w.push_back(weights[d - j]);
	}

	for (int r = k; r >= 2; --r) {
		int j = d;
		for (int s = 0; s <= r - 2; ++s) {
			float omega = (u - U[j]) / (U[j + r - 1] - U[j]);
			C[s] = omega * C[s] + (1 - omega) * C[s + 1];
			w[s] = omega * w[s] + (1 - omega) * w[s + 1];
			j--;
		}
	}

	return C[0] / w[0];
}

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

	// Plant Objects
	std::vector<Plant> plants;

	// Create an orange object
	Plant plant("Plant");
	PlantPart orangePart("PlantPart");

	plant.addPart(orangePart);
	plants.push_back(plant);

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();


		// Three functions that must be called each new frame.
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Preferences");

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

		static const char* options[] = { "Default", "Editing" };
		static int comboSelection = 0;
		static bool modeChanged = false;
		cb->setMode(comboSelection);

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::Text("-------------------------------");
		ImGui::Text("Mode");
		if (ImGui::BeginCombo("##Mode", options[comboSelection])) {
			for (int i = 0; i < 2; ++i) {
				bool isSelected = (comboSelection == i);
				if (ImGui::Selectable(options[i], isSelected)) {
					comboSelection = i;
					modeChanged = true;

				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		if (comboSelection == 1) {
			handleEditingMode(cb, plants, editingShader, cpShader);

			ImGui::End();
			ImGui::Render();
		}
		else {

			ImGui::End();
			ImGui::Render();

			// __________________________________________________________________
			// Handle mouse input for control point dragging
			if (cb->isLeftMouseDown()) {
				handleDraggingControlPoint(cb, index, controlPointsCPU, controlPointsGPU, pickerShader, window, splineSurface, flatControlPoints);
			}

			glEnable(GL_LINE_SMOOTH);
			glEnable(GL_FRAMEBUFFER_SRGB);
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, (simpleWireframe ? GL_LINE : GL_FILL));
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
		}

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
