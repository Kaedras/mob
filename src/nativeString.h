#pragma once

#include <string>
#include <string_view>

#ifdef __unix__
using nativeString     = std::string;
using nativeStringView = std::string_view;
#define _wcsicmp strcasecmp
#else
using nativeString     = std::wstring;
using nativeStringView = std::wstring_view;
#endif
