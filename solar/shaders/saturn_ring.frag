#version 330 core

uniform sampler2D shaderTexture;
uniform float     alpha = 1;

// 받기
in vec2 TexCoords;

// 출력물
out vec4 FragColor;

void main()
{
    FragColor = texture(shaderTexture, TexCoords);
    FragColor.a *= alpha;
}
