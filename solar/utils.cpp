#include "utils.h"

#include <random>

#include <Windows.h>

#include "constants.h"

namespace utils
{
    std::wstring str2wcs(const std::string& str)
    {
        // 필요 길이 측정
        const auto wcsLength = 1 + MultiByteToWideChar(CP_ACP, 0, str.c_str(), static_cast<int>(str.size()), NULL, 0);

        // 문자열 생성
        std::wstring wcs(wcsLength, 0);

        // 변환 작업
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), static_cast<int>(str.size()), wcs.data(), static_cast<int>(wcsLength));

        return wcs;
    }

    std::string wcs2str(const std::wstring& wcs)
    {
        // 필요 길이 측정
        const auto strLength = 1 + WideCharToMultiByte(CP_ACP, 0, wcs.c_str(), static_cast<int>(wcs.size()), NULL, 0, NULL, NULL);

        // 문자열 생성
        std::string str(strLength, 0);

        // 변환 작업
        WideCharToMultiByte(CP_ACP, 0, wcs.c_str(), static_cast<int>(wcs.size()), str.data(), static_cast<int>(strLength), NULL, NULL);

        return str;
    }

    double rand()
    {
        static std::random_device rn;
        static std::mt19937_64 rnd(rn());

        const std::uniform_real_distribution<double> range(0, 1);

        return range(rnd);
    }
}
