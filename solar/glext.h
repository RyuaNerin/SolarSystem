// openGL 유틸들

#pragma once

#include <functional>

#include <glad/glad.h>

#include "defer.h"
#include "v.h"

namespace glext
{
    // dispatch 로 비동기적으로 해결해야 하는 함수를 처리한다.
    void dispatchInvoke();

    /*
    OpenGL 특성 상... 멀티스레드가 지원이 되지 않아 모든 처리를 메인스레드에서 진행해야 한다.
    하지만 메인 스레드에서 큰 텍스쳐를 불러올 경우 프리징 현상이 굉장히 심하다는 단점이 존재하였다.
    이것을 해결하기 위해 현재 OpenGL Context 를 공유하는 또다른 Context 를 생성하여 백그라운드에서 리소스를 로드하게 하였다.
    다만, 기존의 OpenGL Legacy 한 코드에서는 wglShareList 를 사용하여 쉽게 구현하였으나,
    OpenGL Modern 으로 재작업 하면서 단순히 wglShareList 를 사용하는 것 만으로는 해결되지 않는 부분들이 존재하였음.

    따라서
    1. 필요한 데이터를 백그라운드에서 읽도록 한 후에
    2. 데이터를 읽고 처리하였는지 확인하고
    3. 읽은 데이터를 적절하게 OpenGL 에서 처리하는 작업을 거쳐야 하는게 가장 이상적인 구조로 판단되지만,
    4. 해당 구조를 구현하기에는 시간도 오래 걸릴 뿐더러 기존 코드르 대폭 개선해야 할 것 같아
    5. C# 의 Invoke 및 Dispatcher 에서 영감을 받아
    5. openGL 메인 스레드에서 실행해야 하는 부분들을 특정 큐에 집어넣어
    6. 이를 메인 스레드에서 실행하게 하여 이러한 문제를 해결해보고자 하였음.
    */
    void dispatch(std::function<void()> const& func);

    GLuint loadTexture(const std::string& textureName); // 텍스쳐 가져오는 함수
    GLuint loadShader (const std::string& shaderName); // 셰이더 컴파일해서 가져오는 함수
}

