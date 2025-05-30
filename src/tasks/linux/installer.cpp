#include "../pch.h"
#include "../tasks.h"

#define VERSION std::string("3.0.0")

namespace mob::tasks {

    namespace {
        qtifw create_ifw_tool(const fs::path& config_path)
        {
            return std::move(
                qtifw(config_path)
                    .type(qtifw::offline)
                    .resource(installer::source_path() / "resources/additional.qrc")
                    .package(installer::source_path() / "packages")
                    .output(conf().path().install_installer() /
                            ("Mod.Organizer-" + VERSION)));
        }
    }  // namespace

    installer::installer() : basic_task("installer") {}

    bool installer::prebuilt()
    {
        return false;
    }

    std::string installer::version()
    {
        return {};
    }

    fs::path installer::source_path()
    {
        return modorganizer::super_path() / "installer";
    }

    void installer::do_clean(clean c)
    {
        // delete the git clone directory
        if (is_set(c, clean::reclone))
            git_wrap::delete_directory(cx(), source_path());

        // the installer script outputs directly in the installer directory, delete
        // it
        if (is_set(c, clean::rebuild))
            op::delete_directory(cx(), conf().path().install_installer());
    }

    void installer::do_fetch()
    {
        const std::string repo = "modorganizer-Installer";

        run_tool(make_git()
                     .url(make_git_url(task_conf().mo_org(), repo))
                     .branch(task_conf().mo_branch())
                     .root(source_path()));
    }

    void installer::do_build_and_install()
    {
        auto tool = create_ifw_tool(source_path() / "config" / "config.xml");
        run_tool(tool);
    }

}  // namespace mob::tasks
