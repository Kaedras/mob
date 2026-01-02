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

        linuxdeploy create_tool(const string& executable, const fs::path& appDirPath,
                                const fs::path& icon, const fs::path& desktopFile = {})
        {
            return std::move(linuxdeploy()
                                 .output(conf().path().install_appimage())
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
        const fs::path appDirMo = source_path() / "ModOrganizer";
        const fs::path appDirNh = source_path() / "nxmhandler";

        // copy bin
        op::copy_glob_to_dir_if_better(cx(), conf().path().install_bin(),
                                       appDirMo / "usr",
                                       op::flags::copy_files | op::flags::copy_dirs);
        // copy lib
        op::copy_glob_to_dir_if_better(cx(), conf().path().install_libs(),
                                       appDirMo / "usr",
                                       op::flags::copy_files | op::flags::copy_dirs);
        // copy lib64
        op::copy_glob_to_dir_if_better(cx(), conf().path().install() / "lib64",
                                       appDirMo / "usr",
                                       op::flags::copy_files | op::flags::copy_dirs);

        // remove unneeded files
        op::delete_directory(cx(), appDirMo / "usr/lib/cmake");

        // TODO: copy high res icon

        // strip files and save debug symbols
        const vector executables = {
            appDirMo / "usr/bin/ModOrganizer", appDirMo / "usr/bin/helper",
            appDirMo / "usr/bin/loot/lootcli", appDirMo / "usr/bin/nxmhandler"};
        const vector dirs = {
            appDirMo / "usr/bin/",         appDirMo / "usr/bin/loot/",
            appDirMo / "usr/bin/plugins/", appDirMo / "usr/lib/",
            appDirMo / "usr/lib64/",
        };

        op::create_directories(cx(), conf().path().install_pdbs());

        auto copyDebugSymbols = [this](const fs::path& file, const fs::path& out) {
            process proc = process::raw(cx(), format("objcopy --only-keep-debug {} {}",
                                                     file.string(), out.string()));
            run_tool(process_runner(proc));
        };
        auto stripDebugSymbols = [this](const fs::path& file) {
            process proc = process::raw(cx(), format("strip -d {}", file.string()));
            run_tool(process_runner(proc));
        };

        for (const fs::path& dir : dirs) {
            for (const fs::directory_entry& file : fs::directory_iterator(dir)) {
                const fs::path ext = file.path().extension();
                if (ext == ".so" || ext == ".a" ||
                    file.path().filename().string().find(".so.") != string::npos) {
                    copyDebugSymbols(file.path(),
                                     conf().path().install_pdbs() /
                                         (file.path().filename().string() + ".debug"));
                    stripDebugSymbols(file.path());
                }
            }
        }

        for (const fs::path& file : executables) {
            copyDebugSymbols(file, conf().path().install_pdbs() /
                                       (file.filename().string() + ".debug"));
            stripDebugSymbols(file);
        }

        // copy metainfo
        const fs::path metaInfoPath = conf().path().build() /
                                      "modorganizer/src/resources/linux/"
                                      "org.modorganizer2.ModOrganizer.metainfo.xml";
        op::copy_file_to_dir_if_better(cx(), metaInfoPath,
                                       appDirMo / "usr/share/metainfo/");

        const fs::path moDesktopFilePath =
            conf().path().build() /
            "modorganizer/src/resources/linux/org.modorganizer2.ModOrganizer.desktop";

        run_tool(create_tool("ModOrganizer", appDirMo,
                             conf().path().build() /
                                 "modorganizer/src/resources/mo_icon.png",
                             moDesktopFilePath));

        // create nxmhandler.AppImage
        op::copy_file_to_file_if_better(cx(), appDirMo / "usr/bin/nxmhandler",
                                        appDirNh / "usr/bin/nxmhandler");
        op::copy_file_to_file_if_better(cx(), appDirMo / "usr/bin/libuibase.so",
                                        appDirNh / "usr/bin/libuibase.so");

        // create the desktop file
        fs::path nxmHandlerDesktopFilePath =
            conf().path().temp_dir() / "org.modorganizer2.nxmhandler.desktop";
        string nxmHandlerDesktopFileContent = "[Desktop Entry]\n"
                                              "Name=nxmhandler\n"
                                              "Exec=nxmhandler\n"
                                              "Icon=nxmhandler\n"
                                              "Type=Application\n"
                                              "Categories=Utility;\n";
        op::write_text_file(cx(), encodings::dont_know, nxmHandlerDesktopFilePath,
                            nxmHandlerDesktopFileContent);

        run_tool(create_tool("nxmhandler", appDirNh,
                             conf().path().build() /
                                 "modorganizer/src/resources/mo_icon.png",
                             nxmHandlerDesktopFilePath));
    }

}  // namespace mob::tasks
