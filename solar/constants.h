// 상수값들이 저장된 파일

#pragma once

#include <string>

#include <glm/gtc/constants.hpp>

#include "model_text.h"
#include "ui.h"
#include "v.h"

// 실제 행성 비율을 사용할 것인지
//#define USE_REAL_DISTANCE

#ifdef _DEBUG
constexpr bool IS_DEBUG = true;
#else
constexpr bool IS_DEBUG = false;
#endif

/********************************************************************************/

inline std::string getTexturePath(const std::string& textureName) { return "textures\\" + textureName + ".png"; } // 텍스쳐 위치 가져오는 함수
inline std::string getShaderPath (const std::string& shaderName ) { return "shaders\\"  + shaderName          ; } // 셰이더 위치 가져오는 함수

/********************************************************************************/
// 상수값

constexpr auto PI   = glm::pi<float>();
constexpr auto PI2  = glm::two_pi<float>();
constexpr auto PI_2 = glm::half_pi<float>();

constexpr double au2unit(double v) { return v * 10; } // AU 단위를 opengl 좌표 단위로 변환하는 함수
constexpr float km2unit(float v) { return v / 1000000; } // KM 단위를 opengl 좌표 단위로 변환하는 함수

/********************************************************************************/
// 셰이더

constexpr const char* GeoMetry2D_SHADER = "geometry_2d";
constexpr const char* GeoMetry2D_TEX_SHADER = "geometry_2d_tex";

/********************************************************************************/
// 폰트 정보

// 쓸 폰트 크기를 몇개 미리 지정해두기
constexpr int FONT_SIZE_1 = 11;
constexpr int FONT_SIZE_2 = 13;
constexpr int FONT_SIZE_3 = 15;
constexpr int FONT_SIZE_4 = 17;
constexpr int FONT_SIZE_5 = 20;
constexpr int FONT_SIZE_6 = 30;

constexpr const char* FONT_PATH = "resources/D2Coding-Ver1.3.2-20180524.ttf";
constexpr const char* FONT_SHADER = "text_2d";

/********************************************************************************/

// MSAA 샘플링 값
constexpr int OPENGL_MSAA_SAMPLES = 8;
constexpr float OPENGL_ANISTROPY = 16;

constexpr v::Size2i    WINDOW_SIZE    = { 1024, 768 };  // 기본 윈도우 크기
constexpr const char*  WINDOW_CAPTION = "Solar System"; // 윈도우 타이틀

// 텍스트 쓸 때 외각 여백
constexpr float WINDOW_TEXT_MARGIN = 20; 

// 로딩
constexpr const wchar_t*   WINDOW_LOADING_TEXT_FORMAT = 
    L"데이터를 불러오고 있습니다\n"
    L"%d / %d";
constexpr v::Color         WINDOW_LOADING_TEXT_COLOR       = 0xEEEEEE; // LOADING 글자
constexpr int              WINDOW_LOADING_TEXT_SIZE        = FONT_SIZE_6; // 폰트
constexpr model::Alignment WINDOW_LOADING_TEXT_ALIGN_HORIZ = model::Alignment::Middle; // 가로 정렬
constexpr model::Alignment WINDOW_LOADING_TEXT_ALIGN_VERTI = model::Alignment::Middle; // 세로 정렬

// 워터마크박기
constexpr const wchar_t*   WINDOW_WATERMARK_TEXT = L"github.com/RyuaNerin/SolarSystem";
constexpr v::Color         WINDOW_WATERMARK_TEXT_COLOR       = { 0xFFFFFF, 0.4f };
constexpr int              WINDOW_WATERMARK_TEXT_SIZE        = FONT_SIZE_2;
constexpr model::Alignment WINDOW_WATERMARK_TEXT_ALIGN_HORIZ = model::Alignment::Far;
constexpr model::Alignment WINDOW_WATERMARK_TEXT_ALIGN_VERTI = model::Alignment::Far;

// 도움말
constexpr v::Color         WINDOW_HELP_TEXT_COLOR       = 0xFFFFFF;
constexpr int              WINDOW_HELP_TEXT_SIZE        = FONT_SIZE_2;
constexpr model::Alignment WINDOW_HELP_TEXT_ALIGN_HORIZ = model::Alignment::Near; // 가로 정렬
constexpr model::Alignment WINDOW_HELP_TEXT_ALIGN_VERTI = model::Alignment::Near; // 세로 정렬

// 일시정지 메시지
constexpr const wchar_t*   WINDOW_PAUSED_TEXT             = L"일시정지됨";
constexpr v::Color         WINDOW_PAUSED_TEXT_COLOR       = 0xFFFFFF;
constexpr int              WINDOW_PAUSED_TEXT_SIZE        = FONT_SIZE_6;
constexpr model::Alignment WINDOW_PAUSED_TEXT_ALIGN_HORIZ = model::Alignment::Middle; // 가로 정렬
constexpr model::Alignment WINDOW_PAUSED_TEXT_ALIGN_VERTI = model::Alignment::Near;   // 세로 정렬

