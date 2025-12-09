#include "../../core/process.h"
#include "../tools.h"

namespace mob {

    linuxdeploy::linuxdeploy() : basic_process_runner("linuxdeploy") {}

    fs::path linuxdeploy::binary()
    {
        return conf().path().prefix() / conf().tool().get("linuxdeploy");
    }

    linuxdeploy& linuxdeploy::output(const fs::path& p)
    {
        output_ = p;
        return *this;
    }

    linuxdeploy& linuxdeploy::appdir(const fs::path& p)
    {
        appdir_ = p;
        return *this;
    }

    linuxdeploy& linuxdeploy::executable(const fs::path& p)
    {
        executable_ = p;
        return *this;
    }

    linuxdeploy& linuxdeploy::icon(const fs::path& p)
    {
        icon_ = p;
        return *this;
    }

    linuxdeploy& linuxdeploy::nostrip()
    {
        nostrip_ = true;
        return *this;
    }

    void linuxdeploy::do_run()
    {
        env e = this_env::get();

        if (nostrip_) {
            e.set("NO_STRIP", "1");
        }
        std::string ldLibraryPath =
            std::format("{}/usr/bin/:{}/usr/bin/lib/:{}/usr/lib:{}/usr/lib64", appdir_,
                        appdir_, appdir_, appdir_);
        e.set("LD_LIBRARY_PATH", ldLibraryPath, env::append);

        execute_and_join(process()
                             .binary(binary())
                             .cwd(output_)
                             .env(e)
                             // .arg("--verbosity", "0", process::log_debug)
                             .arg("--verbosity", "3", process::log_quiet)
                             .arg("--appdir", appdir_)
                             .arg("--executable", executable_)
                             .arg("--icon-file", icon_)
                             .arg("--icon-filename", "ModOrganizer")
                             .arg("--create-desktop-file")
                             .arg("--output", "appimage"));
    }

}  // namespace mob
