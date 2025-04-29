#include "../pch.h"
#include "../tasks.h"
#include "../../tools/cmakebuild.h"

#ifdef __unix__
static constexpr const char* RELEASE_PRESET = "x64-Release-Linux";
static constexpr const char* DEBUG_PRESET = "x64-Debug-Linux";
static constexpr const char* LIB_NAME = "libDirectXTex.so";
#else
static constexpr const char* RELEASE_PRESET = "x64-Release";
static constexpr const char* DEBUG_PRESET = "x64-Debug";
static constexpr const char* LIB_NAME = "DirectXTex.lib";
#endif

namespace mob::tasks {

    namespace {

        cmakebuild create_cmakebuild_tool(arch a, config config,
                                    cmakebuild::ops o = cmakebuild::build)
        {
            return std::move(cmakebuild(o).architecture(a).configuration(config).sourcedir(
                directxtex::source_path()));
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
            run_tool(create_cmakebuild_tool(arch::x64, config::release, cmakebuild::clean));
            run_tool(create_cmakebuild_tool(arch::x64, config::debug, cmakebuild::clean));
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

        // DO NOT run these in parallel because both generate files that are shared
        // between release and debug
        run_tool(create_cmakebuild_tool(arch::x64, config::release));
        run_tool(create_cmakebuild_tool(arch::x64, config::debug));

        const auto binary_path_release =
            source_path() / "out" / "build" / RELEASE_PRESET;
        const auto binary_path_debug =
            source_path() / "out" / "build" / DEBUG_PRESET;

        for (const auto& header : {"DDS.h", "DirectXTex.h", "DirectXTex.inl"}) {
            op::copy_file_to_dir_if_better(cx(), source_path() / "DirectXTex" / header,
                                           source_path() / "Include");
        }
        op::copy_file_to_dir_if_better(cx(), binary_path_debug / "Debug" / LIB_NAME,
                                       source_path() / "Lib" / "Debug");
        op::copy_file_to_dir_if_better(cx(), binary_path_release / "Release" / LIB_NAME,
                                       source_path() / "Lib" / "Release");
    }

}  // namespace mob::tasks
