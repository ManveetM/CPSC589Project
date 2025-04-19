#include "PlantPart.h"

void PlantPart::generatePlantPart() {

    normals.clear();
    indices.clear();
    cols.clear();
    surface.clear();

    if (crossSectionCurve.size() == 0) {
        return;
    }

    std::vector<glm::dvec3> transformedCurve(crossSectionCurve.begin(), crossSectionCurve.end());
    glm::dvec3 startPoint = transformedCurve.front();
    glm::dvec3 endPoint = transformedCurve.back();
    glm::dvec3 midpoint = (startPoint + endPoint) * 0.5;

    glm::dmat4 translationMatrix = glm::translate(glm::dmat4(1.0), -midpoint);
    for (auto& point : transformedCurve) {
        glm::dvec4 translatedPoint = translationMatrix * glm::dvec4(point, 1.0);
        point = glm::dvec3(translatedPoint);
    }

    startPoint = transformedCurve.front();
    endPoint = transformedCurve.back();

    glm::dvec3 direction = glm::normalize(endPoint - startPoint);
    glm::dvec3 xAxis(1.0, 0.0, 0.0);

    double angleZ = acos(glm::clamp(glm::dot(direction, xAxis), -1.0, 1.0));
    if (direction.y > 0) {
        angleZ = -angleZ;
    }

    glm::dmat4 rotationZMatrix = glm::rotate(glm::dmat4(1.0), angleZ, glm::dvec3(0.0, 0.0, 1.0));
    for (auto& point : transformedCurve) {
        glm::dvec4 rotatedPoint = rotationZMatrix * glm::dvec4(point, 1.0);
        point = glm::dvec3(rotatedPoint.x, rotatedPoint.z, rotatedPoint.y);
    }

    startPoint = transformedCurve.front();
    endPoint = transformedCurve.back();

    double length = glm::length(endPoint - startPoint);
    if (length > 1e-4) {
        double scaleFactor = 1.0 / length;

        glm::dmat4 scaleMatrix = glm::scale(glm::dmat4(1.0), glm::dvec3(scaleFactor));
        for (auto& point : transformedCurve) {
            glm::dvec4 scaledPoint = scaleMatrix * glm::dvec4(point, 1.0);
            point = glm::dvec3(scaledPoint);
        }

        for (size_t i = transformedCurve.size() - 2; i > 0; --i) {
            glm::dvec3 reflectedPoint = transformedCurve[i];
            reflectedPoint.z = -reflectedPoint.z;
            transformedCurve.push_back(reflectedPoint);
        }

        std::vector<std::vector<glm::dvec3>> surfaceStrips;

        for (int i = 0; i < leftCurve.size(); ++i) {
            glm::dvec3 ql = leftCurve[i];
            glm::dvec3 qr = rightCurve[i];
            glm::dvec3 axis = qr - ql;
            glm::dvec3 mid = (ql + qr) * 0.5;

            glm::dvec3 axisDir = glm::normalize(axis);
            double angle = acos(glm::dot(glm::dvec3(1.0f, 0.0, 0.0), axisDir));

            glm::dmat4 transform = glm::translate(glm::dmat4(1.0), mid)
                * glm::rotate(glm::dmat4(1.0), angle, glm::dvec3(0.0, 0.0, 1.0f))
                * glm::scale(glm::dmat4(1.0), glm::dvec3(glm::length(axis)));

            std::vector<glm::dvec3> strip;

            for (const auto& pt : transformedCurve) {
                glm::dvec4 transformed = transform * glm::dvec4(pt, 1.0);
                strip.push_back(glm::dvec3(transformed));
            }

            surfaceStrips.push_back(strip);
        }

        for (const auto& strip : surfaceStrips) {
            for (const auto& point : strip) {
                surface.push_back(glm::vec3(point));
            }
        }

        int N = surfaceStrips.size();
        int M = transformedCurve.size();

        for (int i = 0; i < N - 1; ++i) {
            for (int j = 0; j < M; ++j) {
                unsigned int v0 = i * M + j;
                unsigned int v1 = i * M + (j + 1) % M;
                unsigned int v2 = (i + 1) * M + j;
                unsigned int v3 = (i + 1) * M + (j + 1) % M;
        
                indices.push_back(v0);
                indices.push_back(v1);
                indices.push_back(v2);
        
                indices.push_back(v2);
                indices.push_back(v1);
                indices.push_back(v3);
        
                glm::vec3 vec1 = surface[v1] - surface[v0];
                glm::vec3 vec2 = surface[v2] - surface[v0];
                glm::vec3 normal = glm::normalize(glm::cross(vec1, vec2));

                normals.push_back(normal);
            }
        }

        for (int i = 0; i < transformedCurve.size(); ++i) {
			normals.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
        }
        
        // Normalize all accumulated normals
        for (auto& normal : normals) {
            normal = glm::normalize(normal);
        }
    }

    cols = std::vector<glm::vec3>(surface.size(), baseColor);
    surfaceGenerated = true;
}