#include "../../core/conf.h"
#include "../../core/context.h"
#include "../../core/op.h"
#include "../../core/process.h"
#include "../../tools/tools.h"
#include "../commands.h"

using namespace std;

namespace mob {

    void createSymbolFile(const fs::path& path)
    {
        // create a symbol file for google breakpad
        fs::path symbolFile =
            conf().path().install_pdbs() / (path.filename().string() + ".sym");
        {
            auto p = process::raw(
                gcx(), format("dump_syms {} > {}", path.string(), symbolFile));
            if (p.run_and_join() != 0) {
                gcx().bail_out(context::generic, "error creating symbol files");
            }
        }

        ifstream ifs(symbolFile);
        string module, os, arch, hash, name;
        ifs >> module >> os >> arch >> hash >> name;
        ifs.close();

        fs::path symbolPath = conf().path().install_pdbs() / "symbols" / name / hash;

        op::move_to_directory(gcx(), symbolFile, symbolPath);

        // create a debug file
        fs::path debugFile =
            conf().path().install_pdbs() / (path.filename().string() + ".debug");
        {
            auto p = process::raw(gcx(), format("objcopy --only-keep-debug {} {}",
                                                path.string(), debugFile));
            if (p.run_and_join() != 0) {
                gcx().bail_out(context::generic, "error creating debug files");
            }
        }
        op::move_to_directory(gcx(), debugFile, conf().path().install_pdbs() / "debug");
    }

    void release_command::make_bin()
    {
        const auto out = out_ / make_filename("");
        u8cout << "making binary archive " << path_to_utf8(out) << "\n";

        // the files are first copied into a temporary directory, then stripped and
        // packaged. otherwise `make_pdbs()` will not be able to create any
        // debug symbols

        // copy files to tmp dir
        char tmpName[] = "/tmp/mo2-XXXXXX";
        if (mkdtemp(tmpName) == nullptr) {
            const int e = errno;
            gcx().bail_out(context::generic, "error creating temporary directory: {}",
                           strerror(e));
        }
        const fs::path tmpDir = tmpName;
        op::copy_glob_to_dir_if_better(gcx(), conf().path().install_bin() / "*", tmpDir,
                                       op::copy_dirs | op::copy_files | op::unsafe);

        // strip files
        auto strip = [&](const fs::path& path) {
            const string command = "strip -d " + path.string();
            auto p               = process::raw(gcx(), command);
            if (p.run_and_join() != 0) {
                gcx().bail_out(context::reason::cmd, "error stripping debug symbols");
            }
        };
        const array filesToStrip{
            tmpDir / "helper",
            tmpDir / "ModOrganizer",
            tmpDir / "nxmhandler",
            tmpDir / "lib/*.so",
            tmpDir / "loot/*",
            tmpDir / "plugins/*.so",
            tmpDir / "plugins/plugin_python/*.so",
            tmpDir / "plugins/plugin_python/lib/*.so",
            tmpDir / "plugins/plugin_python/libs/*.so",
            tmpDir / "plugins/plugin_python/libs/PyQt6/*.so",
        };
        for (const auto& file : filesToStrip) {
            strip(file);
        }

        // create archive
        op::archive_from_glob(gcx(), tmpDir / "*", out, {"__pycache__"});

        // delete tmp dir
        op::delete_directory(gcx(), tmpDir, op::unsafe);
    }

    void release_command::make_pdbs()
    {
        const auto out = out_ / make_filename("debug");
        u8cout << "making debug archive " << path_to_utf8(out) << "\n";

        const fs::path base = conf().path().install_bin();

        op::delete_directory(gcx(), conf().path().install_pdbs(), op::optional);
        op::create_directories(gcx(), conf().path().install_pdbs());

        // process executables
        const array files{base / "helper", base / "ModOrganizer", base / "nxmhandler",
                          base / "loot/lootcli"};
        for (const fs::path& file : files) {
            createSymbolFile(file);
        }

        // process libraries
        for (const auto& file : fs::recursive_directory_iterator(base)) {
            if (file.is_directory() || file.path().extension().string() != ".so") {
                continue;
            }
            createSymbolFile(file.path());
        }

        op::archive_from_glob(gcx(), conf().path().install_pdbs() / "*", out,
                              {"__pycache__"});
    }

