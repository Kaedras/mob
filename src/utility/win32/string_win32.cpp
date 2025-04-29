#include "../pch.h"
#include "../string.h"
#include "../../utility.h"

namespace mob {

    std::optional<std::wstring> to_widechar(UINT from, std::string_view s)
    {
        std::wstring ws;

        if (s.empty())
            return ws;

        ws.resize(s.size() + 1);

        for (int t = 0; t < 3; ++t) {
            const int written =
                MultiByteToWideChar(from, 0, s.data(), static_cast<int>(s.size()),
                                    ws.data(), static_cast<int>(ws.size()));

            if (written <= 0) {
                const auto e = GetLastError();

                if (e == ERROR_INSUFFICIENT_BUFFER) {
                    ws.resize(ws.size() * 2);
                    continue;
                }
                else {
                    return {};
                }
            }
            else {
                MOB_ASSERT(static_cast<std::size_t>(written) <= s.size());
                ws.resize(static_cast<std::size_t>(written));
                break;
            }
        }

        return ws;
    }

    std::optional<std::string> to_multibyte(UINT to, std::wstring_view ws)
    {
        std::string s;

        if (ws.empty())
            return s;

        s.resize(static_cast<std::size_t>(static_cast<double>(ws.size()) * 1.5));

        for (int t = 0; t < 3; ++t) {
            const int written = WideCharToMultiByte(
                to, 0, ws.data(), static_cast<int>(ws.size()), s.data(),
                static_cast<int>(s.size()), nullptr, nullptr);

            if (written <= 0) {
                const auto e = GetLastError();

                if (e == ERROR_INSUFFICIENT_BUFFER) {
                    s.resize(ws.size() * 2);
                    continue;
                }
                else {
                    return {};
                }
            }
            else {
                MOB_ASSERT(static_cast<std::size_t>(written) <= s.size());
                s.resize(static_cast<std::size_t>(written));
                break;
            }
        }

        return s;
    }

    std::wstring utf8_to_utf16(std::string_view s)
    {
        auto ws = to_widechar(CP_UTF8, s);
        if (!ws) {
            std::wcerr << L"can't convert from utf8 to utf16\n";
            return L"???";
        }

        return std::move(*ws);
    }

    std::string utf16_to_utf8(std::wstring_view ws)
    {
        auto s = to_multibyte(CP_UTF8, ws);
        if (!s) {
            std::wcerr << L"can't convert from utf16 to utf8\n";
            return "???";
        }

        return std::move(*s);
    }

    std::wstring cp_to_utf16(UINT from, std::string_view s)
    {
        auto ws = to_widechar(from, s);
        if (!ws) {
            std::wcerr << L"can't convert from cp " << from << L" to utf16\n";
            return L"???";
        }

        return std::move(*ws);
    }

    std::string utf16_to_cp(UINT to, std::wstring_view ws)
    {
        auto s = to_multibyte(to, ws);

        if (!s) {
            std::wcerr << L"can't convert from cp " << to << L" to utf16\n";
            return "???";
        }

        return std::move(*s);
    }

    std::string bytes_to_utf8(encodings e, std::string_view s)
    {
        switch (e) {
        case encodings::utf16: {
            const auto* ws   = reinterpret_cast<const wchar_t*>(s.data());
            const auto chars = s.size() / sizeof(wchar_t);
            return utf16_to_utf8({ws, chars});
        }

        case encodings::acp: {
            const std::wstring utf16 = cp_to_utf16(CP_ACP, s);
            return utf16_to_utf8(utf16);
        }

        case encodings::oem: {
            const std::wstring utf16 = cp_to_utf16(CP_OEMCP, s);
            return utf16_to_utf8(utf16);
        }

        case encodings::utf8:
        case encodings::dont_know:
        default: {
            return {s.begin(), s.end()};
        }
        }
    }

    std::string utf16_to_bytes(encodings e, std::wstring_view ws)
    {
        switch (e) {
        case encodings::utf16: {
            return std::string(reinterpret_cast<const char*>(ws.data()),
                               ws.size() * sizeof(wchar_t));
        }

        case encodings::acp: {
            return utf16_to_cp(CP_ACP, ws);
        }

        case encodings::oem: {
            return utf16_to_cp(CP_OEMCP, ws);
        }

        case encodings::utf8:
        case encodings::dont_know:
        default: {
            return utf16_to_utf8(ws);
        }
        }
    }

    std::string utf8_to_bytes(encodings e, std::string_view utf8)
    {
        switch (e) {
        case encodings::utf16:
        case encodings::acp:
        case encodings::oem: {
            const std::wstring ws = utf8_to_utf16(utf8);
            return utf16_to_bytes(e, ws);
        }

        case encodings::utf8:
        case encodings::dont_know:
        default: {
            return std::string(utf8);
        }
        }
    }

    std::string path_to_utf8(fs::path p)
    {
        return utf16_to_utf8(p.native());
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


}