#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 3) in vec3 col;

out vec3 C;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main() {
	C = col;
	gl_Position = P * V * M * vec4(pos, 1.0);
}
