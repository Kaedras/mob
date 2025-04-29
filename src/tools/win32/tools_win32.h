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


    // tool that runs `nmake`
    //
    class nmake : public basic_process_runner {
    public:
        // path to the job binary
        //
        static fs::path binary();

        nmake();

        // path in which to invoke nmake
        //
        nmake& path(const fs::path& p);

        // makefile target
        //
        nmake& target(const std::string& s);

        // adds a macro definition as "NAME=value"
        //
        nmake& def(const std::string& s);

        // sets the architecture used to run nmake, used to get the appropriate
        // VS environment variables, defaults to arch::def
        //
        nmake& architecture(arch a);

        // nmake's exit code
        //
        int result() const;

    protected:
        // runs nmake
        //
        void do_run() override;

    private:
        // set in path()
        fs::path cwd_;

        // set in def(), just plain arguments
        std::vector<std::string> def_;

        // set in target(), another plain argument
        std::string target_;

        // set in architecture()
        arch arch_;
    };


    // tool that runs `jom`, an alternative to `nmake` that supports parallel
    // builds, bundled with mob in third-party/bin
    //
    // note that although jom reduces build times considerably, some third parties
    // don't handle it very well, giving a bunch of errors about files in use, etc.
    //
    // so most tools that use jom use a loop that runs jom several times, allowing
    // failure, and then run jom one last time in single_job mode to finish the
    // build, which should be guaranteed to work
    //
    class jom : public basic_process_runner {
    public:
        // path to the job binary
        //
        static fs::path binary();

        enum flags_t {
            noflags = 0x00,

            // disables multi-process build
            single_job = 0x01,

            // don't bail out on failure
            allow_failure = 0x02
        };

        jom();

        // path in which to invoke jom
        //
        jom& path(const fs::path& p);

        // makefile target
        //
        jom& target(const std::string& s);

        // adds a macro definition as "NAME=value"
        //
        jom& def(const std::string& s);

        // sets flags
        //
        jom& flag(flags_t f);

        // sets the architecture used to run jom, used to get the appropriate
        // VS environment variables, defaults to arch::def
        //
        jom& architecture(arch a);

        // jom's exit code
        //
        int result() const;

    protected:
        // runs jom
        //
        void do_run() override;

    private:
        // set in path()
        fs::path cwd_;

        // set in def(), just plain arguments
        std::vector<std::string> def_;

        // set in target(), another plain argument
        std::string target_;

        // set in flag()
        flags_t flags_;

        // set in architecture()
        arch arch_;
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

    // tool that runs `nuget restore` for the given solution, bundled with mob in
    // third-party/bin
    //
    class nuget : public basic_process_runner {
    public:
        // nuget tool for the given solution
        //
        nuget(fs::path sln);

        // path to the nuget binary
        //
        static fs::path binary();

    protected:
        // runs nuget
        void do_run() override;

    private:
        // solution file set in constructor
        fs::path sln_;
    };


}
