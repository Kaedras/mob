#include "../../core/process.h"
#include "../tools.h"

namespace {
    inline constexpr int zstdCompressionLevel = 19;
}

namespace mob {

    void archiver::create_from_glob(const context& cx, const fs::path& out,
                                    const fs::path& glob,
                                    const std::vector<std::string>& ignore)
    {
        std::string list_file_text;
        for (auto&& i : ignore) {
            list_file_text += i + "\n";
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

        auto p = process().binary("tar").arg("-c");  // create archive
        if (out.extension() == ".zst") {
            p.arg(std::format("-I 'zstd -{} -T0 --auto-threads=logical -q'",
                              zstdCompressionLevel));
        }
        else {
            p.arg("-a");  // auto-compress
        }
        p.arg("-f", out)                    // output file
            .arg("-X", list_file)           // file exclusion list
            .arg("-C", glob.parent_path())  // input directory
            .arg(".");

        p.run();
        p.join();
    }

    void archiver::create_from_files(const context& cx, const fs::path& out,
                                     const std::vector<fs::path>& files,
                                     const fs::path& files_root)
    {
        std::string list_file_text;
        std::error_code ec;

        // make each file relative to files_root and put in
        // list_file_text separated by newlines
        for (auto&& f : files) {
            fs::path rf = fs::relative(f, files_root, ec);

            if (ec) {
                cx.bail_out(context::fs, "file {} is not in root {}", f, files_root);
            }

            list_file_text += rf.string() + "\n";
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

        auto p = process().binary("tar").arg("-c");  // create
        if (out.extension() == ".zst") {
            p.arg(std::format("-I 'zstd -{} -T0 --auto-threads=logical -q'",
                              zstdCompressionLevel));
        }
        else {
            p.arg("-a");  // auto-compress
        }
        p.arg("-f", out)  // output file
            .arg("-C", files_root)
            .arg("-T", list_file);

        p.run();
        p.join();
    }

}  // namespace mob
