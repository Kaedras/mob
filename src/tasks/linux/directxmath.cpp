#include "../pch.h"
#include "../tasks.h"
#include "../../tools/cmake.h"

namespace mob::tasks {

    namespace {

        cmake create_cmake_tool(arch a, config config,
                                    cmake::ops o = cmake::install)
        {
            return std::move(cmake(o).configuration(config).root(
                directxmath::source_path())
                .output(directxmath::source_path() / "out")
                .prefix(conf().path().install())
                );
        }

    }  // namespace

    directxmath::directxmath() : basic_task("directxmath") {}

    std::string directxmath::version()
    {
        return conf().version().get("directxmath");
    }

    bool directxmath::prebuilt()
    {
        return false;
    }

    fs::path directxmath::source_path()
    {
        return conf().path().build() / "DirectXMath";
    }

    void directxmath::do_clean(clean c)
    {
        if (is_set(c, clean::reclone)) {
            git_wrap::delete_directory(cx(), source_path());
            return;
        }

        if (is_set(c, clean::rebuild)) {
            run_tool(create_cmake_tool(arch::x64, config::release, cmake::clean));
        }
    }

    void directxmath::do_fetch()
    {
        run_tool(make_git()
                     .url(make_git_url("microsoft", "DirectXMath"))
                     .branch(version())
                     .root(source_path()));
    }

    void directxmath::do_build_and_install()
    {
        run_tool(create_cmake_tool(arch::x64, config::release));

        // download and copy sal.h
        auto path = run_tool(downloader("https://raw.githubusercontent.com/dotnet/runtime/main/src/coreclr/pal/inc/rt/sal.h"));
        op::copy_file_to_dir_if_better(cx(), path, conf().path().install() / "include");
    }

}  // namespace mob::tasks
