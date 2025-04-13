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
    void updateControlPoint(int index, const glm::vec3 offset) {  
       int i = 0;  
       size_t gridSize = controlGrid.size();  
       while (index > static_cast<int>((i + 1) * gridSize - 1)) {  
           i += 1;  
       }  
       int j = index - static_cast<int>(gridSize * i);  
       controlGrid[i][j] = controlGrid[i][j] + offset;  
    }
	
private:
	std::vector<std::vector<glm::vec3>> controlGrid;
	CPU_Geometry cpuGeom;
	GPU_Geometry gpuGeom;

	int kU, kV;
	int resU, resV;

	std::vector<double> initializeKnot(int k, int m);
	glm::vec3 E_delta(const std::vector<std::vector<glm::vec3>>& ctrlPts,
		const std::vector<double>& U, const std::vector<double>& V, float u, float v,
		int kU, int kV, int mU, int mV);
};