#include "../pch.h"
#include "../tasks.h"

namespace mob::tasks {

    void modorganizer::do_build_and_install()
    {
        // adds a git submodule in modorganizer_super for this project; note that
        // git_submodule_adder runs a thread because adding submodules is slow, but
        // can happen while stuff is building
        git_submodule_adder::instance().queue(
            std::move(git_submodule()
                          .url(git_url())
                          .branch(task_conf().mo_branch())
                          .submodule(name())
                          .root(super_path())));

        // not all modorganizer projects need to actually be built, such as
        // cmake_common, so don't try if there's no cmake file
        if (!fs::exists(source_path() / "CMakeLists.txt")) {
            cx().trace(context::generic, "{} has no CMakeLists.txt, not building",
                       repo_);

            return;
        }

        // run cmake
        run_tool(create_cmake_tool());
    }

    cmake modorganizer::create_cmake_tool(const fs::path& root, cmake::ops o, config c)
    {
        return std::move(cmake(cmake::install)
                             .configuration(c)
                             .def("CMAKE_INSTALL_PREFIX:PATH", conf().path().install())
                             .def("DEPENDENCIES_DIR", conf().path().build())
                             .def("SPDLOG_ROOT", spdlog::source_path())
                             .def("LOOT_PATH", libloot::source_path())
                             .def("DIRECTXTEX_ROOT", directxtex::source_path())
                             .def("SEVENZ_ROOT", sevenz::source_path())
                             .def("LIBBSARCHPP_ROOT", libbsarchpp::source_path())
                             .def("BOOST_DI_ROOT", boost_di::source_path())
                             .root(root));
    }

}  // namespace mob::tasks