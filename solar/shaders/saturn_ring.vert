#version 330 core

// 상수
uniform mat4 projectionMatrix;	// Projection-Matrix
uniform mat4 viewMatrix;	    // View-Matrix
uniform mat4 modelMatrix;		// Model-Matrix

// 입력 데이터
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTextCoords;

// 넘기기
out vec2 TexCoords;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(inPos, 1);

	TexCoords = inTextCoords;
}
