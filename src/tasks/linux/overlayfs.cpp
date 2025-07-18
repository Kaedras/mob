#include "pch.h"
#include "../../tools/cmake.h"
#include "../tasks.h"

namespace mob::tasks {

    std::string overlayfs::cmake_prefix_path()
    {
        return conf().path().qt_install().string() + ";" +
               (modorganizer::super_path() / "cmake_common").string() + ";" +
               (conf().path().install() / "lib" / "cmake").string();
    }

    overlayfs::overlayfs() : basic_task("overlayfs"), repo_("mo2-overlayfs") {}

    std::string overlayfs::version()
    {
        return conf().version().get("overlayfs");
    }

    bool overlayfs::prebuilt()
    {
        return false;
    }

    url overlayfs::git_url() const
    {
        return make_git_url(task_conf().mo_org(), repo_);
    }

    std::string overlayfs::org() const
    {
        return task_conf().mo_org();
    }

    std::string overlayfs::repo() const
    {
        return repo_;
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
            run_tool(cmake(cmake::clean).root(source_path()));
        }
    }

    void overlayfs::do_fetch()
    {
        run_tool(make_git().url(git_url()).branch(version()).root(source_path()));
    }

    void overlayfs::do_build_and_install()
    {
        if (!exists(source_path() / "CMakeLists.txt")) {
            gcx().bail_out(context::generic, "{} has no CMakeLists.txt, not building",
                           repo_);

            return;
        }

        // there must be a CMakePresets.json otherwise
        // we cannot build
        if (!exists(source_path() / "CMakePresets.json")) {
            gcx().bail_out(context::generic,
                           "{} has no CMakePresets.txt, aborting build", repo_);
        }

        run_tool(cmake(cmake::generate)
                     .generator(cmake::ninjaMultiConfig)
                     .def("CMAKE_INSTALL_PREFIX:PATH", conf().path().install())
                     .def("CMAKE_PREFIX_PATH", cmake_prefix_path())
                     .configuration_types({task_conf().configuration()})
                     .preset("linux")
                     .root(source_path()));

        run_tool(cmake(cmake::build)
                     .root(source_path())
                     .arg("--parallel")
                     .arg(std::to_string(std::thread::hardware_concurrency()))
                     .configuration(task_conf().configuration()));

        run_tool(cmake(cmake::install)
                     .root(source_path())
                     .configuration(task_conf().configuration()));
    }

}  // namespace mob::tasks
