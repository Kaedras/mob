#pragma once

#include "../../core/conf.h"
#include "../../core/op.h"
#include "../../net.h"
#include "../../tools/tools.h"
#include "../../utility.h"
#include <set>

namespace mob::tasks {

    // header containing windows specific tasks

    class boost : public basic_task<boost> {
    public:
        struct version_info {
            std::string major, minor, patch, rest;
        };

        boost();

        static version_info parsed_version();
        static std::string version();
        static std::string version_vs();
        static bool prebuilt();

        static fs::path source_path();
        static fs::path lib_path(arch a);
        static fs::path root_lib_path(arch a);

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
        void do_build_and_install() override;

    private:
        void fetch_prebuilt();
        void build_and_install_prebuilt();

        void fetch_from_source();
        void build_and_install_from_source();
        void write_config_jam();

        void bootstrap();

        void do_b2(const std::vector<std::string>& components, const std::string& link,
                   const std::string& runtime_link, arch a);
    };

    // needed by bsapacker
    //
    class boost_di : public basic_task<boost_di> {
    public:
        boost_di();

        static std::string version();
        static bool prebuilt();
        static fs::path source_path();

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
    };

    // needed by python
    //
    class bzip2 : public basic_task<bzip2> {
    public:
        bzip2();

        static std::string version();
        static bool prebuilt();
        static fs::path source_path();

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
    };

    class explorerpp : public basic_task<explorerpp> {
    public:
        explorerpp();

        static std::string version();
        static bool prebuilt();
        static fs::path source_path();

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
    };

    class gtest : public basic_task<gtest> {
    public:
        gtest();

        static std::string version();
        static bool prebuilt();
        static fs::path source_path();
        static fs::path build_path(arch = arch::x64, config = config::release);

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
        void do_build_and_install() override;
    };

    class ncc : public basic_task<ncc> {
    public:
        ncc();

        static std::string version();
        static bool prebuilt();
        static fs::path source_path();

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
        void do_build_and_install() override;

    private:
        msbuild create_msbuild_tool(msbuild::ops o);
    };

    class nmm : public basic_task<nmm> {
    public:
        nmm();

        static std::string version();
        static bool prebuilt();
        static fs::path source_path();

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
        void do_build_and_install() override;

    private:
        msbuild create_msbuild_tool(msbuild::ops o     = msbuild::build,
                                    msbuild::flags_t f = msbuild::noflags);
    };

    class lz4 : public basic_task<lz4> {
    public:
        lz4();

        static std::string version();
        static bool prebuilt();
        static fs::path source_path();

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
        void do_build_and_install() override;

    private:
        void fetch_prebuilt();
        void build_and_install_prebuilt();

        void fetch_from_source();
        void build_and_install_from_source();

        msbuild create_msbuild_tool(msbuild::ops o = msbuild::build);
    };

    class openssl : public basic_task<openssl> {
    public:
        struct version_info {
            std::string major, minor, patch;
        };

        openssl();

        static version_info parsed_version();
        static std::string version();
        static bool prebuilt();

        static fs::path source_path();
        static fs::path include_path();
        static fs::path build_path();
        static fs::path bin_path();

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
        void do_build_and_install() override;

    private:
        void fetch_prebuilt();
        void fetch_from_source();
        void build_and_install_prebuilt();
        void build_and_install_from_source();

        void configure();
        void install_engines();
        void copy_files();
        void copy_dlls_to(const fs::path& dir);
        void copy_pdbs_to(const fs::path& dir);
    };

    // when building from source, builds both pyqt5 and pyqt5-sip; when using the
    // prebuilt, the downloaded zip contains both
    //
    class pyqt : public basic_task<pyqt> {
    public:
        pyqt();

        static std::string version();
        static std::string builder_version();
        static bool prebuilt();

        static fs::path source_path();
        static fs::path build_path();
        static config build_type();

        // "PyQt5.sip", used both in pyqt and sip
        //
        static std::string pyqt_sip_module_name();

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
        void do_build_and_install() override;

    private:
        void fetch_prebuilt();
        void fetch_from_source();
        void build_and_install_prebuilt();
        void build_and_install_from_source();

        void sip_build();
        void install_sip_file();
        void copy_files();
    };

    class python : public basic_task<python> {
    public:
        struct version_info {
            std::string major;
            std::string minor;
            std::string patch;
        };

        python();

        static std::string version();
        static bool prebuilt();
        static version_info parsed_version();

        // build/python-XX
        //
        static fs::path source_path();

        // build/python-XX/PCBuild/amd64
        //
        static fs::path build_path();

        // build/python-XX/PCBuild/amd64/python.exe
        //
        static fs::path python_exe();

        // build/python-XX/Include
        //
        static fs::path include_path();

        // build/python-XX/Scripts
        //
        static fs::path scripts_path();

        // build/python-XX/Lib/site-packages
        //
        static fs::path site_packages_path();

        // configuration to build
        //
        static config build_type();

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
        void do_build_and_install() override;

    private:
        void fetch_prebuilt();
        void fetch_from_source();
        void build_and_install_prebuilt();
        void build_and_install_from_source();

        void prepare_dependencies();
        void package();
        void install_pip();
        void copy_files();

        msbuild create_msbuild_tool(msbuild::ops o = msbuild::build);
    };

    class pybind11 : public basic_task<pybind11> {
    public:
        pybind11();

        static bool prebuilt();
        static std::string version();
        static fs::path source_path();

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
        void do_build_and_install() override;
    };

    class sip : public basic_task<sip> {
    public:
        sip();

        static std::string version();
        static std::string version_for_pyqt();
        static bool prebuilt();

        static fs::path source_path();
        static fs::path sip_module_exe();
        static fs::path sip_install_exe();
        static fs::path module_source_path();

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
        void do_build_and_install() override;

    private:
        void build();
        void generate_header();
        void convert_script_file_to_acp(const std::string& filename);
    };

    class usvfs : public basic_task<usvfs> {
    public:
        usvfs();

        static std::string version();
        static bool prebuilt();
        static fs::path source_path();

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
        void do_build_and_install() override;

    private:
        void fetch_from_source();
        void build_and_install_from_source();

        msbuild create_msbuild_tool(arch, msbuild::ops = msbuild::build,
                                    config = config::release) const;
    };

    class zlib : public basic_task<zlib> {
    public:
        zlib();

        static std::string version();
        static bool prebuilt();
        static fs::path source_path();

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
        void do_build_and_install() override;

    private:
        cmake create_cmake_tool(cmake::ops o = cmake::generate);
        msbuild create_msbuild_tool(msbuild::ops o = msbuild::build);
    };

}  // namespace mob::tasks