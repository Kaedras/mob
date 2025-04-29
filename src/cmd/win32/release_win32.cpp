#include "../pch.h"
#include "../../core/conf.h"
#include "../../core/context.h"
#include "../../core/ini.h"
#include "../../core/op.h"
#include "../../tasks/task_manager.h"
#include "../../tasks/tasks.h"
#include "../../utility.h"
#include "../../utility/threading.h"
#include "../commands.h"

namespace mob {

    std::string release_command::version_from_exe() const
    {
        const auto exe = conf().path().install_bin() / "ModOrganizer.exe";

        // getting version info size
        DWORD dummy      = 0;
        const DWORD size = GetFileVersionInfoSizeW(exe.native().c_str(), &dummy);

        if (size == 0) {
            const auto e = GetLastError();
            gcx().bail_out(context::generic,
                           "can't get file version info size from {}, {}", exe,
                           error_message(e));
        }

        // getting version info
        auto buffer = std::make_unique<std::byte[]>(size);

        if (!GetFileVersionInfoW(exe.native().c_str(), 0, size, buffer.get())) {
            const auto e = GetLastError();
            gcx().bail_out(context::generic, "can't get file version info from {}, {}",
                           exe, error_message(e));
        }

        struct LANGANDCODEPAGE {
            WORD wLanguage;
            WORD wCodePage;
        };

        void* value_pointer     = nullptr;
        unsigned int value_size = 0;

        // getting list of available languages
        auto ret = VerQueryValueW(buffer.get(), L"\\VarFileInfo\\Translation",
                                  &value_pointer, &value_size);

        if (!ret || !value_pointer || value_size == 0) {
            const auto e = GetLastError();
            gcx().bail_out(context::generic,
                           "VerQueryValueW() for translations failed on {}, {}", exe,
                           error_message(e));
        }

        // number of languages
        const auto count = value_size / sizeof(LANGANDCODEPAGE);
        if (count == 0)
            gcx().bail_out(context::generic, "no languages found in {}", exe);

        // using the first language in the list to get FileVersion
        const auto* lcp = static_cast<LANGANDCODEPAGE*>(value_pointer);

        const auto sub_block =
            std::format(L"\\StringFileInfo\\{:04x}{:04x}\\FileVersion", lcp->wLanguage,
                        lcp->wCodePage);

        ret = VerQueryValueW(buffer.get(), sub_block.c_str(), &value_pointer,
                             &value_size);

        if (!ret || !value_pointer || value_size == 0) {
            gcx().bail_out(context::generic, "language {} not found in {}", sub_block,
                           exe);
        }

        // value_size includes the null terminator
        return utf16_to_utf8(
            std::wstring(static_cast<wchar_t*>(value_pointer), value_size - 1));
    }

}  // namespace mob