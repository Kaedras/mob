#include "../pch.h"
#include "../context.h"

namespace mob {

    // retrieves the error message from the system for the given id
    //
    std::string error_message(DWORD id)
    {
        wchar_t* message = nullptr;

        const auto ret =
            FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                               FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           reinterpret_cast<LPWSTR>(&message), 0, NULL);

        std::wstring s;

        std::wostringstream oss;

        // hex error code
        oss << L"0x" << std::hex << id;

        if (ret == 0 || !message) {
            // error message not found, just use the hex error code
            s = oss.str();
        }
        else {
            // FormatMessage() includes a newline, trim it and put the hex code too
            s = trim_copy(message) + L" (" + oss.str() + L")";
        }

        LocalFree(message);

        return utf16_to_utf8(s);
    }

}  // namespace mob