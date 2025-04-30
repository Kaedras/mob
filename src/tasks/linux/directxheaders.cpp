#include "../pch.h"
#include "../tasks.h"
#include "../../tools/cmake.h"

namespace mob::tasks {

    namespace {

        cmake create_cmake_tool(arch a, config config, cmake::ops o = cmake::install)
        {
            return std::move(cmake(o)
                                 .configuration(config)
                                 .root(directxheaders::source_path())
                                 .def("DXHEADERS_BUILD_TEST", "OFF")
                                 .def("DXHEADERS_BUILD_GOOGLE_TEST", "OFF")
                                 .output(directxheaders::source_path() / "out")
                                 .prefix(conf().path().install()));
        }

    }  // namespace

    directxheaders::directxheaders() : basic_task("directxheaders") {}

    std::string directxheaders::version()
    {
        return conf().version().get("directxheaders");
    }

    bool directxheaders::prebuilt()
    {
        return false;
    }

    fs::path directxheaders::source_path()
    {
        return conf().path().build() / "DirectX-Headers";
    }

    void directxheaders::do_clean(clean c)
    {
        if (is_set(c, clean::reclone)) {
            git_wrap::delete_directory(cx(), source_path());
            return;
        }

        if (is_set(c, clean::rebuild)) {
            run_tool(create_cmake_tool(arch::x64, config::release, cmake::clean));
        }
    }

    void directxheaders::do_fetch()
    {
        run_tool(make_git()
                     .url(make_git_url("microsoft", "DirectX-Headers"))
                     .branch(version())
                     .root(source_path()));
    }

    void directxheaders::do_build_and_install()
    {
        run_tool(create_cmake_tool(arch::x64, config::release));

        // copy "dxgiformat.h" to install dir
        op::copy_file_to_dir_if_better(cx(),
                                       source_path() / "include/directx/dxgiformat.h",
                                       conf().path().install() / "include");
    }

}  // namespace mob::tasks
