#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "PlantPart.h"

class Plant {
public:
    Plant(const std::string& name)
        : name(name), modelMatrix(glm::mat4(1.0f)) {}

    const std::string& getName() const {
        return name;
    }

    void setName(const std::string& newName) {
        name = newName;
    }

    const glm::mat4& getModelMatrix() const {
        return modelMatrix;
    }

    void setModelMatrix(const glm::mat4& matrix) {
        modelMatrix = matrix;
    }

    void addPart(const PlantPart& part) {
        parts.push_back(part);
    }

    void removePart(int index) {
        if (index >= 0 && index < parts.size()) {
            parts.erase(parts.begin() + index);
        }
    }

    const std::vector<PlantPart>& getParts() const {
        return parts;
    }

    std::vector<PlantPart>& getParts() {
        return parts;
    }

private:
    std::string name;
    std::vector<PlantPart> parts;
    glm::mat4 modelMatrix;
};