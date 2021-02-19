# OpenGL 로 작성한 태양계

- 2020년 2학기 '컴퓨터 그래픽스 실습' 기말과제
- [LICENSE](LICENSE)

![screenshot](/ScreenShot%20-%201.png)
![screenshot](/ScreenShot%20-%202.png)
![screenshot](/ScreenShot%20-%203.png)
![screenshot](/ScreenShot%20-%204.png)

## 구현 내용

- OpenGL Modern
	- [Glad](https://glad.dav1d.de/)
		- OpenGL 3.3 `compatibility`
			- `GL_EXT_multisample`
		- WGL
			- `WGL_ARB_create_context`
			- `WGL_ARB_create_context_profile`
			- `WGL_ARB_extensions_string`
			- `WGL_EXT_extensions_string`
	- [GLFW v3](https://www.glfw.org/)
	- 퐁 쉐이딩 (pong shading)
	- 셰이더를 사용한 고속 3D 렌더링
	- 비동기 텍스쳐 로딩
- 폰트 : [D2Coding](https://github.com/naver/d2codingfont)
- 태양 수성 목성, 금성 지구 화성 목성 토성 천왕성 해왕성 + 명왕성
	- 행성 크기는 원 사이즈를 바탕으로 임의로 설정됨
	- 공전, 자전, 자전축은 실제 데이터 사용
		- 케플러 행성운동 법칙을 이용한 행성 궤도 계산
			- https://ssd.jpl.nasa.gov/?planet_pos
			- https://ssd.jpl.nasa.gov/txt/p_elem_t1.txt
			- https://ssd.jpl.nasa.gov/txt/aprx_pos_planets.pdf
	- 고화질 텍스쳐 사용
		- [Solar Textures - Solcar System Scope](https://www.solarsystemscope.com/textures/)
			- 태양, 지구, 달 : 8K
			- 외 : 2K
		- [Pluto Global Color Map - Nasa](https://www.nasa.gov/image-feature/pluto-global-color-map)
			- 1K
	- 토성 : 토성 고리
	- 지구
		- 야간 조명 사용 : [스크린샷](/ScreenShot - 3.png) 참조
			- 계산 공식은 [`planet.frag`](solar/shaders/planet.frag) 파일 참조
		- 사용된 텍스쳐
			- 주간 : `earth.png`
			- 야간 : `earth_nightmap.png`
			- 구름 : `earth_clouds.png`
			- 퐁 쉐이딩 : `earth_specular_map.png`
	- `1` ~ `9` 를 사용하여 행성 바라보기
	- 행성 궤도 옵션
		- 옵션 1 : 고유 색 사용 or 흑백 사용
		- 옵션 2 : 실선, 점선
		- 옵션 3 : 그라데이션 있는 궤도 표시, 전체 궤도 표시
- [imgui](https://github.com/ocornut/imgui) 를 사용한 옵션 창
	- FPS on/off
	- 날자 on/off
	- 도움말 on/off
	- 애니메이션 속도 on/off
	- 애니메이션 재생 속도 설정
	- 배경 on/off
	- 행성 이름 on/off
	- 행성 궤도 on/off 및 스타일 설정
	- 카메라 위치 설정
	- `VSync`, `MSAA MultiSampling` on/off
- 등등...

## lib

- OpenGL modern
- imgui
- glad
- glm
- freetype
- glfw-3.3.2

