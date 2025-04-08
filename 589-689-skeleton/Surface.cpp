// BSplineSurface.cpp
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

glm::vec3 Surface::E_delta_1(const std::vector<glm::vec3>& ctrlPts, const std::vector<double>& U, float u, int k, int m) {
	int d = -1;
	for (int i = 0; i < m + k; ++i) {
		if (u >= U[i] && u < U[i + 1]) {
			d = i;
			break;
		}
	}
	if (d == -1) d = m;

	std::vector<glm::vec3> C;
	for (int i = 0; i < k; ++i) {
		int idx = std::max(0, d - i);
		C.push_back(ctrlPts[idx]);
	}

	for (int r = k; r >= 2; --r) {
		int i = d;
		for (int s = 0; s <= r - 2; ++s) {
			float omega = (u - U[i]) / (U[i + r - 1] - U[i]);
			C[s] = omega * C[s] + (1 - omega) * C[s + 1];
			i--;
		}
	}
	return C[0];
}

glm::vec3 Surface::evaluateSurfacePoint(float u, float v) {
	int mU = controlGrid.size() - 1;
	int mV = controlGrid[0].size() - 1;

	auto U = initializeKnot(kU, mU);
	auto V = initializeKnot(kV, mV);

	// Interpolate along U (for each column)
	std::vector<glm::vec3> tempCurve;
	for (int i = 0; i <= mV; ++i) {
		std::vector<glm::vec3> col = controlGrid[i];
		tempCurve.push_back(E_delta_1(col, U, u, kU, mU));
	}

	return E_delta_1(tempCurve, V, v, kV, mV);
}

