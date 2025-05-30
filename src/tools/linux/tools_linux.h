#pragma once

namespace mob {

    // tool that runs qt installer framework
    //
    class qtifw : public basic_process_runner {
    public:
        enum type {
            unspecified,
            online,  // Compile without any component in the installer binary.
            offline  // Create an offline installer that never accesses online
                     // repositories.
        };
        enum archive_format { seven_zip, zip, tar, tar_gz, tar_bz2, tar_xz };

        // path to the ifw binary
        //
        static fs::path binary();

        // ifw tool with an optional path to the config file
        //
        qtifw(fs::path config = {});

        // output path
        //
        qtifw& output(const fs::path& p);

        // resource path
        //
        qtifw& resource(const fs::path& p);

        // config path
        //
        qtifw& config(const fs::path& p);

        // package path
        //
        qtifw& package(const fs::path& p);

        // installer type
        //
        qtifw& type(type t);

        // archive format
        //
        qtifw& format(archive_format af);

        // compression level (0-9)
        // note: Some formats do not support all the possible values, for example, bzip2
        // compression only supports values from 1 to 9.
        //
        qtifw& compression_level(int level);

    protected:
        // runs ifw
        //
        void do_run() override;

    private:
        // path to the config file
        fs::path config_;
        // path to the output file
        fs::path output_;
        // path to the resource file
        fs::path resource_;
        // package path
        fs::path package_;
        // installer type
        enum type type_ = unspecified;
        // archive format
        archive_format format_ = seven_zip;
        // compression level
        int compression_level_ = 5;

        static std::string archiveFormatToString(archive_format af);
    };

}  // namespace mob
