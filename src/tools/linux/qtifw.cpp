#include "../../core/process.h"
#include "../tools.h"

#include <utility>

namespace mob {
    fs::path qtifw::binary()
    {
        return conf().tool().get("qtifw");
    }

    qtifw::qtifw(fs::path config)
        : basic_process_runner("qtifw"), config_(std::move(config)),
          package_("packages")
    {
    }

    qtifw& qtifw::output(const fs::path& p)
    {
        output_ = p;
        return *this;
    }

    qtifw& qtifw::resource(const fs::path& p)
    {
        resource_ = p;
        return *this;
    }

    qtifw& qtifw::config(const fs::path& p)
    {
        config_ = p;
        return *this;
    }

    qtifw& qtifw::type(enum type t)
    {
        type_ = t;
        return *this;
    }

    qtifw& qtifw::package(const fs::path& p)
    {
        package_ = p;
        return *this;
    }

    qtifw& qtifw::format(archive_format af)
    {
        format_ = af;
        return *this;
    }

    qtifw& qtifw::compression_level(int level)
    {
        if (level >= 0 && level <= 9) {
            compression_level_ = level;
        }
        return *this;
    }

    void qtifw::do_run()
    {
        if (config_.empty())
            cx().bail_out(context::generic, "qtifw missing config file");

        process p = process().binary(binary()).arg("-c", config_);

        switch (type_) {
        case online:
            p.arg("--online-only");
            break;
        case offline:
            p.arg("--offline-only");
            break;
        case unspecified:
        default:
            break;
        }

        if (!resource_.empty()) {
            p.arg("-r", resource_);
        }

        p.arg("-p", package_);
        p.arg("--archive-format", archiveFormatToString(format_));
        p.arg("--compression", compression_level_);
        p.arg(output_);

        execute_and_join(p);
    }

    std::string qtifw::archiveFormatToString(archive_format af)
    {
        switch (af) {
        case zip:
            return "zip";
        case tar:
            return "tar";
        case tar_gz:
            return "tar.gz";
        case tar_bz2:
            return "tar.bz2";
        case tar_xz:
            return "tar.xz";
        default:
        case seven_zip:
            return "7z";
        }
    }

}  // namespace mob
