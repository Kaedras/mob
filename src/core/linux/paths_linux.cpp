#include "pch.h"
#include "../../tasks/task_manager.h"
#include "../../tasks/tasks.h"
#include "../../utility/string.h"
#include "../paths.h"

namespace mob {

    extern bool try_parts(fs::path& check, const std::vector<std::string>& parts);

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

    // looks for `qmake` in the given path, tries a variety of subdirectories
    //
    bool find_qmake(fs::path& check)
    {
        // try bin
        if (try_parts(check, {"bin", "qmake"})) {
            return true;
        }

        return false;
    }

    fs::path find_qt()
    {
        // check from the ini first
        fs::path p = conf().path().qt_install();

        if (!p.empty()) {
            p = fs::absolute(p);

            // check if qmake exists in there
            if (find_qmake(p)) {
                p = fs::absolute(p.parent_path() / "..");
                return p;
            }

            // fail early, don't try to guess if the user had something in the ini
            gcx().bail_out(context::conf, "no qt install in {}", p);
        }

        // a list of possible location
        std::deque<fs::path> locations = {"/usr/lib64/qt6", "/usr/lib/qt6"};

        // look for qmake in PATH, which is in %qt%/bin
        fs::path qmake = find_in_path("qmake");
        if (!qmake.empty())
            locations.push_front(qmake.parent_path() / "../");

        // check each location
        for (fs::path loc : locations) {
            loc = fs::absolute(loc);

            // check for qmake in there
            if (find_qmake(loc)) {
                loc = fs::absolute(loc.parent_path() / "..");
                return loc;
            }
        }

        gcx().bail_out(context::conf, "can't find qt install");
    }
}  // namespace mob
