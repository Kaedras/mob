#include "pch.h"
#include "string.h"
#include "utility.h"

namespace mob {

    std::wstring utf8_to_utf16(std::string_view s)
    {
        return std::wstring(s.begin(), s.end());
    }

    std::string utf16_to_utf8(std::wstring_view ws)
    {
        return std::string(ws.begin(), ws.end());
    }

    std::wstring cp_to_utf16(uint from, std::string_view s)
    {
        return std::wstring(s.begin(), s.end());
    }

    std::string utf16_to_cp(uint to, std::wstring_view ws)
    {
        return std::string(ws.begin(), ws.end());
    }

    std::string bytes_to_utf8(encodings e, std::string_view s)
    {
        return {s.begin(), s.end()};
    }

    std::string utf16_to_bytes(encodings e, std::wstring_view ws)
    {
        return std::string(ws.begin(), ws.end());
    }

    std::string utf8_to_bytes(encodings e, std::string_view utf8)
    {
        return std::string(utf8);
    }

    std::string path_to_utf8(fs::path p)
    {
        return p.string();
    }

    encoded_buffer::encoded_buffer(encodings e, std::string bytes)
        : e_(e), bytes_(std::move(bytes)), last_(0)
    {
    }

    void encoded_buffer::add(std::string_view bytes)
    {
        bytes_.append(bytes.begin(), bytes.end());
    }

    std::string encoded_buffer::utf8_string() const
    {
        return bytes_to_utf8(e_, bytes_);
    }

}  // namespace mob
