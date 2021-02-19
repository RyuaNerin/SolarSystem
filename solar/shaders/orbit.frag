#version 330 core

uniform float angleVisible;
uniform float alphaGradient;

uniform float angleCurrent;
uniform vec4  color;

in float Angle;

// 출력물
out vec4  FragColor;

void main()
{
    FragColor = color;

    // angleCurrent == -1 이면 그라디언트 사용 안 함.
    if (angleCurrent == -1)
    {
        return;
    }

    // alphaGradient 을 기준으로 상대적인 각도를 구한다
    // 단, 반시계방향이니까 주의
    float angle = Angle - angleCurrent;
    if (angle < 0) angle = 360 + angle;

    // angleVisible 까지 색 넣지 않기
    if (angle < angleVisible)
    {
        FragColor = vec4(0);
        return;
    }

    // 적절하게 색 넣기
    FragColor.a = alphaGradient * angle / (360 - angleVisible);
}
