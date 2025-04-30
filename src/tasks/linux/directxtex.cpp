#include "../pch.h"
#include "../tasks.h"
#include "../../tools/cmake.h"

#ifdef __unix__
static constexpr const char* LIB_NAME = "libDirectXTex.a";
#else
static constexpr const char* LIB_NAME = "DirectXTex.lib";
#endif

// todo: test this on windows
namespace mob::tasks {

    namespace {

        cmake create_cmake_tool(arch a, config config,
                                    cmake::ops o = cmake::build)
        {
            return std::move(cmake(o).configuration(config).root(directxtex::source_path())
                .prefix_path(conf().path().install()).output(directxtex::source_path() / "out"));
        }

    }  // namespace

    directxtex::directxtex() : basic_task("directxtex") {}

    std::string directxtex::version()
    {
        return conf().version().get("directxtex");
    }

    bool directxtex::prebuilt()
    {
        return false;
    }

    fs::path directxtex::source_path()
    {
        return conf().path().build() / "DirectXTex";
    }

    void directxtex::do_clean(clean c)
    {
        if (is_set(c, clean::reclone)) {
            git_wrap::delete_directory(cx(), source_path());
            return;
        }

        if (is_set(c, clean::rebuild)) {
            run_tool(create_cmake_tool(arch::x64, config::release, cmake::clean));
            run_tool(create_cmake_tool(arch::x64, config::debug, cmake::clean));
        }
    }

    void directxtex::do_fetch()
    {
        run_tool(make_git()
                     .url(make_git_url("microsoft", "DirectXTex"))
                     .branch(version())
                     .root(source_path()));
    }

    void directxtex::do_build_and_install()
    {
        op::create_directories(cx(), directxtex::source_path() / "Include");
        op::create_directories(cx(), directxtex::source_path() / "Lib" / "Debug");
        op::create_directories(cx(), directxtex::source_path() / "Lib" / "Release");

        const auto binary_path =
            source_path() / "out/lib";

        // DO NOT run these in parallel because both generate files that are shared
        // between release and debug
        run_tool(create_cmake_tool(arch::x64, config::release));
        op::copy_file_to_dir_if_better(cx(), binary_path / LIB_NAME,
                                       source_path() / "Lib" / "Debug");

        run_tool(create_cmake_tool(arch::x64, config::debug));
        op::copy_file_to_dir_if_better(cx(), binary_path / LIB_NAME,
                                       source_path() / "Lib" / "Release");

        for (const auto& header : {"DDS.h", "DirectXTex.h", "DirectXTex.inl"}) {
            op::copy_file_to_dir_if_better(cx(), source_path() / "DirectXTex" / header,
                                           source_path() / "Include");
        }

        // copy "dxgiformat.h" from install dir
        op::copy_file_to_dir_if_better(cx(), conf().path().install() / "include/dxgiformat.h", source_path() / "Include");
    }

}  // namespace mob::tasks
