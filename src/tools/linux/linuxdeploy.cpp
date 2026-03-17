#include "../../core/process.h"
#include "../tools.h"

using namespace std;

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

    linuxdeploy& linuxdeploy::iconFileName(const std::string& p)
    {
        iconFileName_ = p;
        return *this;
    }

    linuxdeploy& linuxdeploy::customAppRun(const fs::path& p)
    {
        customAppRun_ = p;
        return *this;
    }

    linuxdeploy&
    linuxdeploy::excludeLibraries(const std::vector<std::string>& excludeLibraries)
    {
        excludeLibraries_ = excludeLibraries;
        return *this;
    }

    linuxdeploy& linuxdeploy::excludeLibraries(const std::string& excludeLibraries)
    {
        excludeLibraries_.emplace_back(excludeLibraries);
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

    linuxdeploy& linuxdeploy::desktopFile(const fs::path& p)
    {
        desktopFile_ = p;
        return *this;
    }

    linuxdeploy& linuxdeploy::createDesktopFile()
    {
        createDesktopFile_ = true;
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
        string ldLibraryPath =
            format("{}/usr/bin/:{}/usr/bin/lib/:{}/usr/lib:{}/usr/lib64", appdir_,
                   appdir_, appdir_, appdir_);
        e.set("LD_LIBRARY_PATH", ldLibraryPath, env::prepend, ":");

        if (!excludeLibraries_.empty()) {
            string excludeString;
            for (int i = 0; i < excludeLibraries_.size() - 1; ++i) {
                excludeString += excludeLibraries_[i] + ";";
            }
            excludeString += excludeLibraries_.back();

            e.set("LINUXDEPLOY_EXCLUDED_LIBRARIES", excludeString, env::append);
        }

        // ensure wayland support
        e.set("EXTRA_QT_PLUGINS", "waylandcompositor");
        e.set("EXTRA_PLATFORM_PLUGINS", "libqwayland.so");

        process p;
        p.binary(binary())
            .cwd(output_)
            .env(e)
            // .arg("--verbosity", "0", process::log_debug)
            .arg("--verbosity", "3", process::log_quiet)
            .arg("--appdir", appdir_)
            .arg("--plugin", "qt")
            .arg("--output", "appimage");

        if (!customAppRun_.empty()) {
            p.arg("--custom-apprun", customAppRun_.string());
        }
        if (!executable_.empty()) {
            p.arg("--executable", executable_);
        }
        if (!icon_.empty()) {
            p.arg("--icon-file", icon_);
        }
        if (createDesktopFile_) {
            p.arg("--create-desktop-file");
        }
        else if (!iconFileName_.empty()) {
            p.arg("--desktop-file", desktopFile_);
        }

        execute_and_join(p);
    }

}  // namespace mob
