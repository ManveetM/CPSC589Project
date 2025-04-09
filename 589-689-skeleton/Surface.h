// #pragma once

// #include <glad/glad.h>
// #include <GLFW/glfw3.h>

// #include <iostream>

// #include "Geometry.h"

// class Surface {
//     public:
//         Surface(int gridSize);
//         void initializeSurface();
//     private:
//         std::vector<std::vector<glm::vec3>> controlGrid;
//         int gridSize;
//         CPU_Geometry surface;
//         const int resolutionU; // number of samples in u direction
//         const int resolutionV; // number of samples in v direction

//         CPU_Geometry createBSpline(std::vector<std::vector<glm::vec3>> pointsData, int k, float uIncr);
//         std::vector<double> initializeKnot(int k, int m);
//         int delta(int k, int m, float u, std::vector<double>& U);
//         glm::vec3 E_delta_1(std::vector<std::vector<glm::vec3>> controlGrid, std::vector<double> U, float u, int k, int m);
// };

// BSplineSurface.h
#pragma once

#include "Geometry.h"
#include <vector>
#include "glm/glm.hpp"

class Surface {
public:
	Surface(int controlSize, int kU, int kV, int resU, int resV);

	void generateSurface();      // Compute surface + upload to GPU
	void bind();                 // Bind VAO
	size_t numVerts();           // For draw call

	std::vector<std::vector<glm::vec3>> getControlGrid() { return controlGrid; }
	void setControlPoint(int index, const glm::vec3& newPos) {
		int i = 0;
		while (index > (i + 1) * controlGrid.size() - 1) {
			i += 1;
		}
		int j = index - controlGrid.size() * i;
		controlGrid[i][j] = newPos;
	}

private:
	std::vector<std::vector<glm::vec3>> controlGrid;
	CPU_Geometry cpuGeom;
	GPU_Geometry gpuGeom;

	int kU, kV;
	int resU, resV;

	std::vector<double> initializeKnot(int k, int m);
	//glm::vec3 E_delta_1(const std::vector<glm::vec3>& ctrlPts, const std::vector<double>& U, float u, int k, int m);
	glm::vec3 E_delta(const std::vector<std::vector<glm::vec3>>& ctrlPts,
		const std::vector<double>& U, const std::vector<double>& V, float u, float v,
		int kU, int kV, int mU, int mV);
	//glm::vec3 evaluateSurfacePoint(float u, float v);
};