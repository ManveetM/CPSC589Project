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
        , partTransformMatrix(glm::mat4(1.0f))
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

    const glm::mat4& getPartTransformMatrix() const {
        return partTransformMatrix;
    }

    bool isUpdateNeeded() const {
        return needsUpdate;
    }

    const std::vector<glm::vec3>& getSurface() const {
        return surface;
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

    void setPartTransformMatrix(const glm::mat4& matrix) {
        partTransformMatrix = matrix;
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

    glm::vec3 scale;
    glm::vec3 translation;
	glm::vec3 rotation;

    glm::mat4 partTransformMatrix;

    bool surfaceGenerated = false;
};