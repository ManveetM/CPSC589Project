#include "Surface.h"
#include <algorithm>

Surface::Surface(int controlSize, int kU, int kV, int resU, int resV)
	: kU(kU), kV(kV), resU(resU), resV(resV)
{
	// Initialize 2D control grid (flat terrain)
	float spacing = 1.0f;
	float half = (controlSize - 1) * spacing / 2.0f;
	for (int i = 0; i < controlSize; ++i) {
		std::vector<glm::vec3> row;
		for (int j = 0; j < controlSize; ++j) {
			row.push_back(glm::vec3(j * spacing - half, 0.0f, i * spacing - half));
		}
		controlGrid.push_back(row);
	}
}

std::vector<double> Surface::initializeKnot(int k, int m) {
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

glm::vec3 Surface::E_delta(const std::vector<std::vector<glm::vec3>>& ctrlPts,
	const std::vector<double>& U, const std::vector<double>& V,
	float u, float v, int kU, int kV, int mU, int mV) {

	int dU = -1;
	for (int i = 0; i < mU + kU; ++i) {
		if (u >= U[i] && u < U[i + 1]) {
			dU = i;
			break;
		}
	}
	if (dU == -1) dU = mU;

	int dV = -1;
	for (int i = 0; i < mV + kV; ++i) {
		if (v >= V[i] && v < V[i + 1]) {
			dV = i;
			break;
		}
	}
	if (dV == -1) dV = mV;

	std::vector<glm::vec3> C;

	for (int i = 0; i < kU; ++i) {
		std::vector<glm::vec3> D;

		for (int j = 0; j < kV; ++j) {
			D.push_back(ctrlPts[dU - i][dV - j]);
		}

		for (int r = kV; r >= 2; --r) {
			int j = dV;
			for (int s = 0; s <= r - 2; ++s) {
				float omega = (v - V[j]) / (V[j + r - 1] - V[j]);
				D[s] = omega * D[s] + (1 - omega) * D[s + 1];
				j--;
			}
		}

		C.push_back(D[0]);
	}

	for (int r = kU; r >= 2; --r) {
		int i = dU;
		for (int s = 0; s <= r - 2; ++s) {
			float omega = (u - U[i]) / (U[i + r - 1] - U[i]);
			C[s] = omega * C[s] + (1 - omega) * C[s + 1];
			i--;
		}
	}

	return C[0];
}

void Surface::generateSurface() {
	cpuGeom.verts.clear();
	cpuGeom.normals.clear();
	//cpuGeom.cols.clear();

	int mU = controlGrid.size() - 1;
	int mV = controlGrid[0].size() - 1;

	auto U = initializeKnot(kU, mU);
	auto V = initializeKnot(kV, mV);

	for (int i = 0; i < resU - 1; ++i) {
		float u0 = float(i) / (resU - 1);
		float u1 = float(i + 1) / (resU - 1);
		for (int j = 0; j < resV - 1; ++j) {
			float v0 = float(j) / (resV - 1);
			float v1 = float(j + 1) / (resV - 1);

			//glm::vec3 p00 = evaluateSurfacePoint(u0, v0);
			//glm::vec3 p10 = evaluateSurfacePoint(u1, v0);
			//glm::vec3 p01 = evaluateSurfacePoint(u0, v1);
			//glm::vec3 p11 = evaluateSurfacePoint(u1, v1);

			glm::vec3 p00 = E_delta(controlGrid, U, V, u0, v0, kU, kV, mU, mV);
			glm::vec3 p10 = E_delta(controlGrid, U, V, u1, v0, kU, kV, mU, mV);
			glm::vec3 p01 = E_delta(controlGrid, U, V, u0, v1, kU, kV, mU, mV);
			glm::vec3 p11 = E_delta(controlGrid, U, V, u1, v1, kU, kV, mU, mV);



			// First triangle
			cpuGeom.verts.push_back(p00);
			cpuGeom.verts.push_back(p10);
			cpuGeom.verts.push_back(p01);

			// Second triangle
			cpuGeom.verts.push_back(p01);
			cpuGeom.verts.push_back(p10);
			cpuGeom.verts.push_back(p11);

			// Colors (for now all green)
			//for (int k = 0; k < 6; ++k)
			//	cpuGeom.cols.push_back(glm::vec3(0.2f, 0.8f, 0.3f));
		}
	}
	gpuGeom.bind();
	gpuGeom.setVerts(cpuGeom.verts);
	//gpuGeom.setCols(cpuGeom.cols);
}

void Surface::bind() {
	gpuGeom.bind();
}

size_t Surface::numVerts() {
	return cpuGeom.verts.size();
}