#include "pch.h"
#include "tasks.h"
#include "../tools/cmake.h"

#ifdef __unix__
static constexpr std::string LIB_EXT = "so";
#else
static constexpr std::string LIB_EXT = "dll";
#endif

namespace mob::tasks {

    namespace {

        cmake create_cmake_tool(arch a, config config,
                                    cmake::ops o = cmake::build)
        {
            return std::move(cmake(o).configuration(config).root(
                libbsarchpp::source_path()));
        }

    }  // namespace

    libbsarchpp::libbsarchpp() : basic_task("libbsarchpp") {}

    std::string libbsarchpp::version()
    {
        return conf().version().get("libbsarchpp");
    }

    config libbsarchpp::build_type()
    {
        return conf().build_types().get("libbsarchpp");
    }

    bool libbsarchpp::prebuilt()
    {
        return false;
    }

    fs::path libbsarchpp::source_path()
    {
        return conf().path().build() / "libbsarchpp";
    }

    void libbsarchpp::do_clean(clean c)
    {
        // delete the whole directory
        if (is_set(c, clean::reclone)) {
            op::delete_directory(cx(), source_path());
        }

        if (is_set(c, clean::rebuild)) {
            run_tool(create_cmake_tool(arch::x64, config::release, cmake::clean));
            run_tool(create_cmake_tool(arch::x64, config::debug, cmake::clean));
        }
    }

    void libbsarchpp::do_fetch()
    {
        run_tool(make_git()
                     .url(make_git_url("Kaedras", "libbsarchpp"))
                     .branch(version())
                     .root(source_path()));
    }

    void libbsarchpp::do_build_and_install()
    {
        auto tool = create_cmake_tool(arch::x64, config::release);
        run_tool(tool);
        run_tool(create_cmake_tool(arch::x64, config::debug));

        // copy dll
        op::copy_file_to_dir_if_better(cx(), tool.build_path() / ("libbsarchpp." + LIB_EXT),
                                       conf().path().install_dlls());
        op::copy_file_to_dir_if_better(cx(), tool.build_path() / ("libbsarchppd." + LIB_EXT),
                                       conf().path().install_dlls());
    }

}  // namespace mob::tasks
