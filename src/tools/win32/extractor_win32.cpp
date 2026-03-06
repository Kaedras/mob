#include "../../core/process.h"
#include "../tools.h"

namespace mob {
    void archiver::create_from_glob(const context& cx, const fs::path& out,
                                    const fs::path& glob,
                                    const std::vector<std::string>& ignore)
    {
        op::create_directories(cx, out.parent_path());

        auto p = process()
                     .binary(extractor::binary())
                     .arg("a")      // add to archive
                     .arg(out)      // output file
                     .arg("-r")     // recursive
                     .arg("-mx=5")  // normal compression level
                     .arg(glob);    // input file

        for (auto&& i : ignore) {
            // x: exclude
            // r: recurse
            // !: filename or glob
            p.arg("-xr!", i, process::nospace);
        }

        p.run();
        p.join();
    }

    void archiver::create_from_files(const context& cx, const fs::path& out,
                                     const std::vector<fs::path>& files,
                                     const fs::path& files_root)
    {
        std::string list_file_text;
        std::error_code ec;

        // make each file relative to files_root, convert to utf8 and put in
        // list_file_text separated by newlines
        for (auto&& f : files) {
            fs::path rf = fs::relative(f, files_root, ec);

            if (ec) {
                cx.bail_out(context::fs, "file {} is not in root {}", f, files_root);
            }

            list_file_text += path_to_utf8(rf) + "\n";
        }

        const auto list_file = make_temp_file();

        // always delete the list file when done
        guard g([&] {
            if (fs::exists(list_file)) {
                std::error_code ec;
                fs::remove(list_file, ec);
            }
        });

        op::write_text_file(gcx(), encodings::utf8, list_file, list_file_text);
        op::create_directories(cx, out.parent_path());

        auto p = process()
                     .binary(extractor::binary())
                     .arg("a")  // add to archive
                     .arg(out)  // output file
                     .arg("@", list_file, process::nospace)
                     .cwd(files_root);

        p.run();
        p.join();
    }

}  // namespace mob
