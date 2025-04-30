#include "../pch.h"
#include "../paths.h"
#include "../../tasks/task_manager.h"
#include "../../tasks/tasks.h"
#include "../../utility/string.h"

namespace mob {
    // searches PATH for the given executable, returns empty if not found
    //
    fs::path find_in_path(std::string_view exe)
    {
        fs::path path;
        const char* item;
        auto paths = getenv("PATH");
        while ((item = strsep(&paths, ":")) != nullptr) {
            std::string fullpath = std::format("{}/{}", item, exe);
            if (fs::exists(fullpath)) {
                return fullpath;
            }
        }
        return "";
    }

    fs::path mob_exe_path()
    {
        try {
            return fs::read_symlink("/proc/self/exe");
        }
        catch (...) {
            gcx().bail_out(context::conf, "can't get module filename");
        }
    }
}  // namespace mob
