#include "Scene.h"

void Scene::initializeControlPoints() {
	// Resize control points and colors to match the number of control points in U
	controlPoints.resize(numCtrlPtsU);
	controlPointColors.resize(numCtrlPtsU);

	float spacing = 1.0f; // Space between control points

	// Calculate offsets to center the control points on the origin
	float offsetX = (numCtrlPtsU - 1) * spacing / 2.0f;
	float offsetZ = (numCtrlPtsV - 1) * spacing / 2.0f;

	// Initialize control points and their colors
	for (int i = 0; i < numCtrlPtsU; ++i) {
		controlPoints[i].resize(numCtrlPtsV);
		controlPointColors[i].resize(numCtrlPtsV);

		for (int j = 0; j < numCtrlPtsV; ++j) {
			// Set control point position on the xz plane, centered at the origin
			controlPoints[i][j] = glm::vec3(i * spacing - offsetX, 0.0f, j * spacing - offsetZ);
		}
	}

	// Generate open uniform knot vectors
	knotU.clear();
	knotV.clear();

	int nU = numCtrlPtsU - 1;
	int nV = numCtrlPtsV - 1;

	int mU = nU + degreeU + 1;
	int mV = nV + degreeV + 1;

	// Generate knot vector for U
	for (int i = 0; i <= mU; ++i) {
		if (i < degreeU) {
			knotU.push_back(0.0f);
		}
		else if (i <= nU) {
			knotU.push_back((float)(i - degreeU) / (nU - degreeU + 1));
		}
		else {
			knotU.push_back(1.0f);
		}
	}

	// Generate knot vector for V
	for (int i = 0; i <= mV; ++i) {
		if (i < degreeV) {
			knotV.push_back(0.0f);
		}
		else if (i <= nV) {
			knotV.push_back((float)(i - degreeV) / (nV - degreeV + 1));
		}
		else {
			knotV.push_back(1.0f);
		}
	}

	// Mark the surface as needing an update
	surfaceNeedsUpdate = true;
}

void Scene::updateControlPointGeom(bool forPicking) {
	std::vector<glm::vec3> verts;
	std::vector<glm::vec3> cols;

	for (int i = 0; i < numCtrlPtsU; ++i) {
		for (int j = 0; j < numCtrlPtsV; ++j) {
			verts.push_back(controlPoints[i][j]);
			if (forPicking)
				cols.push_back(controlPointColors[i][j]); // encode ID as color
			else
				cols.push_back(glm::vec3(1.0f, 0.0f, 0.0f)); // red for visualization
		}
	}

	cp_gpu.setVerts(verts);
	cp_gpu.setCols(cols);
	cp_gpu.bind();

	cp_cpu.verts = verts;
}

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

void Scene::handleGpuPicking() {
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST);

	// Bind our new framebuffer.
	pickerFB.bind();
	// Integer textures don't support the dithering that's enabled by default.
	// What is dithering? See: https://community.khronos.org/t/what-is-gl-dither/31034
	glDisable(GL_DITHER);

	// Because our framebuffer's an int framebuffer, we'll use glClearBufferiv
	// to clear it instead of glClear
	glClearBufferiv(GL_COLOR, 0, pickerClearValue);
	// But we will use glClear for the depth.
	glClear(GL_DEPTH_BUFFER_BIT);

	// Hard-coded "cursor" position for initial testing.
	const glm::ivec2 pickPos(200, 200);

	// Restrict the rendering region to the single pixel of interest.
	// Not require, but *might* help efficiency.
	glEnable(GL_SCISSOR_TEST);
	glScissor(pickPos.x, pickPos.y, 1, 1);

	// Activate the shader and set up the uniforms, then draw the model.
	setShader(ShaderType::PICKER);
	useShader();
	cp_gpu.bind();
	callbacks->viewPipelinePicker(*activeShader);
	glDrawArrays(GL_TRIANGLES, 0, cp_cpu.verts.size());

	// Bind and read the rendered result from the texture.
	pickerTex.bind();
	GLint pickTexCPU[1];
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glReadPixels.xhtml
	glReadPixels(pickPos.x, pickPos.y, 1, 1, GL_RED_INTEGER, GL_INT, pickTexCPU);

	//std::cout << "Picked ID:" << pickTexCPU[0] << std::endl;

	// Binds the "0" default framebuffer, which we use for our visual result.
	pickerFB.unbind();

	// Reset changed settings to default for the main visual render.
	glEnable(GL_DITHER);
	glDisable(GL_SCISSOR_TEST);
}


