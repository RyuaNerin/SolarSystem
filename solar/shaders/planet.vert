#version 330 core

// 상수
uniform mat4 projectionMatrix;	// Projection-Matrix
uniform mat4 viewMatrix;	    // View-Matrix
uniform mat4 modelMatrix;		// Model-Matrix

// 입력 데이터
layout (location = 0) in vec3 inPos;     // 버텍스
layout (location = 1) in vec3 inNormal;  // 노말 벡터 = 법선벡터
layout (location = 2) in vec2 texCoords; // 텍스쳐 매핑

// frag 로 넘길 것들
out vec2 TexCoords; // 텍스쳐 좌표
out vec3 Normal;    // 노말벡터
out vec3 FragPos;   // fragment 좌표

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(inPos, 1);
	
    TexCoords = texCoords;
    Normal = vec3(viewMatrix * modelMatrix * vec4(inNormal, 0));
    FragPos = vec3(viewMatrix * modelMatrix * vec4(inPos, 1.0));
}
