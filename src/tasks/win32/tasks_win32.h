#pragma once

#include "../../tools/tools.h"
#include "../../utility.h"

namespace mob::tasks {

    // header containing windows-specific tasks

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

}  // namespace mob::tasks