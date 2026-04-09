#include "../../core/paths.h"
#include "../../core/process.h"
#include "../tasks.h"

using namespace std;

namespace mob::tasks {

    namespace {

        url source_url()
        {
            return "https://github.com/linuxdeploy/linuxdeploy/releases/download/" +
                   conf().version().get("linuxdeploy") + "/linuxdeploy-x86_64.AppImage";
        }

        url plugin_qt_url()
        {
            return "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/"
                   "download/" +
                   conf().version().get("linuxdeploy-plugin-qt") +
                   "/linuxdeploy-plugin-qt-x86_64.AppImage";
        }

        linuxdeploy create_tool(const fs::path& appDirPath,
                                const vector<fs::path>& deployDepsOnly)
        {
            linuxdeploy tool;
            tool.output(conf().path().install_appimage())
                .appdir(appDirPath)
                .excludeLibraries("libqsqlmimer")
                .customAppRun(find_root() / "AppRun");

            for (const auto& dep : deployDepsOnly) {
                tool.deployDepsOnly(dep);
            }

            return tool;
        }

    }  // namespace

    appimage::appimage() : basic_task("appimage") {}

    std::string appimage::version()
    {
        return {};
    }

    bool appimage::prebuilt()
    {
        return false;
    }

    fs::path appimage::source_path()
    {
        return modorganizer::super_path() / "AppImage";
    }

    void appimage::do_clean(clean c)
    {
        switch (c) {
        case clean::redownload:
            op::delete_file(cx(),
                            conf().path().prefix() / "linuxdeploy-x86_64.AppImage",
                            op::optional);
            break;
        default:
            op::delete_directory(cx(), conf().path().install_appimage(), op::optional);
            op::delete_directory(cx(), conf().path().build() / "AppImage",
                                 op::optional);
            break;
        }
    }

    void appimage::do_fetch()
    {
        auto fetch = [&](const url& url) {
            const auto file = run_tool(downloader(url));

            // chmod u+x
            struct stat st{};
            if (stat(file.c_str(), &st) == -1) {
                const int e = errno;
                cx().bail_out(context::reason::cmd, "stat() failed: {}", strerror(e));
            }
            if (chmod(file.c_str(), st.st_mode | S_IXUSR) == -1) {
                const int e = errno;
                cx().bail_out(context::reason::cmd, "chmod() failed: {}", strerror(e));
            }

            // copy to mob prefix dir
            op::copy_file_to_dir_if_better(cx(), file, conf().path().prefix());
        };

        // fetch linuxdeploy
        fetch(source_url());

        // fetch linuxdeploy-plugin-qt
        fetch(plugin_qt_url());
    }

    void appimage::do_build_and_install()
    {
        const fs::path appDir = source_path() / "ModOrganizer";

        // copy bin
        op::copy_glob_to_dir_if_better(cx(), conf().path().install_bin(),
                                       appDir / "usr",
                                       op::flags::copy_files | op::flags::copy_dirs);
        // copy lib
        op::copy_glob_to_dir_if_better(cx(), conf().path().install_libs() / "*.so",
                                       appDir / "usr/lib",
                                       op::flags::copy_files | op::flags::copy_dirs);

        // copy libraries that are not inside the lib directory
        op::copy_file_to_dir_if_better(cx(), appDir / "usr/bin/libuibase.so",
                                       appDir / "usr/lib");
        op::copy_glob_to_dir_if_better(cx(), appDir / "usr/bin/lib/*.so",
                                       appDir / "usr/lib", op::flags::copy_files);

        // copy translations
        op::copy_glob_to_dir_if_better(cx(), appDir / "usr/bin/translations/*",
                                       appDir / "usr/translations",
                                       op::flags::copy_files);

        // remove unneeded files
        op::delete_file(cx(), appDir / "usr/bin/libuibase.so");
        op::delete_directory(cx(), appDir / "usr/bin/lib");
        op::delete_directory(cx(), appDir / "usr/bin/translations");

        // remove plugins from usr/lib
        const fs::path libDir = appDir / "usr/lib";
        const array libs{libDir / "libbsa_*.so",          libDir / "libcheck_fnis.so",
                         libDir / "libdiagnose_basic.so", libDir / "libgame_*.so",
                         libDir / "libinibakery.so",      libDir / "libinieditor.so",
                         libDir / "libinstaller_*.so",    libDir / "libplugin_*.so",
                         libDir / "libpreview_*.so"};
        for (const auto& lib : libs) {
            op::delete_file_glob(cx(), lib);
        }

        // strip debug symbols from files that are not automatically stripped by
        // linuxdeploy
        auto strip = [&](const string& path) {
            const string command = "strip -d " + path;
            auto p               = process::raw(cx(), command);
            if (p.run_and_join() != 0) {
                cx().bail_out(context::reason::cmd, "error stripping debug symbols");
            }
        };

        // strip files
        const fs::path bin = appDir / "usr/bin";
        const array filesToStrip{bin / "ModOrganizer",       bin / "helper",
                                 bin / "nxmhandler",         bin / "loot/*",
                                 bin / "plugins/*.so",       libDir / "lib7zip.so",
                                 libDir / "libarchive.so",   libDir / "libuibase.so",
                                 libDir / "libusvfs-fuse.so"};
        for (const auto& file : filesToStrip) {
            strip(file.string());
        }

        // copy metainfo
        // const fs::path metaInfoPath = conf().path().build() /
        //                               "modorganizer/src/resources/linux/"
        //                               "ModOrganizer.metainfo.xml";
        // op::copy_file_to_file_if_better(cx(), metaInfoPath,
        //                                appDir /
        //                                "usr/share/metainfo/ModOrganizer.appdata.xml");

        // copy icon
        op::copy_file_to_dir_if_better(
            cx(),
            conf().path().build() / "modorganizer/src/resources/linux/ModOrganizer.svg",
            appDir / "usr/share/icons/hicolor/scalable/apps");

        // copy desktop file
        op::copy_file_to_dir_if_better(
            cx(),
            conf().path().build() /
                "modorganizer/src/resources/linux/ModOrganizer.desktop",
            appDir / "usr/share/applications");

        vector deployDepsOnly{
            bin,
            bin / "loot",
            bin / "plugins",
            libDir / "lib7zip.so",
            libDir / "libarchive.so",
            libDir / "libuibase.so",
            libDir / "libusvfs-fuse.so",
        };

        // todo: refactor this once plugin_python is stable
        if (conf().task({"plugin_python"}).get<bool>("enabled")) {
            auto pluginPythonPath = bin / "plugins/plugin_python/libplugin_python.so";
            deployDepsOnly.push_back(pluginPythonPath);

            strip(pluginPythonPath.string());
            strip(bin / "plugins/plugin_python/lib/*.so");
        }

        run_tool(create_tool(appDir, deployDepsOnly));
    }

}  // namespace mob::tasks
