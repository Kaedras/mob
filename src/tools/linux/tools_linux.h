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

    // tool that runs linuxdeploy to create the appimage
    //
    class linuxdeploy : public basic_process_runner {
    public:
        // path to linuxdeploy-x86_64.AppImage
        //
        static fs::path binary();

        // output path
        //
        linuxdeploy& output(const fs::path& p);

        // appdir path
        //
        linuxdeploy& appdir(const fs::path& p);

        // icon file name
        //
        linuxdeploy& iconFileName(const std::string& p);

        // custom AppRun file
        //
        linuxdeploy& customAppRun(const fs::path& p);

        // libraries to exclude
        //
        linuxdeploy& excludeLibraries(const std::vector<std::string>& excludeLibraries);

        // libraries to exclude
        //
        linuxdeploy& excludeLibraries(const std::string& excludeLibraries);

        // additional paths to look up libraries
        //
        linuxdeploy& additionalLibraryPath(std::string path);

        // add an executable to deploy
        //
        linuxdeploy& executable(const fs::path& p);

        // add a shared library to deploy
        //
        linuxdeploy& library(const fs::path& p);

        // path to icon
        //
        linuxdeploy& icon(const fs::path& p);

        // path to desktop file
        //
        linuxdeploy& desktopFile(const fs::path& p);

        // pass --create-desktop-file
        linuxdeploy& createDesktopFile();

        // set NO_STRIP environment variable to 1
        linuxdeploy& nostrip();

        // linuxdeploy tool
        //
        linuxdeploy();

    protected:
        // runs linuxdeploy
        //
        void do_run() override;

    private:
        // path to the output file
        fs::path output_;
        // executables to deploy
        std::vector<fs::path> executables_;
        // libraries to deploy
        std::vector<fs::path> libraries_;
        // path to the AppRun file
        fs::path customAppRun_;
        // path to the icon
        fs::path icon_;
        // appdir path
        fs::path appdir_;
        // icon file name
        std::string iconFileName_;
        // desktop file
        fs::path desktopFile_;
        // libraries to exclude
        std::vector<std::string> excludeLibraries_;
        // additional library paths
        std::vector<std::string> additionalLibPaths_;
        // nostrip
        bool nostrip_ = false;
        // create a desktop file
        bool createDesktopFile_ = false;
    };

}  // namespace mob
