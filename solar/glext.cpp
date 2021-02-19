#include "glext.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <ostream>
#include <queue>
#include <sstream>
#include <vector>

#include <Windows.h>
#include <ole2.h>
#include <gdiplus.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "defer.h"
#include "utils.h"

namespace glext
{
    /*
    *       | <----------------------------------|
    * MAIN THREAD ---------------------------> RENDERING
    *                       |
    *                       |
    *                     LOADER
    *
    */
    std::mutex taskLock;
    std::queue<std::packaged_task<void()>> taskQueue;

    void dispatchInvoke()
    {
        std::lock_guard __lock(taskLock);

        while (taskQueue.size() > 0)
        {
            taskQueue.front()();
            taskQueue.pop();
        }
    }

    void dispatch(std::function<void()> const& func)
    {
        std::packaged_task<void()> task(func);

        auto fut = task.get_future();

        {
            // f
            std::lock_guard __lock(taskLock);
            taskQueue.push(std::packaged_task<void()>(std::move(task)));
        }

        // 대기하고
        fut.wait();
    }

    unsigned int loadTexture(const std::string& textureName)
    {
        std::cout << "loadTexture textureName: " << textureName << std::endl;

        auto path = getTexturePath(textureName);
        const auto wpath = utils::str2wcs(path); // wstring 으로 변경

        /**************************************************/
        // 작성자의 편의에 따라 glaux 대신 윈도우 내장 라이브러리 사용.
        // WINAPI GDI+ 가 BMP PNG GIF JPG 등 다양한 타입을 지원하기 때문에 이를 사용하여 이미지를 불러온다.

        // GDI+ 초기화
        ULONG_PTR gdiplusToken;
        const Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        auto res = Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
        if (res != Gdiplus::Ok)
        {
            // 오류 메시지 생성
            std::stringstream ss;
            ss << "Failed to initalize GDI+. return: " << res;

            // 오류 발생
            throw std::invalid_argument(ss.str());
        }
        defer(Gdiplus::GdiplusShutdown(gdiplusToken)); // gdiplusToken 해제

        // 이미지 불러오고
        const auto bitmap = Gdiplus::Bitmap::FromFile(wpath.c_str());
        if (bitmap == nullptr)
        {
            throw std::runtime_error("Failed to load bitmap.");
        }
        defer(delete bitmap); // bitmap 객체 해제

        res = bitmap->GetLastStatus();
        if (res != Gdiplus::Status::Ok)
        {
            // 오류 메시지 생성
            std::stringstream ss;
            ss << "Failed to load bitmap. return: " << res << " path: " << path;

            // 오류 발생
            throw std::invalid_argument(ss.str());
        }

        // opengl 의 좌표는 밑에서부터 시작하므로 이미지를 역전시켜준다
        //bitmap->RotateFlip(Gdiplus::RotateFlipType::Rotate180FlipX);

        // GDI+ 의 LockBits 를 사용해서 이미지 데이터 획득
        const Gdiplus::Rect rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
        Gdiplus::BitmapData bitmapData;
        res = bitmap->LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeRead, PixelFormat32bppARGB, &bitmapData);
        if (res != Gdiplus::Ok)
        {
            // 오류 메시지 생성
            std::stringstream ss;
            ss << "Failed to lock bits. return: " << res;

            // 오류 발생
            throw std::invalid_argument(ss.str());
        }
        defer(bitmap->UnlockBits(const_cast<Gdiplus::BitmapData*>(&bitmapData))); // LockBits 후 객체 해제하는 과정

        // 텍스쳐 생성하기.
        unsigned int textureId;
        dispatch([&]() {
            glGenTextures(1, &textureId);
            
            glBindTexture(GL_TEXTURE_2D, textureId); // 텍스쳐 바인딩
            defer(glBindTexture(GL_TEXTURE_2D, 0));

            // 텍스쳐 설정
            glTexImage2D(
                GL_TEXTURE_2D,
                0, // 자동
                GL_RGBA,
                bitmap->GetWidth(),
                bitmap->GetHeight(),
                0, // 여백
                GL_BGRA,
                GL_UNSIGNED_BYTE,
                bitmapData.Scan0
            );

            // Mipmap 생성
            glGenerateMipmap(GL_TEXTURE_2D);
        });

        return textureId;
    }

    const std::string readAll(const std::string& shaderName)
    {
        std::ifstream fs;

        // 오류 발생 시 에러 발생시키기
        fs.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        // 파일 열고
        fs.open(getShaderPath(shaderName));

        std::string str;

        // 파일 크기 측정하고 벡터 초기화
        fs.seekg(0, std::ios::end);
        str.reserve(static_cast<size_t>(fs.tellg()));
        fs.seekg(0, std::ios::beg);

        // 파일 읽기
        str.assign((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());

        return str;
    };

    const int compile(GLenum shaderType, const std::string& shaderData)
    {
        const auto shaderDataPtr = shaderData.data();
        const auto shaderDataSize = static_cast<int>(shaderData.size());

        GLuint shader = 0;
            shader = glCreateShader(shaderType); // 셰이더 생성
            glShaderSource(shader, 1, &shaderDataPtr, &shaderDataSize); // 셰이더 데이터 설정
            glCompileShader(shader); // 셰이더 컴파일

            // 컴파일 결과를 가져오고
            GLint success = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

            // 오류 확인한다
            if (success == GL_FALSE)
            {
                // 오류 문자열 길이를 얻고
                int logSize = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

                // 할당한 다음
                std::string log;
                log.resize(logSize);

                // 오류 문자열 가져오고
                glGetShaderInfoLog(shader, logSize, NULL, log.data());

                // 오류 발생
                throw std::runtime_error(log);
            }
        return shader;
    }

    unsigned int glext::loadShader(const std::string& shaderName)
    {
        std::cout << "shader load. shaderName: " << shaderName << std::endl;

        const auto shader_vertex   = compile(GL_VERTEX_SHADER,   readAll(shaderName + ".vert"));
        const auto shader_fragment = compile(GL_FRAGMENT_SHADER, readAll(shaderName + ".frag"));
        
        GLuint id;

            // 이 블록 끝에서 생성된 셰이더 삭제
            defer(glDeleteShader(shader_vertex  ));
            defer(glDeleteShader(shader_fragment));

            // 프로그램 생성
            id = glCreateProgram();

            // 셰이더 연결
            glAttachShader(id, shader_vertex);
            glAttachShader(id, shader_fragment);

            // 이 블록 끝에서 셰이더 연결 해제
            defer(glDetachShader(id, shader_vertex));
            defer(glDetachShader(id, shader_fragment));

            // 링크
            glLinkProgram(id);

            // 오류 확인
            GLint success;
            glGetProgramiv(id, GL_LINK_STATUS, &success);
            if (!success)
            {
                // 오류 문자열 길이를 얻고
                int logSize = 0;
                glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logSize);

                // 할당한 다음
                std::string log;
                log.resize(logSize);

                // 오류 문자열 가져오고
                glGetShaderInfoLog(id, logSize, NULL, log.data());

                // 오류 발생
                throw std::runtime_error(log);
            }

        return id;
    }
}
