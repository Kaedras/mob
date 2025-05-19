#include "../pch.h"
#include "../tasks.h"
#include "../../tools/cmake.h"

// build from source

namespace mob::tasks {

    namespace {

        cmake create_cmake_tool(config config, cmake::ops o = cmake::install)
        {
            return std::move(cmake(o)
                                 .configuration(config)
                                 .root(libloot::source_path())
                                 .def("LIBLOOT_BUILD_TESTS", "OFF")
                                 .def("LIBLOOT_INSTALL_DOCS", "OFF")
                                 .prefix(conf().path().install()));
        }

    }  // namespace

    libloot::libloot() : basic_task("libloot") {}

    std::string libloot::version()
    {
        return conf().version().get("libloot");
    }

    bool libloot::prebuilt()
    {
        return false;
    }

    fs::path libloot::source_path()
    {
        return conf().path().build() / "libloot";
    }

    void libloot::do_clean(clean c)
    {
        if (is_set(c, clean::reclone)) {
            git_wrap::delete_directory(cx(), source_path());
            return;
        }

        if (is_set(c, clean::rebuild)) {
            run_tool(create_cmake_tool(config::release, cmake::clean));
        }
    }

    void libloot::do_fetch()
    {
        run_tool(make_git()
                     .url(make_git_url("loot", "libloot"))
                     .branch(version())
                     .root(source_path()));
    }

    void libloot::do_build_and_install()
    {
        run_tool(create_cmake_tool(config::release));

        op::copy_file_to_dir_if_better(cx(),
                               conf().path().install() / "lib64/libloot.so.0",
                               conf().path().install_bin() / "loot");
    }

}  // namespace mob::tasks
