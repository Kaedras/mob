#include "../../core/process.h"
#include "../tasks.h"

#include <sys/stat.h>

using namespace std;

namespace mob::tasks {

    namespace {

        url source_url()
        {
            return "https://github.com/linuxdeploy/linuxdeploy/releases/download/" +
                   conf().version().get("linuxdeploy") + "/linuxdeploy-x86_64.AppImage";
        }

        linuxdeploy create_tool(const string& executable, const fs::path& appDirPath,
                                const fs::path& icon, const fs::path& desktopFile = {})
        {
            return std::move(linuxdeploy()
                                 .output(conf().path().prefix())
                                 .nostrip()
                                 .appdir(appDirPath)
                                 .executable(appDirPath / ("usr/bin/" + executable))
                                 .iconFileName(executable)
                                 .icon(icon)
                                 .desktopFile(desktopFile));
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
        return modorganizer::super_path() / "appimage";
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
            op::delete_directory(
                cx(), conf().path().install_appimage().string() + "_nxmhandler",
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
        auto appDir = conf().path().install_appimage();

        // copy bin
        op::copy_glob_to_dir_if_better(cx(), conf().path().install_bin(),
                                       appDir / "usr",
                                       op::flags::copy_files | op::flags::copy_dirs);
        // copy lib
        op::copy_glob_to_dir_if_better(cx(), conf().path().install_libs(),
                                       appDir / "usr",
                                       op::flags::copy_files | op::flags::copy_dirs);
        // copy lib64
        op::copy_glob_to_dir_if_better(cx(), conf().path().install() / "lib64",
                                       appDir / "usr",
                                       op::flags::copy_files | op::flags::copy_dirs);

        // remove unneeded files
        op::delete_directory(cx(), appDir / "usr/lib/cmake");

        // TODO: copy high res icon

        // strip files
        vector filesToStrip = {
            appDir / "usr/bin/ModOrganizer", appDir / "usr/bin/helper",
            appDir / "usr/bin/nxmhandler",   appDir / "usr/bin/libuibase.so",
            appDir / "usr/bin/loot/lootcli", appDir / "usr/bin/loot/libloot.*",
            appDir / "usr/bin/lib/*.so",     appDir / "usr/bin/plugins/*.so",
            appDir / "usr/lib/*.so",         appDir / "usr/lib/*.a",
            appDir / "usr/lib64/*.so",
        };

        for (const auto& file : filesToStrip) {
            // using `/bin/sh -c` is required to use wildcards in paths
            // TODO: check why this is, process is already run as fork -> execle /bin/sh
            // -c
            process p =
                process::raw(cx(), format("/bin/sh -c \"strip -d {}\"", file.string()));
            run_tool(process_runner(p));
        }

        // create the desktop file
        string desktopFileContent = "[Desktop Entry]\n"
                                    "Name=ModOrganizer\n"
                                    "Exec=ModOrganizer\n"
                                    "Icon=ModOrganizer\n"
                                    "Type=Application\n"
                                    "Categories=Utility;\n";
        op::write_text_file(cx(), encodings::dont_know,
                            conf().path().temp_dir() / "ModOrganizer.desktop",
                            desktopFileContent);

        run_tool(create_tool("ModOrganizer", conf().path().install_appimage(),
                             conf().path().build() /
                                 "modorganizer/src/resources/mo_icon.png",
                             conf().path().temp_dir() / "ModOrganizer.desktop"));

        // create nxmhandler.AppImage
        fs::path appDir_nxmhandler =
            conf().path().install_appimage().string() + "_nxmhandler";

        op::copy_file_to_file_if_better(cx(), appDir / "usr/bin/nxmhandler",
                                        appDir_nxmhandler / "usr/bin/nxmhandler");
        op::copy_file_to_file_if_better(cx(), appDir / "usr/bin/libuibase.so",
                                        appDir_nxmhandler / "usr/bin/libuibase.so");

        run_tool(create_tool(
            "nxmhandler", conf().path().install_appimage().string() + "_nxmhandler",
            conf().path().build() / "modorganizer/src/resources/mo_icon.png"));
    }

}  // namespace mob::tasks
