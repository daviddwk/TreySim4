#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aNormal;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 transform;
uniform uint frameIdx;
uniform uint frameLen;
uniform bool flip; // bool really

out vec2 vertUV;
out vec4 vertColor;

void main() {
    gl_Position = projection * view * transform * vec4(aPos, 1.0f);
    vertColor = aColor;
    if (!flip) {
        vertUV = vec2((1.0f / float(frameLen)) * (float(frameIdx) + aUV.x), -aUV.y);
    } else {
        vertUV = vec2((1.0f / float(frameLen)) * (float(frameIdx) - aUV.x + 1), -aUV.y);
    }
}
