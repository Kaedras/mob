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

        linuxdeploy create_tool(const fs::path& appDirPath)
        {
            return std::move(linuxdeploy()
                                 .output(conf().path().install_appimage())
                                 .nostrip()
                                 .appdir(appDirPath)
                                 .customAppRun(find_root() / "AppRun"));
        }

    }  // namespace

    appimage::appimage() : basic_task("appimage") {}

    std::string appimage::version()
    {
        return {};
    }

    bool appimage::prebuilt()
    {
        return true;
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
        // fetch linuxdeploy-x86_64.AppImage
        const auto file = run_tool(downloader(source_url()));

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
    }

    void appimage::do_build_and_install()
    {
        const fs::path appDir = source_path() / "ModOrganizer";

        // copy bin
        op::copy_glob_to_dir_if_better(cx(), conf().path().install_bin(),
                                       appDir / "usr",
                                       op::flags::copy_files | op::flags::copy_dirs);
        // copy lib
        op::copy_glob_to_dir_if_better(cx(), conf().path().install_libs(),
                                       appDir / "usr",
                                       op::flags::copy_files | op::flags::copy_dirs);

        // copy libraries that are not inside the lib directory
        op::copy_file_to_dir_if_better(cx(), appDir / "usr/bin/libuibase.so",
                                       appDir / "usr/lib");
        op::copy_file_to_dir_if_better(cx(), appDir / "usr/bin/loot/libloot.so.0",
                                       appDir / "usr/lib");
        op::copy_glob_to_dir_if_better(cx(), appDir / "usr/bin/lib/*.so",
                                       appDir / "usr/lib", op::flags::copy_files);

        // remove unneeded files
        op::delete_directory(cx(), appDir / "usr/lib/cmake");
        op::delete_file(cx(), appDir / "usr/bin/lib/libbsarchpp.so");
        op::delete_file(cx(), appDir / "usr/bin/loot/libloot.so.0");
        op::delete_file_glob(cx(), appDir / "usr/lib/*.a");
        op::delete_file(cx(), appDir / "usr/bin/libuibase.so");
        op::delete_directory(cx(), appDir / "usr/bin/lib");

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

        const fs::path desktopFilePath =
            appDir / "usr/share/applications/ModOrganizer.desktop";

        // copy desktop file
        op::copy_file_to_dir_if_better(
            cx(),
            conf().path().build() /
                "modorganizer/src/resources/linux/ModOrganizer.desktop",
            appDir / "usr/share/applications");

        run_tool(create_tool(appDir));
    }

}  // namespace mob::tasks
