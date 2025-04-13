#pragma once

#include "Geometry.h"

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