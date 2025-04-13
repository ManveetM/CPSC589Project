#include "PlantPart.h"

void PlantPart::generatePlantPart() {

    surface.clear();

    std::vector<glm::dvec3> transformedCurve(crossSectionCurve.begin(), crossSectionCurve.end());
    glm::dvec3 startPoint = transformedCurve.front();
    glm::dvec3 endPoint = transformedCurve.back();
    glm::dvec3 midpoint = (startPoint + endPoint) * 0.5;

    glm::dmat4 translationMatrix = glm::translate(glm::dmat4(1.0), -midpoint);
    for (auto& point : transformedCurve) {
        glm::dvec4 translatedPoint = translationMatrix * glm::dvec4(point, 1.0);
        point = glm::dvec3(translatedPoint);
    }

    // Recalculate start and end points after translation
    startPoint = transformedCurve.front();
    endPoint = transformedCurve.back();

    glm::dvec3 direction = glm::normalize(endPoint - startPoint);
    glm::dvec3 xAxis(1.0f, 0.0f, 0.0f);
    double angle = acos(glm::clamp(glm::dot(direction, xAxis), -1.0, 1.0));
	std::cout << angle << std::endl;

    // Rotate the curve around the z-axis and x-axis
    glm::dmat4 rotationMatrix = glm::rotate(glm::dmat4(1.0), -angle, glm::dvec3(0.0, 0.0, 1.0));
    rotationMatrix = glm::rotate(glm::dmat4(1.0), glm::radians(-90.0), xAxis) * rotationMatrix;
    for (auto& point : transformedCurve) {
        glm::dvec4 rotatedPoint = rotationMatrix * glm::dvec4(point, 1.0);
        point = glm::dvec3(rotatedPoint);
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

        int N = surfaceStrips.size();
        int M = transformedCurve.size();

        for (int i = 0; i < N - 1; ++i) {
            for (int j = 0; j < M; ++j) {
                glm::dvec3 v0 = surfaceStrips[i][j];
                glm::dvec3 v1 = surfaceStrips[i + 1][j];
                glm::dvec3 v2 = surfaceStrips[i][(j + 1) % M];
                glm::dvec3 v3 = surfaceStrips[i + 1][(j + 1) % M];

                surface.push_back(glm::vec3(v0));
                surface.push_back(glm::vec3(v1));
                surface.push_back(glm::vec3(v2));
                surface.push_back(glm::vec3(v2));
                surface.push_back(glm::vec3(v1));
                surface.push_back(glm::vec3(v3));
            }
        }
    }

	surfaceGenerated = true;
}
