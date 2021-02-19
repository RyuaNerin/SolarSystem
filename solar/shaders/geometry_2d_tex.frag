#version 330 core

uniform sampler2D shaderTexture;

in vec2 TexCoords;

out vec4 FragColor;

void main()
{
    FragColor = texture(shaderTexture, TexCoords);
}  
