#include "../tools.h"
#include "core/process.h"
#include <sys/stat.h>

namespace mob {

    linuxdeploy::linuxdeploy() : basic_process_runner("linuxdeploy") {}

    fs::path linuxdeploy::binary()
    {
        return conf().path().prefix() / conf().tool().get("linuxdeploy");
    }

    void linuxdeploy::do_run()
    {
        auto appDir = conf().path().install_appimage();

        op::copy_glob_to_dir_if_better(cx(), conf().path().install(), appDir,
                                       op::flags::copy_files | op::flags::copy_dirs);

        // remove unneeded files
        op::delete_directory(cx(), appDir / "usr/include");
        op::delete_directory(cx(), appDir / "usr/lib/cmake");

        // copy icons
        // TODO: copy high res icon
        // op::copy_file_to_file_if_better(
        //     cx(), conf().path().build() / "modorganizer/src/resources/mo_icon.png",
        //     appDir / "ModOrganizer.png");

        // strip files
        std::vector filesToStrip = {
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
            execute_and_join(process().binary("/bin/sh").arg(
                "-c", "strip -d " + file.string(), process::quote));
        }

        // create the desktop file
        // std::string desktopFileContent = "[Desktop Entry]\n"
        //                                  "Name=ModOrganizer\n"
        //                                  "Exec=ModOrganizer\n"
        //                                  "Icon=ModOrganizer\n"
        //                                  "Type=Application\n"
        //                                  "Categories=Utility;\n";
        // op::write_text_file(cx(), encodings::dont_know, appDir /
        // "ModOrganizer.desktop",
        //                     desktopFileContent);

        execute_and_join(
            process()
                .binary(binary())
                .cwd(conf().path().prefix())
                .env(env::vs_x64()
                         .set("NO_STRIP", "1")
                         .set("LD_LIBRARY_PATH",
                              std::format("{}/usr/bin/:{}/usr/bin/lib/:{}/usr/lib64",
                                          appDir, appDir, appDir)))
                .arg("--verbosity", "0", process::log_debug)
                .arg("--verbosity", "3", process::log_quiet)
                .arg("--appdir", appDir)
                .arg("--executable", appDir / "usr/bin/ModOrganizer")
                .arg("--icon-file",
                     conf().path().build() / "modorganizer/src/resources/mo_icon.png")
                .arg("--icon-filename", "ModOrganizer")
                .arg("--create-desktop-file")
                .arg("--output", "appimage"));

        // chmod u+x
        std::string file = conf().path().prefix().string();
        struct stat st{};
        if (stat(file.c_str(), &st) == -1) {
            const int e = errno;
            cx().bail_out(context::reason::cmd, "stat() failed: {}", strerror(e));
        }
        if (chmod(file.c_str(), st.st_mode | S_IXUSR) == -1) {
            const int e = errno;
            cx().bail_out(context::reason::cmd, "chmod() failed: {}", strerror(e));
        }
    }

}  // namespace mob
