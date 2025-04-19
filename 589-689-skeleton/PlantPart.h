#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "PointsData.h"

#include <iostream>
#include "glm/gtc/type_ptr.hpp"

class PlantPart {
public:
    PlantPart(const std::string& name)
        : name(name)
        , scale(1.0f, 1.0f, 1.0f)
        , translation(0.0f, 0.0f, 0.0f)
        , rotation(0.0f, 0.0f, 0.0f)
		, baseColor(0.0f, 0.0f, 0.0f)
        , needsUpdate(false) {}

    // Getters
    const std::string& getName() const {
        return name;
    }

    glm::vec3& getScale() {
        return scale;
    }

    glm::vec3& getTranslation() {
        return translation;
    }

    glm::vec3& getRotation() {
        return rotation;
    }

    glm::vec3& getBaseColor() {
        return baseColor;
    }

    const glm::mat4 getPartTransformMatrix() const {
		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    bool isUpdateNeeded() const {
        return needsUpdate;
    }

    const std::vector<glm::vec3>& getSurface() const {
        return surface;
    }

    const std::vector<unsigned int>& getIndices() const {
        return indices;
    }

    const std::vector<glm::vec3>& getNormals() const {
        return normals;
    }

    const std::vector<glm::vec3>& getCols() const {
        return cols;
    }

    PointsData& getLeftControlPoints() {
        return leftControlPoints;
    }

    PointsData& getRightControlPoints() {
        return rightControlPoints;
    }

    PointsData& getCrossSectionControlPoints() {
        return crossSectionControlPoints;
    }

    const std::vector<glm::vec3>& getLeftCurve() const {
        return leftCurve;
    }

    const std::vector<glm::vec3>& getRightCurve() const {
        return rightCurve;
    }

    const std::vector<glm::vec3>& getCrossSectionCurve() const {
        return crossSectionCurve;
    }

    // Setters
    void setName(const std::string& newName) {
        name = newName;
    }

    void setUpdateNeeded(bool update) {
        needsUpdate = update;
    }

    void setSurface(const std::vector<glm::vec3>& newSurface) {
        surface = newSurface;
    }

    void setLeftCurve(const std::vector<glm::vec3>& curve) {
        leftCurve = curve;
    }

    void setRightCurve(const std::vector<glm::vec3>& curve) {
        rightCurve = curve;
    }

    void setNormal(const std::vector<glm::vec3>& newNormal) {
        normals = newNormal;
    }

    void setCrossSectionCurve(const std::vector<glm::vec3>& curve) {
        crossSectionCurve = curve;
    }

    // Miscellaneous
    void clear() {
        leftControlPoints.clear();
        rightControlPoints.clear();
        crossSectionControlPoints.clear();
        leftCurve.clear();
        rightCurve.clear();
        crossSectionCurve.clear();
        surface.clear();
    }

	bool isSurfaceGenerated() {
		return surfaceGenerated;
	}

	void setSurfaceGenerated(bool generated) {
		surfaceGenerated = generated;
	}

    void generatePlantPart();

private:
    std::string name;

    PointsData leftControlPoints;
    PointsData rightControlPoints;
    PointsData crossSectionControlPoints;


    std::vector<glm::vec3> leftCurve;
    std::vector<glm::vec3> rightCurve;
    std::vector<glm::vec3> crossSectionCurve;

    bool needsUpdate;
    std::vector<glm::vec3> surface;
    std::vector<glm::vec3> cols;
    std::vector<glm::vec3> normals;
	std::vector<unsigned int> indices;

    glm::vec3 scale;
    glm::vec3 translation;
	glm::vec3 rotation;
    glm::vec3 baseColor;

    bool surfaceGenerated = false;
};