void Surface::generateSurface() {
	cpuGeom.verts.clear();
	cpuGeom.normals.clear();
	//cpuGeom.cols.clear();

	for (int i = 0; i < resU - 1; ++i) {
		float u0 = float(i) / (resU - 1);
		float u1 = float(i + 1) / (resU - 1);
		for (int j = 0; j < resV - 1; ++j) {
			float v0 = float(j) / (resV - 1);
			float v1 = float(j + 1) / (resV - 1);

			glm::vec3 p00 = evaluateSurfacePoint(u0, v0);
			glm::vec3 p10 = evaluateSurfacePoint(u1, v0);
			glm::vec3 p01 = evaluateSurfacePoint(u0, v1);
			glm::vec3 p11 = evaluateSurfacePoint(u1, v1);

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

// #include "Surface.h"

// class Surface {
//     public:
//         Surface(int gridSize) : gridSize(gridSize) {
//             //std::vector<std::vector<glm::vec3>> controlGrid;

//             int gridSize = gridSize;
//             float spacing = 1.0f; // distance between points

//             float half = (gridSize - 1) * spacing / 2.0f; // to center the grid around (0, 0)

//             for (int i = 0; i < gridSize; ++i) {
//                 std::vector<glm::vec3> row;
//                 for (int j = 0; j < gridSize; ++j) {
//                     float x = j * spacing - half; // x-axis
//                     float y = 0.0f;               // height (can be adjusted later)
//                     float z = i * spacing - half; // z-axis
//                     row.push_back(glm::vec3(x, y, z));
//                 }
//                 controlGrid.push_back(row);
//             }
//         }

//         void initializeSurface() {
//             for (int i = 0; i < resolutionU; ++i) {
//                 float u = float(i) / (resolutionU - 1);

//                 for (int j = 0; j < resolutionV; ++j) {
//                     float v = float(j) / (resolutionV - 1);
//                     glm::vec3 point = evaluateBSplineSurface(u, v, controlGrid, 3, 3);
//                     surface.verts.push_back(point);
//                 }
//             }
//         }

//     private:
//         std::vector<std::vector<glm::vec3>> controlGrid;
//         int gridSize = 5;
//         CPU_Geometry surface;
//         const int resolutionU = 50; // number of samples in u direction
//         const int resolutionV = 50; // number of samples in v direction

//         CPU_Geometry createBSpline(std::vector<std::vector<glm::vec3>> pointsData, int k, float uIncr) {
//             CPU_Geometry curve;
//             int m = pointsData.data.verts.size();
        
//             // Only run if more than 2 points and k is not greater than number of points
//             if (pointsData.data.verts.size() >= 2 && pointsData.data.verts.size() >= k) {
//                 // setup of standard knot sequence
//                 std::vector<double> U = initializeKnot(k, m);
//                 // Calculate S(u) over parameter domain
//                 for (float u = U[k-1]; u < U[m+2]; u+=uIncr) {
//                     glm::vec3 point = E_delta_1(pointsData, U, u, k, m);
//                     curve.verts.push_back(point);
//                     // curve.cols.push_back(glm::vec3(23.f/255.f, 176.f/255.f, 59.f/255.f));
//                 }
//                 // Make sure to calculate last point at u = 1
//                 glm::vec3 endpoint = E_delta_1(pointsData, U, 1.0f, k, m);
//                 curve.verts.push_back(endpoint);
//                 // curve.cols.push_back(glm::vec3(23.f/255.f, 176.f/255.f, 59.f/255.f));
//             }
        
//             return curve;
//         }
        
//         // Used to initialize an array of knots (standard sequence)
//         std::vector<double> initializeKnot(int k, int m) {
//             std::vector<double> U;
        
//             for (int i = 0; i < k; i++) {
//                 U.push_back(0.0);
//             }
        
//             double step = 1.0f / (m - k + 1);
//             for (int i = 1; i <= (m - k + 1); i++) {
//                 U.push_back(i*step);
//             }
        
//             for (int i = 0; i < k; i++) {
//                 U.push_back(1.0);
//             }
        
//             return U;
//         }
        
//         // Find delta value of chosen u
//         int delta(int k, int m, float u, std::vector<double>& U) {
//             for (int i = 0; i < (m + k - 1); i++) {
//                 if (u >= U[i] && u < U[i+1]) {
//                     return i;
//                 }
//             }
//             return -1;
//         }
        
//         // Function to calculate S(u) efficiently
//         glm::vec3 E_delta_1(std::vector<std::vector<glm::vec3>> pointsData, std::vector<double> U, float u, int k, int m) {
//             int d = delta(k, m, u, U);
            
//             std::vector<glm::vec3> C;
//             for (int i = 0; i <= k - 1; i++) {
//                 int index = std::max(0, d - i);
//                 C.push_back(pointsData.data.verts[index]);
//             }
            
//             std::vector<glm::vec3> geometry;
//             for (int r = k; r >= 2; r--) {
//                 int i = d;
//                 for (int s = 0; s <= r-2; s++) {
//                     float omega = (u - U[i])/(U[i + r - 1] - U[i]);
//                     C[s] = (omega * C[s]) + ((1 - omega) * C[s + 1]);
//                     i--;
//                 }
//             }
        
//             return C[0];
//         }

//         glm::vec3 evaluateBSplineSurface(
//             float u, float v,
//             const std::vector<std::vector<glm::vec3>>& controlGrid,
//             int kU, int kV
//         ) {
//             int mU = controlGrid.size();       // Number of control points in u direction
//             int mV = controlGrid[0].size();    // Number of control points in v direction
        
//             // 1. Create knot vectors
//             std::vector<double> U = initializeKnot(kU, mU);
//             std::vector<double> V = initializeKnot(kV, mV);
        
//             // 2. Evaluate along u first to create intermediate curve in v direction
//             std::vector<glm::vec3> tempCurve;
//             for (int j = 0; j < mV; ++j) {
//                 // Extract column j (fixed v) → curve in u
//                 PointsData colCurve;
//                 for (int i = 0; i < mU; ++i) {
//                     colCurve.data.verts.push_back(controlGrid[i][j]);
//                 }
        
//                 // Evaluate B-spline at u along this column
//                 glm::vec3 pointU = E_delta_1(colCurve, U, u, kU, mU);
//                 tempCurve.push_back(pointU);
//             }
        
//             // 3. Now evaluate result of tempCurve along v
//             PointsData tempPointsData;
//             tempPointsData.data.verts = tempCurve;
        
//             glm::vec3 finalPoint = E_delta_1(tempPointsData, V, v, kV, mV);
//             return finalPoint;
//         }
// };