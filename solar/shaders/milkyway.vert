#version 330 core

// 상수
uniform mat4 projectionMatrix;	// Projection-Matrix
uniform mat4 viewMatrix;	    // View-Matrix

// 입력 데이터
layout (location = 0) in vec3 inPos;     // 벡터
layout (location = 1) in vec2 texCoords; // 텍스쳐 매핑

// frag 로 넘길 것들표
out vec2 TexCoords; // 텍스쳐 좌표

void main()
{
	vec4 pos = projectionMatrix * viewMatrix * vec4(inPos, 1);
    gl_Position = pos.xyww; // z 값을 w 로 설정해서 깊이검사 최적화
	
    TexCoords = texCoords;
}
