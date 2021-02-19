#version 330 core

// 상수
uniform mat4  projectionMatrix;	// Projection-Matrix
uniform mat4  viewMatrix;	    // View-Matrix
uniform mat4  modelMatrix;		// Model-Matrix

// 입력 데이터
layout (location = 0) in vec4 inPos;

// 넘기기
out float Angle;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(inPos.xyz, 1);
    Angle = inPos.w;
}
