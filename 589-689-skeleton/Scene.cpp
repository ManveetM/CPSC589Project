#include "Scene.h"
# include "Noise.h"

// Fix the issue by properly initializing the `pickerTex` object using its constructor instead of calling it like a function.
void Scene::initializeGpuPicking() {
	const glm::ivec2 fbSize = window.getFramebufferSize();

	pickerRB.setStorage(GL_DEPTH_COMPONENT24, fbSize.x, fbSize.y);

	pickerFB.addTextureAttachment(GL_COLOR_ATTACHMENT0, pickerTex);
	pickerFB.addRenderbufferAttachment(GL_DEPTH_ATTACHMENT, pickerRB);

	auto fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fbStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		Log::error("Error creating framebuffer : {}", fbStatus);
		throw std::runtime_error("Framebuffer creation error!");
	}
}

void Scene::setShader(ShaderType type)
{
	switch (type) {
	case ShaderType::DEFAULT:
		if (shaders.count("default")) {
			activeShader = shaders.at("default");
		}
		break;
	case ShaderType::CONTROL_POINTS:
		if (shaders.count("controlPoint")) {
			activeShader = shaders.at("controlPoint");
		}
		break;
	case ShaderType::PICKER:
		if (shaders.count("picker")) {
			activeShader = shaders.at("picker");
		}
		break;
	}
}

void Scene::initialize() {

	initializeLandscape();

	shaders.at("default")->use();
	cb->updateShadingUniforms(lightPos, lightCol, diffuseCol, ambientStrength, false);

	// Create an orange object
	Plant plant("Plant");
	PlantPart part("PlantPart");

	plant.addPart(part);
	plants.push_back(plant);
}

void Scene::initializeLandscape() {
	landscape.generateSurface();
	landscape.bind();
}

void Scene::handleGPUPickingLandscape() {
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_DITHER);

	glm::ivec2 pickPos = cb->getMousePos();
	pickPos.y = window.getHeight() - pickPos.y;

	GPU_Geometry gpuGeom;
	std::vector<std::vector<glm::vec3>> cp = landscape.getControlGrid();

	std::vector<glm::vec3> flattenedControlPoints;
	for (const auto& row : cp) {
		flattenedControlPoints.insert(flattenedControlPoints.end(), row.begin(), row.end());
	}
	gpuGeom.setVerts(flattenedControlPoints);

	// We'll draw each point one at a time
	for (int i = 0; i < flattenedControlPoints.size(); ++i) {
		pickerFB.bind();
		glClearBufferiv(GL_COLOR, 0, pickerClearValue);
		glClear(GL_DEPTH_BUFFER_BIT);

		glEnable(GL_SCISSOR_TEST);
		glScissor(pickPos.x, pickPos.y, 1, 1);

		shaders.at("picker")->use();
		cb->viewPipelinePicker();

		gpuGeom.bind();

		glPointSize(15);
		glDrawArrays(GL_POINTS, i, 1);

		GLint pickTexCPU[1];
		pickerTex.bind();
		glReadPixels(pickPos.x, pickPos.y, 1, 1, GL_RED_INTEGER, GL_INT, pickTexCPU);

		pickerFB.unbind();
		glDisable(GL_SCISSOR_TEST);

		if (pickTexCPU[0] == 1) {
			controlPointIndex = i;
			landscape.updateControlPoint(i, cb->getDragOffset());
			break;
		}
	}
	glEnable(GL_DITHER);
}

