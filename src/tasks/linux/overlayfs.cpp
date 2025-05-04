#include "../pch.h"
#include "../tasks.h"
#include "../../tools/cmake.h"

namespace mob::tasks {

    namespace {

        cmake create_cmake_tool(arch a, config config, cmake::ops o = cmake::install)
        {
            return std::move(cmake(o)
                                 .configuration(config)
                                 .root(overlayfs::source_path())
                                 .prefix(conf().path().install()));
        }

    }  // namespace

    overlayfs::overlayfs() : basic_task("overlayfs") {}

    std::string overlayfs::version()
    {
        return conf().version().get("overlayfs");
    }

    bool overlayfs::prebuilt()
    {
        return false;
    }

    fs::path overlayfs::source_path()
    {
        return conf().path().build() / "overlayfs";
    }

    void overlayfs::do_clean(clean c)
    {
        if (is_set(c, clean::reclone)) {
            git_wrap::delete_directory(cx(), source_path());
            return;
        }

        if (is_set(c, clean::rebuild)) {
            run_tool(create_cmake_tool(arch::x64, config::release, cmake::clean));
        }
    }

    void overlayfs::do_fetch()
    {
        run_tool(make_git()
                     .url(make_git_url("Kaedras", "mo2-overlayfs"))
                     .branch(version())
                     .root(source_path()));
    }

    void overlayfs::do_build_and_install()
    {
        auto tool = create_cmake_tool(arch::x64, config::release);
        run_tool(tool);
    }

}  // namespace mob::tasks
