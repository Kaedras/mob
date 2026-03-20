#include "../../core/conf.h"
#include "../../core/context.h"
#include "../../core/op.h"
#include "../commands.h"

using namespace std;

namespace mob {

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
        const auto file = "ModOrganizer-x86_64.AppImage";
        const auto src  = conf().path().install_appimage() / file;
        const auto dest = out_;

        u8cout << "copying appimage " << file << "\n";

        op::copy_file_to_dir_if_better(gcx(), src, dest);
    }

}  // namespace mob