void Scene::drawImGui() {
	// Three functions that must be called each new frame.
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Preferences");

	// The rest of our ImGui widgets.
	lightingChange |= ImGui::DragFloat3("Light's position", glm::value_ptr(lightPos));
	lightingChange |= ImGui::ColorEdit3("Light's colour", glm::value_ptr(lightCol));
	lightingChange |= ImGui::ColorEdit3("DiffuseColor", glm::value_ptr(diffuseCol));
	lightingChange |= ImGui::SliderFloat("Ambient strength", &ambientStrength, 0.0f, 1.f);
	lightingChange |= ImGui::Checkbox("Simple wireframe", &simpleWireframe);

	ImGui::Text("Brush Tool");
	ImGui::Checkbox("Enable Brush Tool", &brushEnabled);
	ImGui::Checkbox("Raise (Uncheck to Lower)", &brushRaise);
	ImGui::SliderFloat("Brush Radius", &brushRadius, 0.1f, 10.0f);
	ImGui::SliderFloat("Brush Strength", &brushStrength, 0.01f, 1.0f);
	ImGui::SliderFloat("Noise Scale", &noiseScale, 0.01f, 2.0f);
	ImGui::SliderFloat("Noise Amplitude", &noiseAmplitude, 0.0f, 1.0f);

	// Framerate display, in case you need to debug performance.
	ImGui::Text("Average %.1f ms/frame (%.1f fps)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::Dummy(ImVec2(0.0f, 5.0f));
	ImGui::Checkbox("3D Axes", &show3DAxes);

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

	if (comboSelection == 0) {

	}
	else if (comboSelection == 1) {
		drawEditingImGui();
	}
		
	ImGui::End();
	ImGui::Render();
	
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Scene::drawEditingImGui() {
	if (ImGui::BeginCombo("Plants", selectedPlantIndex >= 0 ? plants[selectedPlantIndex].getName().c_str() : "Select a Plant")) {
		for (int i = 0; i < plants.size(); ++i) {
			bool isSelected = (selectedPlantIndex == i);
			if (ImGui::Selectable(plants[i].getName().c_str(), isSelected)) {
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
		if (ImGui::BeginCombo("Parts", selectedPartIndex >= 0 ? selectedPlant.getParts()[selectedPartIndex].getName().c_str() : "Select a Part")) {
			for (int i = 0; i < selectedPlant.getParts().size(); ++i) {
				bool isSelected = (selectedPartIndex == i);
				if (ImGui::Selectable(selectedPlant.getParts()[i].getName().c_str(), isSelected)) {
					selectedPartIndex = i;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::Button("Delete Part")) {
			if (!plants.empty() && selectedPlantIndex < plants.size()) {
				auto& parts = plants[selectedPlantIndex].getParts();

				if (!parts.empty() && selectedPartIndex < parts.size()) {
					plants[selectedPlantIndex].removePart(selectedPartIndex);
					selectedPartIndex = -1;
				}
			}
		}

		ImGui::Dummy(ImVec2(0.0f, 5.0f));
		static char plantName[64] = "";
		ImGui::InputText("Plant Name", plantName, sizeof(plantName));
		if (ImGui::Button("Add New Plant")) {
			if (strlen(plantName) > 0) {

				for (auto& plant : plants) {
					if (plant.getName() == plantName) {
						std::cout << "Another plant with the same name already exists" << std::endl;
						return;
					}
				}

				previewingPlant = false;
				previewingPart = false;

				selectedPlantIndex = -1;
				selectedPartIndex = -1;

				Plant newPlant(plantName);
				plants.push_back(newPlant);
			}
		}

		ImGui::Dummy(ImVec2(0.0f, 5.0f));
		static char partName[64] = "";
		ImGui::InputText("Part Name", partName, sizeof(partName));
		if (ImGui::Button("Add New Part")) {
			if (strlen(partName) > 0 && selectedPlantIndex < plants.size()) {
				auto& parts = plants[selectedPlantIndex].getParts();

				for (auto& part : parts) {
					if (part.getName() == partName) {
						std::cout << "Another part with the same name already exists" << std::endl;
						return;
					}
				}

				previewingPlant = false;
				previewingPart = false;

				PlantPart newPart(partName);
				plants[selectedPlantIndex].addPart(newPart);
			}
		}
	}

	if (selectedPlantIndex >= 0 && selectedPartIndex >= 0 && !previewingPlant && !previewingPart) {
		
		auto& selectedPart = plants[selectedPlantIndex].getParts()[selectedPartIndex];

		selectedPart.setSurfaceGenerated(false);

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::Text("-------------------------------");
		ImGui::Text("Editing Part: %s", selectedPart.getName().c_str());
		if (ImGui::Button("Clear")) {
			previewingPlant = false;
			previewingPart = false;

			selectedPart.clear();
		}

		ImGui::Dummy(ImVec2(0.0f, 10.0f));

		ImGui::Checkbox("Left Curve", &showLeftCurve);
		if (showLeftCurve && !previewingPart && !previewingPlant) {
			int index = -1;

			PointsData& leftControlPoints = selectedPart.getLeftControlPoints();

			for (int i = 0; i < leftControlPoints.selected.size(); ++i) {
				if (leftControlPoints.selected.at(i)) {
					index = i;
				}
			}

			if (index != -1) {
				float weight = leftControlPoints.weights.at(index);
				bool weightChanged = ImGui::SliderFloat("Weight", &weight, 0.0f, 20.0f);

				if (weightChanged) {
					leftControlPoints.weights.at(index) = weight;
					std::vector<glm::vec3> leftCurve = updateBSpline(leftControlPoints);
					selectedPart.setLeftCurve(leftCurve);
				}
			}

			if (leftControlPoints.needsUpdate) {
				selectedPart.setLeftCurve(updateBSpline(leftControlPoints));
				leftControlPoints.needsUpdate = false;
			}

			handleEditingControlPointUpdate(leftControlPoints);

			showRightCurve = false;
			showCrossSection = false;

			GPU_Geometry gpuGeom;
			gpuGeom.setVerts(leftControlPoints.cpuGeom.verts);
			gpuGeom.setCols(leftControlPoints.cpuGeom.cols);
			gpuGeom.bind();
		}

		ImGui::Checkbox("Right Curve", &showRightCurve);
		if (showRightCurve && !previewingPart && !previewingPlant) {
			int index = -1;

			PointsData& rightControlPoints = selectedPart.getRightControlPoints();

			for (int i = 0; i < rightControlPoints.selected.size(); ++i) {
				if (rightControlPoints.selected.at(i)) {
					index = i;
				}
			}

			if (index != -1) {
				float weight = rightControlPoints.weights.at(index);
				bool weightChanged = ImGui::SliderFloat("Weight", &weight, 0.0f, 20.0f);

				if (weightChanged) {
					rightControlPoints.weights.at(index) = weight;
					std::vector<glm::vec3> rightCurve = updateBSpline(rightControlPoints);
					selectedPart.setRightCurve(rightCurve);
				}
			}

			if (rightControlPoints.needsUpdate) {
				selectedPart.setRightCurve(updateBSpline(rightControlPoints));
				rightControlPoints.needsUpdate = false;
			}

			if (cb->isLeftMouseDown()) {

			}

			handleEditingControlPointUpdate(rightControlPoints);

			showLeftCurve = false;
			showCrossSection = false;

			GPU_Geometry gpuGeom;
			gpuGeom.setVerts(rightControlPoints.cpuGeom.verts);
			gpuGeom.setCols(rightControlPoints.cpuGeom.cols);
			gpuGeom.bind();
		}

		ImGui::Checkbox("Cross Section", &showCrossSection);
		if (showCrossSection && !previewingPart && !previewingPlant) {
			int index = -1;

			PointsData& crossSectionControlPoints = selectedPart.getCrossSectionControlPoints();

			for (int i = 0; i < crossSectionControlPoints.selected.size(); ++i) {
				if (crossSectionControlPoints.selected.at(i)) {
					index = i;
				}
			}

			if (index != -1) {
				float weight = crossSectionControlPoints.weights.at(index);
				bool weightChanged = ImGui::SliderFloat("Weight", &weight, 0.0f, 20.0f);

				if (weightChanged) {
					crossSectionControlPoints.weights.at(index) = weight;
					std::vector<glm::vec3> crossSectionCurve = updateBSpline(crossSectionControlPoints);
					selectedPart.setCrossSectionCurve(crossSectionCurve);
				}
			}

			if (crossSectionControlPoints.needsUpdate) {
				selectedPart.setCrossSectionCurve(updateBSpline(crossSectionControlPoints));
				crossSectionControlPoints.needsUpdate = false;
			}

			handleEditingControlPointUpdate(crossSectionControlPoints);

			showLeftCurve = false;
			showRightCurve = false;

			GPU_Geometry gpuGeom;
			gpuGeom.setVerts(crossSectionControlPoints.cpuGeom.verts);
			gpuGeom.setCols(crossSectionControlPoints.cpuGeom.cols);
			gpuGeom.bind();
		}


		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::Text("Transformations");
		ImGui::DragFloat3("Scale", glm::value_ptr(selectedPart.getScale()), 0.1f, 0.1f, 10.0f);
		ImGui::DragFloat3("Translation", glm::value_ptr(selectedPart.getTranslation()), 0.1f, -10.0f, 10.0f);
		ImGui::DragFloat3("Rotation (degrees)", glm::value_ptr(selectedPart.getRotation()), 0.1f, -180.0f, 180.0f);

		ImGui::ColorEdit3("Base Color", glm::value_ptr(selectedPart.getBaseColor()));

		if (ImGui::Button("Preview Part")) {
			if (selectedPart.getLeftCurve().empty() || selectedPart.getRightCurve().empty() || selectedPart.getCrossSectionCurve().empty()) {
				std::cout << "Error: All three curves (left, right, cross-section) must be set before calculating the surface." << std::endl;
			}
			else {
				if (selectedPart.getLeftCurve().size() != selectedPart.getCrossSectionCurve().size()) {
					std::cout << "Error: Left and Right curves must have the same number of points." << std::endl;
				}
				else {
					std::cout << "Part previewing..." << selectedPart.getName() << std::endl;

					showLeftCurve = false;
					showRightCurve = false;
					showCrossSection = false;

					previewingPart = true;
				}
			}
		}

		if (ImGui::Button("Preview Plant")) {

			for (auto& part : plants[selectedPlantIndex].getParts()) {
				if (selectedPart.getLeftCurve().empty() || selectedPart.getRightCurve().empty() || selectedPart.getCrossSectionCurve().empty()) {
					std::cout << "Error: All three curves (left, right, cross-section) must be set before calculating the surface." << std::endl;
					return;
				}
				if (selectedPart.getLeftCurve().empty() != selectedPart.getRightCurve().size()) {
					std::cout << "Error: All left and right curve sizes must be equal" << std::endl;
				}
			}

			std::cout << "Plant previewing..." << selectedPart.getName() << std::endl;

			showLeftCurve = false;
			showRightCurve = false;
			showCrossSection = false;

			previewingPlant = true;
		}
	}

	if (selectedPlantIndex >= 0 && selectedPartIndex >= 0) {
		auto& plant = plants[selectedPlantIndex];

		if (previewingPart) {
			if (ImGui::Button("Edit Surface")) {
				cb->resetCamera();
				previewingPart = false;
				previewingPlant = false;
			}
		}
		else if (previewingPlant) {
			if (ImGui::Button("Edit Surface")) {
				cb->resetCamera();
				previewingPart = false;
				previewingPlant = false;
			}
		}
	}
}

void Scene::handleEditingControlPointUpdate(PointsData& cp) {
	if (cb->isRightMouseDown()) {
		int indexToDelete = -1;

		for (int i = 0; i < cp.cpuGeom.verts.size(); i++) {
			glm::vec2 p = cb->getCursorPosGL();
			glm::vec3 delta = (cp.cpuGeom.verts[i] - glm::vec3(p, 0.0f));
			if (glm::length(delta) < 0.025f) {
				indexToDelete = i;
				break;
			}
		}

		if (indexToDelete != -1) {
			cp.cpuGeom.verts.erase(cp.cpuGeom.verts.begin() + indexToDelete);
			cp.cpuGeom.cols.erase(cp.cpuGeom.cols.begin() + indexToDelete);
			cp.selected.erase(cp.selected.begin() + indexToDelete);
			cp.weights.erase(cp.weights.begin() + indexToDelete);
			cp.needsUpdate = true;
		}
	}

	if (cb->isLeftMouseDown() && controlPointIndex == -1) {
		int indexToMove = -1;
		glm::vec2 p = cb->getCursorPosGL();

		for (int i = 0; i < cp.cpuGeom.verts.size(); i++) {
			glm::vec3 delta = (cp.cpuGeom.verts[i] - glm::vec3(p, 0.0f));
			if (glm::length(delta) < 0.05f) {
				indexToMove = i;

				std::fill(cp.cpuGeom.cols.begin(), cp.cpuGeom.cols.end(), glm::vec3(1.0f, 0.0f, 0.0f));
				std::fill(cp.selected.begin(), cp.selected.end(), false);

				cp.cpuGeom.cols.at(i) = glm::vec3(0.0f, 1.0f, 0.0f);
				cp.selected.at(i) = true;
				break;
			}
		}

		if (indexToMove == -1) {
			// Add a new point
			std::fill(cp.cpuGeom.cols.begin(), cp.cpuGeom.cols.end(), glm::vec3(1.0f, 0.0f, 0.0f));
			std::fill(cp.selected.begin(), cp.selected.end(), false);

			cp.cpuGeom.verts.push_back(glm::vec3(p, 0.f));
			cp.cpuGeom.cols.push_back(glm::vec3(0.f, 1.f, 0.f));
			cp.selected.push_back(true);
			cp.weights.push_back(1.0f);
			cp.needsUpdate = true;
		}
		else {
			controlPointIndex = indexToMove;
		}
	}
	else if (cb->isLeftMouseDown()) {
		assert(selectedPartIndex >= 0 && selectedPlantIndex >= 0);
		auto& selectedPart = plants[selectedPlantIndex].getParts()[selectedPartIndex];

		if (showLeftCurve) {
			selectedPart.getLeftControlPoints().cpuGeom.verts.at(controlPointIndex) += 2.45f * cb->getDragOffset();
			selectedPart.getLeftControlPoints().needsUpdate = true;
		}
		else if (showRightCurve) {
			selectedPart.getRightControlPoints().cpuGeom.verts.at(controlPointIndex) += 2.45f * cb->getDragOffset();
			selectedPart.getRightControlPoints().needsUpdate = true;
		}
		else if (showCrossSection) {
			selectedPart.getCrossSectionControlPoints().cpuGeom.verts.at(controlPointIndex) += 2.45f * cb->getDragOffset();
			selectedPart.getCrossSectionControlPoints().needsUpdate = true;
		}
	}
	else {
		controlPointIndex = -1;
	}
}

std::vector<glm::vec3> Scene::updateBSpline(PointsData& controlPoints) {
	std::vector<glm::vec3> bSpline;
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
	return bSpline;
}

std::vector<double> Scene::getKnotSequence(int k, int m) {
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

glm::vec3 Scene::E_delta_1(const std::vector<glm::vec3>& ctrlPts, const std::vector<float>& weights, const std::vector<double>& U, float u, int k, int m) {

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

void Scene::updateScene() {
	if (!cb->isLeftMouseDown()) {
		controlPointIndex = -1;
	}

	if (lightingChange) {
		shaders.at("default")->use();
		cb->updateShadingUniforms(lightPos, lightCol, diffuseCol, ambientStrength, false);
	}

	if (modeChanged) {
		cb->resetCamera();
		modeChanged = false;
	}

	if (comboSelection == 0) {
		cb->setIs3D(true);
		updateLandscapeState();
	}
	else if (comboSelection == 1) {
		if (previewingPlant || previewingPart) cb->setIs3D(true);
		else cb->setIs3D(false);
	}
}

void Scene::updateLandscapeState() {
	if (brushEnabled && cb->isLeftMouseDown()) {
		applyBrushDeformation();
	}
	else if (cb->isLeftMouseDown() && controlPointIndex == -1) {
		handleGPUPickingLandscape();
	}
	else if (cb->isLeftMouseDown()) {
		landscape.updateControlPoint(controlPointIndex, cb->getDragOffset());
		landscape.generateSurface();
	}
}

void Scene::draw() {
	glEnable(GL_LINE_SMOOTH);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// draw rest of the scene
	glEnable(GL_FRAMEBUFFER_SRGB);
	glPolygonMode(GL_FRONT_AND_BACK, (simpleWireframe ? GL_LINE : GL_FILL));

	// draw scene
	if (comboSelection == 0) {
		drawLandscapeControlPoints();
		drawLandscape();
		drawAxes("controlPoints");
	}
	else if (comboSelection == 1) {
		previewPlants();
		drawControlPoints();
		drawCurves();
		drawAxes("editing");
	}

	// draw imgui
	glDisable(GL_FRAMEBUFFER_SRGB);
	drawImGui();
}

void Scene::previewPlants() {
	if (previewingPart) {
		assert(selectedPartIndex >= 0 && selectedPlantIndex >= 0);
		auto& selectedPart = plants[selectedPlantIndex].getParts()[selectedPartIndex];
		if (!selectedPart.isSurfaceGenerated()) {
			selectedPart.generatePlantPart();
		}

		// Draw the surface
		shaders.at("default")->use();

		GPU_Geometry gpuGeom;
		gpuGeom.setVerts(selectedPart.getSurface());
		gpuGeom.setCols(selectedPart.getCols());
		gpuGeom.setIndices(selectedPart.getIndices());
		gpuGeom.setNormals(selectedPart.getNormals());
		gpuGeom.bind();

		cb->viewPipeline();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawElements(GL_TRIANGLES, selectedPart.getIndices().size(), GL_UNSIGNED_INT, 0);

	}
	else if (previewingPlant) {
		assert(selectedPlantIndex >= 0);
		auto& plant = plants[selectedPlantIndex];

		for (auto& part : plant.getParts()) {
			if (!part.isSurfaceGenerated()) {
				part.generatePlantPart();
			}

			if (part.getSurface().size() == 0) {
				continue;
			}

			shaders.at("default")->use();

			GPU_Geometry gpuGeom;

			gpuGeom.setVerts(part.getSurface());
			gpuGeom.setCols(part.getCols());
			gpuGeom.setIndices(part.getIndices());
			gpuGeom.setNormals(part.getNormals());
			gpuGeom.bind();

			cb->viewPipelinePlantPreview(part.getPartTransformMatrix());

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDrawElements(GL_TRIANGLES, part.getIndices().size(), GL_UNSIGNED_INT, 0);
		}
	}
}

void Scene::drawCurves() {

	if (!previewingPlant && !previewingPart) {
		if (selectedPlantIndex >= 0 && selectedPartIndex >= 0) {
			auto& selectedPart = plants[selectedPlantIndex].getParts()[selectedPartIndex];

			shaders.at("editing")->use();
			cb->viewPipelineEditing(*shaders.at("editing"));

			GPU_Geometry gpuGeom;

			gpuGeom.setVerts(selectedPart.getLeftCurve());
			gpuGeom.setCols(std::vector<glm::vec3>(selectedPart.getLeftCurve().size(), glm::vec3(1.0f, 0.0f, 0.0f)));
			gpuGeom.bind();
			glDrawArrays(GL_LINE_STRIP, 0, selectedPart.getLeftCurve().size());

			gpuGeom.setVerts(selectedPart.getRightCurve());
			gpuGeom.setCols(std::vector<glm::vec3>(selectedPart.getRightCurve().size(), glm::vec3(0.0f, 1.0f, 0.0f)));
			gpuGeom.bind();
			glDrawArrays(GL_LINE_STRIP, 0, selectedPart.getRightCurve().size());

			gpuGeom.setVerts(selectedPart.getCrossSectionCurve());
			gpuGeom.setCols(std::vector<glm::vec3>(selectedPart.getCrossSectionCurve().size(), glm::vec3(0.0f, 0.0f, 1.0f)));
			gpuGeom.bind();
			glDrawArrays(GL_LINE_STRIP, 0, selectedPart.getCrossSectionCurve().size());
		}
	}
}

void Scene::drawControlPoints() {
	if (!previewingPlant && !previewingPart) {
		if (selectedPlantIndex >= 0 && selectedPartIndex >= 0) {
			auto& selectedPart = plants[selectedPlantIndex].getParts()[selectedPartIndex];

			shaders.at("editing")->use();
			cb->viewPipelineEditing(*shaders.at("editing"));

			CPU_Geometry cpuGeom;
			GPU_Geometry gpuGeom;

			if (showLeftCurve) {
				cpuGeom = selectedPart.getLeftControlPoints().cpuGeom;

				gpuGeom.setVerts(cpuGeom.verts);
				gpuGeom.setCols(std::vector<glm::vec3>(cpuGeom.verts.size(), glm::vec3(1.0f, 0.0f, 0.0f)));

			}
			else if (showRightCurve) {
				cpuGeom = selectedPart.getRightControlPoints().cpuGeom;
				gpuGeom.setVerts(cpuGeom.verts);
				gpuGeom.setCols(std::vector<glm::vec3>(cpuGeom.verts.size(), glm::vec3(1.0f, 0.0f, 0.0f)));
			}
			else if (showCrossSection) {
				cpuGeom = selectedPart.getCrossSectionControlPoints().cpuGeom;
				gpuGeom.setVerts(cpuGeom.verts);
				gpuGeom.setCols(std::vector<glm::vec3>(cpuGeom.verts.size(), glm::vec3(1.0f, 0.0f, 0.0f)));
			}

			gpuGeom.bind();
			glPointSize(10);
			glDrawArrays(GL_POINTS, 0, cpuGeom.verts.size());

			glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
		}
	}
}

void Scene::drawAxes(const char* shaderType) {
	if (!show3DAxes) return;

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

	shaders.at(shaderType)->use();
	if (strcmp(shaderType, "editing") == 0) {
		cb->viewPipelineEditing(*shaders.at(shaderType));
	}
	else if (strcmp(shaderType, "controlPoints") == 0) {
		cb->viewPipelineControlPoints(*shaders.at(shaderType));
	}


	glLineWidth(2.0f);
	glDrawArrays(GL_LINES, 0, axisVerts.size());
}

void Scene::drawLandscape() {
	shaders.at("controlPoint")->use();
	cb->viewPipelineControlPoints(*shaders.at("controlPoint"));

	landscape.bind();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, landscape.numVerts());
}

void Scene::drawLandscapeControlPoints() {
	shaders.at("controlPoint")->use();  
	cb->viewPipelineControlPoints(*shaders.at("controlPoint"));  

	GPU_Geometry controlPointsGPU;  

	std::vector<std::vector<glm::vec3>> cp = landscape.getControlGrid();  

	std::vector<glm::vec3> flattenedControlPoints;  
	for (const auto& row : cp) {  
		flattenedControlPoints.insert(flattenedControlPoints.end(), row.begin(), row.end());  
	}  

	controlPointsGPU.setVerts(flattenedControlPoints);  
	controlPointsGPU.setCols(std::vector<glm::vec3>(flattenedControlPoints.size(), glm::vec3(1.0f, 0.0f, 0.0f)));  
	controlPointsGPU.bind();  

	glPointSize(10);  
	glDrawArrays(GL_POINTS, 0, flattenedControlPoints.size());  
}

void Scene::applyBrushDeformation() {
	/*static float brushRadius = 1.5f;
	static float brushStrength = 0.1f;
	static float noiseScale = 0.5f;
	static float noiseAmplitude = 0.2f;
	static bool brushRaise = true;*/

	if (!cb->isLeftMouseDown()) return;

	glm::vec2 mouseGL = cb->getCursorPosGL();
	glm::mat4 view = cb->getCamera().getView();
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.01f, 1000.0f);
	glm::mat4 invVP = glm::inverse(proj * view);

	glm::vec4 screenNear = glm::vec4(mouseGL.x, mouseGL.y, -1.0f, 1.0f);
	glm::vec4 screenFar = glm::vec4(mouseGL.x, mouseGL.y, 1.0f, 1.0f);

	glm::vec4 nearPos4 = invVP * screenNear;
	glm::vec4 farPos4 = invVP * screenFar;
	nearPos4 /= nearPos4.w;
	farPos4 /= farPos4.w;

	glm::vec3 worldNear = glm::vec3(nearPos4);
	glm::vec3 worldFar = glm::vec3(farPos4);

	glm::vec3 rayOrigin = cb->getCamera().getPos();
	glm::vec3 rayDir = glm::normalize(worldFar - worldNear);

	float t = -rayOrigin.y / rayDir.y;
	if (t <= 0) return;

	glm::vec3 intersectionPt = rayOrigin + rayDir * t;

	std::vector<std::vector<glm::vec3>> grid = landscape.getControlGrid();
	int gridSize = grid.size();
	bool modified = false;

	for (int i = 0; i < gridSize; ++i) {
		for (int j = 0; j < gridSize; ++j) {
			glm::vec3 pt = grid[i][j];
			glm::vec2 pt2d(pt.x, pt.z);
			glm::vec2 intersection2d(intersectionPt.x, intersectionPt.z);
			float dist = glm::distance(pt2d, intersection2d);

			if (dist < brushRadius) {
				float influence = 1.0f - (dist / brushRadius);
				influence = std::pow(influence, 2.0f);
				float baseDisplacement = brushStrength * influence * (brushRaise ? 1.0f : -1.0f);
				float noise = generateNoise(pt.x, pt.z, noiseScale, 1.0f);
				pt.y += baseDisplacement * (1.0f + noiseAmplitude * noise);
				landscape.updateControlPoint(i * gridSize + j, pt - grid[i][j]);
				modified = true;
			}
		}
	}

	if (modified) {
		landscape.generateSurface();
	}
}