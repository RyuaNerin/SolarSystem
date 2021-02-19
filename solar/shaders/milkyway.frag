#version 330 core

// 상수
uniform sampler2D shaderTexture;

// vert 에서 넘어오는 것들.
in vec2 TexCoords; // 텍스쳐 좌표

// 출력물
out vec4 FragColor;

void main()
{
    FragColor = texture2D(shaderTexture, TexCoords);
}
