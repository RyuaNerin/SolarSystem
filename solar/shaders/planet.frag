#version 330 core

// 상수
struct Light
{
    vec3  color;     // 광원 색
    vec3  pos;       // 광원 좌표
    float ambient;   // 반사광 = 기본 밝기 크기
    float diffuse;   // diffuse 세기
    float specular;  // specular 밝기 세기값
    int   shininess; // specular shininess. = 빛남 정도
};

uniform Light     light;              // 조명        : 로컬 기준임!!
uniform vec3      cameraPos;          // 카메라 위치 : 로컬 기준임!!
uniform sampler2D shaderTexture;      // 셰이더 텍스쳐 (주간)

// 지구용
uniform int       isEarth;
uniform sampler2D shaderTextureSpecular; // 반사광
uniform sampler2D shaderTextureNight;    // 야간 텍스쳐
uniform sampler2D shaderTextureCloud;    // 구름 텍스쳐

// vert 에서 넘어오는 것들.
in vec2 TexCoords; // 텍스쳐 좌표
in vec3 Normal;    // 노말벡터
in vec3 FragPos;   // fragment 좌표

// 출력물
out vec4 FragColor;

float getBrightness(vec3 c)
{
    return 0.299f * c.r + 0.587f * c.g + 0.114f * c.b;
}

void main()
{
    // 태양은 언제나 태양색
    if (light.shininess == 0)
    {
        FragColor = texture2D(shaderTexture, TexCoords);
        return;
    }
    
    /********************************************************************************/
    // 광선 밝기 계산하는 부분

    // ambient
    vec3 ambient = light.color * light.ambient;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.pos - FragPos);

    float nDotL = dot(norm, lightDir);
    float diff = max(nDotL, 0.0);

    vec3 diffuse = light.color * (diff * light.diffuse);
    
    // specular
    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), light.shininess);
    vec3 specular = light.color * (spec * light.specular);

    /********************************************************************************/

    // 지구 외 행성
    if (isEarth == 0)
    {
        // Phong
        FragColor = texture2D(shaderTexture, TexCoords) * vec4(ambient + diffuse + specular, 1);
        return;
    }
    
    vec3 colorDay      = texture2D(shaderTexture,         TexCoords).rgb;
    vec3 colorNight    = texture2D(shaderTextureNight,    TexCoords).rgb;
    vec3 colorSpecular = texture2D(shaderTextureSpecular, TexCoords).rgb;
    vec3 colorCloud    = texture2D(shaderTextureNight,    TexCoords).rgb;

    // Phong
    colorSpecular = colorSpecular * specular;
    colorDay = colorDay * (ambient + diffuse);

    // 밝기를 확인해서 0.5 미만은 잘라내어 밝은 부분만 추출한다.
    if (getBrightness(colorNight) < 0.3)
    {
        colorNight = vec3(0);
    }
    
    // 밤의 야간조명. 부드럽게 처리하기 위해서 smoothstep 함수 사용
    float nDotLStep = smoothstep(-0.05, 0.05, nDotL);
    vec3 color = colorDay + colorNight * (1 - nDotLStep) + colorSpecular;

    // 구름 추가. 구름은 반사안함
    // 구름이 너무 밝아서 밝기 낮추고
    // 아침에는 잘 보이고 저녁에는 잘 안보이게 nDotL 를 범위 내에서 추가
    color += texture2D(shaderTextureCloud, TexCoords).r * 0.3 * clamp(nDotL, 0.1, 1.0);

    FragColor = vec4(color, 1);
}
