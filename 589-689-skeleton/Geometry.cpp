#include "Geometry.h"

#include <utility>


GPU_Geometry::GPU_Geometry()
	: vao()
	, vertBuffer(0, 3, GL_FLOAT)
	, uvBuffer(1, 2, GL_FLOAT)
	, normalsBuffer(2, 3, GL_FLOAT)
	, colBuffer(3, 3, GL_FLOAT)
	, ebo(4, 3, GL_FLOAT)
{}


void GPU_Geometry::setVerts(const std::vector<glm::vec3>& verts) {
	vertBuffer.uploadData(sizeof(glm::vec3) * verts.size(), verts.data(), GL_STATIC_DRAW);
}


void GPU_Geometry::setUVs(const std::vector<glm::vec2>& uvs) {
	uvBuffer.uploadData(sizeof(glm::vec2) * uvs.size(), uvs.data(), GL_STATIC_DRAW);
}

void GPU_Geometry::setNormals(const std::vector<glm::vec3>& norms) {
	normalsBuffer.uploadData(sizeof(glm::vec3) * norms.size(), norms.data(), GL_STATIC_DRAW);
}

void GPU_Geometry::setCols(const std::vector<glm::vec3>& cols) {
	colBuffer.uploadData(sizeof(glm::vec3) * cols.size(), cols.data(), GL_STATIC_DRAW);
}

void GPU_Geometry::setIndices(const std::vector<unsigned int>& indices)
{
	ebo.uploadData(sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);
}