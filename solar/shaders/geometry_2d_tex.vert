#version 330 core

uniform mat4 projectionMatrix;

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 intexCoord;

out vec2 TexCoords;

void main()
{
    gl_Position = projectionMatrix * vec4(inPos, 0.0, 1.0);
    TexCoords = intexCoord;
} 
