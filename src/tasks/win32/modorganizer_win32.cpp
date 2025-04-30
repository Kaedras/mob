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

        // run restore for nuget
        //
        // until https://gitlab.kitware.com/cmake/cmake/-/issues/20646 is resolved,
        // we need a manual way of running the msbuild -t:restore
        if (is_nuget_plugin())
            run_tool(create_msbuild_tool().targets({"restore"}));

        // run msbuild
        run_tool(create_msbuild_tool());
    }

    cmake modorganizer::create_cmake_tool(const fs::path& root, cmake::ops o, config c)
    {
        return std::move(
            cmake(o)
                .generator(cmake::vs)
                .def("CMAKE_INSTALL_PREFIX:PATH", conf().path().install())
                .def("DEPENDENCIES_DIR", conf().path().build())
                .def("BOOST_ROOT", boost::source_path())
                .def("BOOST_LIBRARYDIR", boost::lib_path(arch::x64))
                .def("SPDLOG_ROOT", spdlog::source_path())
                .def("LOOT_PATH", libloot::source_path())
                .def("LZ4_ROOT", lz4::source_path())
                .def("QT_ROOT", qt::installation_path())
                .def("ZLIB_ROOT", zlib::source_path())
                .def("PYTHON_ROOT", python::source_path())
                .def("SEVENZ_ROOT", sevenz::source_path())
                .def("LIBBSARCH_ROOT", libbsarch::source_path())
                .def("BOOST_DI_ROOT", boost_di::source_path())
                // gtest has no RelWithDebInfo, so simply use Debug/Release
                .def("GTEST_ROOT",
                     gtest::build_path(arch::x64, c == config::debug ? config::debug
                                                                     : config::release))
                .def("OPENSSL_ROOT_DIR", openssl::source_path())
                .def("DIRECTXTEX_ROOT", directxtex::source_path())
                .root(root));
    }

    msbuild modorganizer::create_msbuild_tool(msbuild::ops o)
    {
        return std::move(msbuild(o)
                             .solution(project_file_path())
                             .configuration(task_conf().configuration())
                             .architecture(arch::x64));
    }

}  // namespace mob::tasks