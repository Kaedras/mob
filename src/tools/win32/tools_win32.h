#pragma once

#include <string>

namespace mob {

    // bundled with mob in third-party/bin
    //
    struct vswhere {
        // runs the vswhere binary and returns the output, empty on error
        //
        // note that vswhere may return more than one line if there are multiple
        // installations of vs found; this is handled in find_vs() in paths.cpp
        //
        static std::string find_vs();
    };
    // tool that runs devenv.exe, only invoked to upgrade projects for now
    //
    class vs : public basic_process_runner {
    public:
        // path to devenv.exe
        //
        static fs::path devenv_binary();

        // path to visual studio's root directory, the one that contains Common7,
        // VC, etc.
        //
        static fs::path installation_path();

        // path to vswhere.exe, typically from mob's third-party/bin
        //
        static fs::path vswhere();

        // path to vcvars batch file
        //
        static fs::path vcvars();

        // vs version from ini
        //
        static std::string version();

        // vs year from ini
        //
        static std::string year();

        // vs toolset from ini
        //
        static std::string toolset();

        // vs sdk version from ini
        //
        static std::string sdk();

        // what run() should do
        //
        enum ops { upgrade = 1 };

        vs(ops o);

        // path to the solution file to be upgraded
        //
        vs& solution(const fs::path& sln);

    protected:
        // calls do_upgrade()
        //
        void do_run() override;

    private:
        ops op_;
        fs::path sln_;

        // upgrades the solution file
        //
        void do_upgrade();
    };

    // tool that runs Inno Setup's iscc.exe to create the installer
    //
    class iscc : public basic_process_runner {
    public:
        // path to the iscc.exe binary
        //
        static fs::path binary();

        // iscc tool with an optional path to the .iss file
        //
        iscc(fs::path iss = {});

        // .iss file
        //
        iscc& iss(const fs::path& p);

    protected:
        // runs iscc
        //
        void do_run() override;

    private:
        // path to the .iss file
        fs::path iss_;
    };

}  // namespace mob
