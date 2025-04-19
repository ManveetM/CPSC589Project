#pragma once

float hash(float x, float z) {
    return glm::fract(sin(glm::dot(glm::vec2(x, z), glm::vec2(12.9898f, 78.233f))) * 43758.5453f);
}

float generateNoise(float x, float z, float scale = 0.5f, float amplitude = 1.0f) {
    float val = hash(x * scale, z * scale);
    return (val * 2.0f - 1.0f) * amplitude; // range [-amplitude, amplitude]
}