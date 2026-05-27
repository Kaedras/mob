#include "assert.h"
#include "../core/context.h"

#ifdef __cpp_lib_debugging
#include <debugging>
#endif

namespace mob {

    void mob_assertion_failed(const char* message, const char* exp, const wchar_t* file,
                              int line, const char* func)
    {
        if (message) {
            gcx().error(context::generic, "assertion failed: {}:{} {}: {} ({})",
                        std::wstring(file), line, func, message, exp);
        }
        else {
            gcx().error(context::generic, "assertion failed: {}:{} {}: '{}'",
                        std::wstring(file), line, func, exp);
        }

#ifdef __cpp_lib_debugging
        if (std::is_debugger_present())
            std::breakpoint();
#else
        if (IsDebuggerPresent())
            DebugBreak();
#endif
        else
            std::exit(1);
    }

}  // namespace mob