// 날자
constexpr const char*      WINDOW_DATE_TEXT             = "%04d-%02d-%02d %02d:%02d:%02d UTC";
constexpr v::Color         WINDOW_DATE_TEXT_COLOR       = 0xFFFFFF;
constexpr int              WINDOW_DATE_TEXT_SIZE        = FONT_SIZE_4;
constexpr model::Alignment WINDOW_DATE_TEXT_ALIGN_HORIZ = model::Alignment::Near;  // 가로 정렬
constexpr model::Alignment WINDOW_DATE_TEXT_ALIGN_VERTI = model::Alignment::Far; // 세로 정렬

// 재생 속도
constexpr const char*      WINDOW_SPEED_TEXT_FORMAT       = "%.3lf days / sec";
constexpr const char*      WINDOW_SPEED_TEXT_FORMAT_EX    = "0000.00 days / sec"; // 버퍼 폭 계산용 텍스트 예시
constexpr v::Color         WINDOW_SPEED_TEXT_COLOR        = 0xFFFFFF;
constexpr int              WINDOW_SPEED_TEXT_SIZE         = FONT_SIZE_3;
constexpr model::Alignment WINDOW_SPEED_TEXT_ALIGN_HORIZ  = model::Alignment::Far;  // 가로 정렬
constexpr model::Alignment WINDOW_SPEED_TEXT_ALIGN_VERTI  = model::Alignment::Near; // 세로 정렬

// FPS
// fps 표시 간격 (ms)
constexpr float            WINDOW_FPS_UPDATE_INTERVAL   = 500.0f; // 갱신 간걱. ms
constexpr const char*      WINDOW_FPS_TEXT_FORMAT       = "fps : %.1f (%.2f ms)";
constexpr const char*      WINDOW_FPS_TEXT_FORMAT_EX    = "fps : 0000.0 (0000.00 ms)"; // 버퍼 폭 계산용 텍스트 예시
constexpr v::Color         WINDOW_FPS_TEXT_COLOR        = 0xFFFFFF;
constexpr int              WINDOW_FPS_TEXT_SIZE         = FONT_SIZE_3;
constexpr model::Alignment WINDOW_FPS_TEXT_ALIGN_HORIZ  = model::Alignment::Far;  // 가로 정렬
constexpr model::Alignment WINDOW_FPS_TEXT_ALIGN_VERTI  = model::Alignment::Near; // 세로 정렬

constexpr int WINDOW_DRAG_MIN = 2; // 이 이상 움직이면 마우스 드래그로 간주. (픽셀)

constexpr int WINDOW_LONG_PRESSED_TIME = 700; // 이 이상 누르고 있으면 꾹 누른 버튼으로 간주. (ms)

/********************************************************************************/

constexpr float CAMERA_PERSPECTIVE_FOV  =    45;      // FOV
constexpr float CAMERA_PERSPECTIVE_NEAR =     0.001f; // zNear
constexpr float CAMERA_PERSPECTIVE_FAR  = 30000;      // zFar

constexpr float CAMERA_RADIUS_SPEED_KEY   =    1.80f; // pow per sec    => 1 * n * n * n * n ...
constexpr float CAMERA_RADIUS_SPEED_MOUSE =    1.05f; // pow per scroll => 1 * n * n * n * n ...
constexpr float CAMERA_RADIUS_DEFAULT     =   80.00f; // 기본
constexpr float CAMERA_RADIUS_MIN         =    0.01f; // 최소
constexpr float CAMERA_RADIUS_MAX         = 1000.00f; // 최대

constexpr float CAMERA_ELEVATION_SPEED_KEY   = 180.0f / 1;   // delta/sec
constexpr float CAMERA_ELEVATION_SPEED_MOUSE = 180.0f / 500; // delta/pixel
constexpr float CAMERA_ELEVATION_DEFAULT     =  20;          // 기본값
constexpr float CAMERA_ELEVATION_MIN         = -90;          // 최소
constexpr float CAMERA_ELEVATION_MAX         =  90;          // 최대

constexpr float CAMERA_AZIMUTH_SPEED_KEY   = 360.0f / 2;   // delta/sec
constexpr float CAMERA_AZIMUTH_SPEED_MOUSE = 180.0f / 500; // delta/pixel
constexpr float CAMERA_AZIMUTH_DEFAULT     =   0;          // 기본값

constexpr float CAMERA_MOVEMENT_DELTA = 0.2f; // percent/sec = 5초동안 화면의 100% 만큼 움직임

/********************************************************************************/
// 은하수 배경 = milky-way

//constexpr const char* MILKYWAY_MODEL_TEXTURE = "stars"; // 배경화면 텍스쳐
constexpr const char* MILKYWAY_MODEL_TEXTURE = "stars_milky_way"; // 배경화면 텍스쳐
constexpr const char* MILKYWAY_MODEL_SHADER  = "milkyway";        // 배경화면 셰이더

