// 유틸 함수

#pragma once

#include <string>

#include "constants.h"

namespace utils
{
    std::wstring str2wcs(const std::string&  str); // char -> wchar 변환 함수.
    std::string  wcs2str(const std::wstring& wcs); // wchar -> char 변환 함수.

    double rand();
}
