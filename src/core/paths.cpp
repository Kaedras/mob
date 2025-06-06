#include "pch.h"
#include "paths.h"
#include "../tasks/task_manager.h"
#include "../tasks/tasks.h"
#include "../utility/string.h"
#include "env.h"

#ifdef __unix__
static constexpr auto BUILD_DIR = u8"src";
#else
static constexpr auto BUILD_DIR = u8"x64";
#endif

namespace mob {

    // checks if a path exists that starts with `check` and ends with as many parts
    // as possible
    //
    // for example:
    //
    //   try_parts("c:/", {"1", "2", "3"})
    //
    // will try in order:
    //
    //   c:/1/2/3
    //   c:/2/3
    //   c:/3
    //
    // if none of the paths exist, returns false; if one of the paths exists,
    // `check` is set to it and returns true
    //
    bool try_parts(fs::path& check, const std::vector<std::string>& parts)
    {
        for (std::size_t i = 0; i < parts.size(); ++i) {
            fs::path p = check;

            for (std::size_t j = i; j < parts.size(); ++j)
                p /= parts[j];

            gcx().trace(context::conf, "trying parts {}", p);

            if (fs::exists(p)) {
                check = p;
                return true;
            }
        }

        return false;
    }

    fs::path find_root(bool verbose)
    {
        gcx().trace(context::conf, "looking for root directory");

        fs::path mob_exe_dir = mob_exe_path().parent_path();

        auto third_party = mob_exe_dir / "third-party";

        if (!fs::exists(third_party)) {
            // doesn't exist, maybe this is the build directory

            auto p = mob_exe_dir;

            if (p.filename().u8string() == BUILD_DIR) {
                p = p.parent_path();

                if (p.filename().u8string() == u8"Debug" ||
                    p.filename().u8string() == u8"Release" ||
                    p.filename().u8string() == u8"debug" ||
                    p.filename().u8string() == u8"release") {
                    if (verbose)
                        u8cout << "mob.exe is in its build directory, looking up\n";

                    // mob_exe_dir is in the build directory
                    third_party = mob_exe_dir / ".." / ".." / ".." / "third-party";
                }
            }
        }

        if (!fs::exists(third_party))
            gcx().bail_out(context::conf, "root directory not found");

        const auto p = fs::canonical(third_party.parent_path());
        gcx().trace(context::conf, "found root directory at {}", p);

        return p;
    }

    fs::path find_in_root(const fs::path& file)
    {
        static fs::path root = find_root();

        fs::path p = root / file;
        if (!fs::exists(p))
            gcx().bail_out(context::conf, "{} not found", p);

        gcx().trace(context::conf, "found {}", p);
        return p;
    }

    fs::path find_third_party_directory()
    {
        static fs::path path = find_in_root("third-party");
        return path;
    }

    fs::path find_vcpkg()
    {
        const auto env_path = this_env::get().get("VCPKG_ROOT");

        if (!env_path.empty()) {
            return fs::absolute(env_path);
        }

        const auto vs_path       = conf().path().vs();
        const auto vcpkg_vs_path = vs_path / "VC" / "vcpkg";
        if (!exists(vcpkg_vs_path)) {
            gcx().bail_out(context::conf, "vcpkg is not part of VS installation at {}",
                           vs_path);
        }

        return vcpkg_vs_path;
    }

    fs::path find_temp_dir()
    {
        std::error_code ec;
        fs::path p = fs::temp_directory_path(ec);
        if (ec) {
            gcx().bail_out(context::conf, "can't get temp path",
                           error_message(ec.value()));
        }
        gcx().trace(context::conf, "temp dir is {}", p);

        return p;
    }

}  // namespace mob