constexpr float MILKYWAY_MODEL_RADIUS = 1;  // 크기
constexpr int   MILKYWAY_MODEL_STACKS = 5; // 세로 분할
constexpr int   MILKYWAY_MODEL_SLICES = 5; // 가로 분할

/********************************************************************************/
// 재생 속도
constexpr float SPEED_DELTA_SPEED_WHEEL = 1.10f; // 1 휠당 변경값 => 1 * n * n * n * n ...
constexpr float SPEED_DELTA_SPEED_KEY   = 1.80f; // 1 초당 변경값 => 1 * n * n * n * n ...

constexpr float SPEED_MIN     =    0.01f; //    0.1 일 / 초
constexpr float SPEED_DEFAULT =   30.00f; //   30   일 / 초
constexpr float SPEED_MAX     = 3650.00f; // 3650   일 / 초

/********************************************************************************/
// 궤도

constexpr const char* ORBIT_SHADER = "orbit";

constexpr v::Color ORBIT_COLOR = { 0x888888, 0.4f }; // 궤도 기본 색

constexpr float ORBIT_GRADIENT_START_ANGLE = 60;   // 이만큼은 지나고 나서 궤도 표시
constexpr float ORBIT_GRADIENT_ALPHA_MAX   = 0.6f; // 궤도 알파 최대값

constexpr float ORBIT_SOLID_WIDTH = 1; // 궤도 두께
constexpr float ORBIT_DOTTED_SIZE = 1; // 점 크기

constexpr float ORBIT_SOLID_DAY_PER_VERTEX = 0.25; // n 일마다 버텍스 한개
constexpr float ORBIT_DOTTED_DAY_PER_VERTEX = 2; // n 일마다 버텍스 한개

/********************************************************************************/
// 조명 상수

constexpr float    LIGHT_AMBIENT        = 0.02f;  // 행성 기본 밝기
constexpr float    LIGHT_EARTH_AMBIENT  = 0.1f;  // 지구는 밤에도 밝게

constexpr v::Color LIGHT_COLOR     = 0xFFFFFF; // 조명 색
constexpr float    LIGHT_DIFFUSE   = 1;        // 조명 밝기
constexpr float    LIGHT_SPECULAR  = 0.3f;     // 행성 반사광 (빛을 받으면 더 환해보임)
constexpr int      LIGHT_SHININESS = 4;        // 반사광빛나는 정도

/********************************************************************************/
// 행성 관련

constexpr float PLANET_SIZE_MUL = 10; // 행성 크기 배율

constexpr const char* SUN_SHADER = "sun";

constexpr const char* PLANET_MODEL_SHADER_NAME = "planet";
constexpr int         PLANET_MODEL_SLICES_AND_STACKS = 100; // 행성 크기 1 당 slices 와 stacks 크기

constexpr const char* PLANET_SATURN_RING_SHADER           = "saturn_ring";
constexpr const char* PLANET_SATURN_RING_TEXTURE          = "saturn_ring_alpha";
constexpr int         PLANET_SATURN_RING_PARTICLES        = 720;   // 토성 고리 파티클 분할 수
constexpr float       PLANET_STAURN_RING_RAIDUS_MUL_IN    = 1.14f; // 토성 고리 안쪽 위치
constexpr float       PLANET_STAURN_RING_RAIDUS_MUL_OUT   = 2.40f; // 토성 고리 바깥쪽 위치
constexpr float       PLANET_SATURN_RING_ALPHA            = 0.80f; // 토성 고리를 좀 더 투명하게 하기 위한 알파값

// 행성 정보 관련 텍스트 색 : 행성 위쪽 -> render.cpp
constexpr v::Color PLANET_NAME_TEXT_COLOR       = { 0xFFFFFF, 0.8f }; // 색
constexpr int      PLANET_NAME_TEXT_SIZE_NEAR   = FONT_SIZE_6; // 확대 시 글씨 크기
constexpr int      PLANET_NAME_TEXT_SIZE_FAR    = FONT_SIZE_1; // 축소 시 글씨 크기
constexpr float    PLANET_NAME_TEXT_SIZE_NEAR_D = 1.0;        // 글씨 크기가 제일 클 때의 거리
constexpr float    PLANET_NAME_TEXT_SIZE_FAR_D  = 4.0;        // 글씨 크기가 제일 작을 때의 거리

// 지구 뒷쪽
constexpr const char* PLANET_EARTH_SPECULAR_TEXTURE = "earth_specular_map";
constexpr const char* PLANET_EARTH_NIGHT_TEXTURE    = "earth_nightmap";
constexpr const char* PLANET_EARTH_CLOUD_TEXTURE    = "earth_clouds";
constexpr float       PLANET_EARTH_CLOUD_NOVE_DELTA = 0.001f;

/********************************************************************************/