    std::string release_command::version_from_exe() const
    {
        const auto mo = conf().path().install_bin() / "ModOrganizer";
        // clang-format off
        // `readelf -Wn <file> | grep NT_VERSION | sed -E 's/.*description data: (.*)/\\1/'` could also be used
        // clang-format on
        const string command = "readelf -Wn \"" + mo.string() + "\" 2>&1";
        FILE* file           = popen(command.c_str(), "r");
        if (file == nullptr) {
            const int e = errno;
            gcx().bail_out(context::generic, "can't get file version info from {}, {}",
                           mo, strerror(e));
        }

        // read output
        static constexpr int bufferSize = 1024;
        char buffer[bufferSize];
        vector<string> lines;
        gcx().debug(context::generic, "readelf output:");
        while (fgets(buffer, bufferSize, file) != nullptr) {
            string_view line = buffer;
            // remove trailing \n
            if (line.ends_with('\n')) {
                line.remove_suffix(1);
            }
            // only store non-empty lines
            if (!line.empty()) {
                gcx().debug(context::generic, " . {}", line);
                lines.emplace_back(line);
            }
        }

        pclose(file);

        // clang-format off
        // example line:
        //   (NONE)               0x0000000e       NT_VERSION (version)       description data: 33 2e 30 2e 30 2d 61 6c 70 68 61 2e 31 00
        // clang-format on

        // parse output
        static constexpr string_view anchor = "description data: ";
        for (const auto& line : lines) {
            if (line.contains("NT_VERSION")) {
                string_view lineView = line;

                // remove everything up to and including anchor
                const size_t pos = lineView.find(anchor);
                if (pos == string_view::npos) {
                    gcx().bail_out(context::generic,
                                   "can't get file version info from {}, error parsing "
                                   "version info",
                                   mo);
                }
                lineView.remove_prefix(pos + anchor.size());

                // remove trailing space
                if (lineView.ends_with(' ')) {
                    lineView.remove_suffix(1);
                }

                // extract hex values and store them as chars
                string versionString;
                istringstream iss(lineView.data());
                int c;
                iss >> hex;
                while (iss >> c && c != 0) {
                    versionString += static_cast<char>(c);
                }

                return versionString;
            }
        }
        gcx().bail_out(context::generic,
                       "can't get file version info from {}, no version string found",
                       mo);
    }

    void release_command::make_appimage()
    {
        const auto fileName = "ModOrganizer-" + version_ + "-x86_64.AppImage";
        const auto src      = conf().path().build() / "AppImage/ModOrganizer";
        const auto out      = out_ / fileName;

        static constexpr auto updateInfo = "gh-releases-zsync|Kaedras|ModOrganizer|"
                                           "latest|ModOrganizer-*x86_64.AppImage.zsync";

        u8cout << "making appimage " << out.string() << "\n";

        const string command = format("{} --appdir {} --output appimage",
                                      linuxdeploy::binary().string(), src.string());
        auto env             = this_env::get()
                       .set("UPDATE_INFORMATION", updateInfo)
                       .set("LINUXDEPLOY_OUTPUT_VERSION", version_)
                       .set("LINUXDEPLOY_EXCLUDED_LIBRARIES",
                            "libmysqlclient.so*;libqsqlmimer.so;libqsqlmysql.so;"
                            "libqsqlodbc.so;libqsqlpsql.so;libqsqloci.so");

        process p = process::raw(gcx(), command);
        p.cwd(out_);
        p.env(env);

        if (p.run_and_join() != 0) {
            gcx().bail_out(context::generic, "error creating appimage");
        }
    }

}  // namespace mob
