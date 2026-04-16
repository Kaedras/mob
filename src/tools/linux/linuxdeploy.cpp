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

    linuxdeploy& linuxdeploy::ldLibraryPath(fs::path path)
    {
        ldLibraryPath_.emplace_back(std::move(path));
        return *this;
    }

    linuxdeploy& linuxdeploy::ldLibraryPath(const std::vector<fs::path>& paths)
    {
        ldLibraryPath_.append_range(paths);
        return *this;
    }

    linuxdeploy& linuxdeploy::executable(const fs::path& p)
    {
        executables_.emplace_back(p);
        return *this;
    }

    linuxdeploy& linuxdeploy::library(const fs::path& p)
    {
        libraries_.push_back(p);
        return *this;
    }

    linuxdeploy& linuxdeploy::library(const std::vector<fs::path>& p)
    {
        libraries_.append_range(p);
        return *this;
    }

    linuxdeploy& linuxdeploy::deployDepsOnly(const fs::path& p)
    {
        deployDepsOnly_.push_back(p);
        return *this;
    }

    linuxdeploy& linuxdeploy::deployDepsOnly(const std::vector<fs::path>& p)
    {
        deployDepsOnly_.append_range(p);
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
            format("{}/usr/bin:{}/usr/bin/lib:{}/usr/lib:{}/usr/lib64", appdir_,
                   appdir_, appdir_, appdir_);

        for (const auto& path : ldLibraryPath_) {
            ldLibraryPath += ":" + path.string();
        }

        e.set("LD_LIBRARY_PATH", ldLibraryPath, env::prepend, ":");

        if (!excludeLibraries_.empty()) {
            string excludeString;
            for (size_t i = 0; i < excludeLibraries_.size() - 1; ++i) {
                excludeString += excludeLibraries_[i] + ";";
            }
            excludeString += excludeLibraries_.back();

            e.set("LINUXDEPLOY_EXCLUDED_LIBRARIES", excludeString, env::append);
        }

        // ensure wayland support
        // e.set("EXTRA_QT_PLUGINS", "waylandcompositor");
        // e.set("EXTRA_PLATFORM_PLUGINS", "libqwayland.so");

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
        for (const auto& path : executables_) {
            p.arg("--executable", path);
        }
        for (const auto& path : libraries_) {
            p.arg("--library", path);
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
        for (const auto& path : deployDepsOnly_) {
            p.arg("--deploy-deps-only", path);
        }

        execute_and_join(p);
    }

}  // namespace mob
