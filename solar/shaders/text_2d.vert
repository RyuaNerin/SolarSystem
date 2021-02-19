// https://learnopengl.com/In-Practice/2D-Game/Render-text

#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 inTexCoord;

out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    gl_Position.z = vertex.z;
    TexCoords = inTexCoord;
} 
