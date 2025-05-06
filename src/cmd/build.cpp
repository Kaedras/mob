#include "pch.h"
#include "../core/conf.h"
#include "../core/context.h"
#include "../core/ini.h"
#include "../core/op.h"
#include "../tasks/task_manager.h"
#include "commands.h"

namespace mob {

    build_command::build_command() : command(requires_options | handle_sigint) {}

    command::meta_t build_command::meta() const
    {
        return {"build", "builds tasks"};
    }

    clipp::group build_command::do_group()
    {
        return (clipp::command("build")).set(picked_),

               (clipp::option("-h", "--help") >> help_) % ("shows this message"),

               (clipp::option("-g", "--redownload") >> redownload_) %
                   "redownloads archives, see --reextract",

               (clipp::option("-e", "--reextract") >> reextract_) %
                   "deletes source directories and re-extracts archives",

               (clipp::option("-c", "--reconfigure") >> reconfigure_) %
                   "reconfigures the task by running cmake, configure scripts, "
                   "etc.; some tasks might have to delete the whole source "
                   "directory",

               (clipp::option("-b", "--rebuild") >> rebuild_) %
                   "cleans and rebuilds projects; some tasks might have to "
                   "delete the whole source directory",

               (clipp::option("-n", "--new") >> new_) %
                   "deletes everything and starts from scratch",

               (clipp::option("--clean-task").call([&] {
                   clean_ = true;
               }) |
                clipp::option("--no-clean-task").call([&] {
                    clean_ = false;
                })) %
                   "sets whether tasks are cleaned",

               (clipp::option("--fetch-task").call([&] {
                   fetch_ = true;
               }) |
                clipp::option("--no-fetch-task").call([&] {
                    fetch_ = false;
                })) %
                   "sets whether tasks are fetched",

               (clipp::option("--build-task").call([&] {
                   build_ = true;
               }) |
                clipp::option("--no-build-task").call([&] {
                    build_ = false;
                })) %
                   "sets whether tasks are built",

               (clipp::option("--pull").call([&] {
                   nopull_ = false;
               }) |
                clipp::option("--no-pull").call([&] {
                    nopull_ = true;
                })) %
                   "whether to pull repos that are already cloned; global override",

               (clipp::option("--revert-ts").call([&] {
                   revert_ts_ = true;
               }) |
                clipp::option("--no-revert-ts").call([&] {
                    revert_ts_ = false;
                })) %
                   "whether to revert all the .ts files in a repo before pulling to "
                   "avoid merge errors; global override",

               (clipp::option("--ignore-uncommitted-changes") >> ignore_uncommitted_) %
                   "when --reextract is given, directories controlled by git will "
                   "be deleted even if they contain uncommitted changes",

               (clipp::option("--keep-msbuild") >> keep_msbuild_) %
                   "don't terminate msbuild.exe instances after building",

               (clipp::opt_values(clipp::match::prefix_not("-"), "task", tasks_)) %
                   "tasks to run; specify 'super' to only build modorganizer "
                   "projects";
    }

    void build_command::convert_cl_to_conf()
    {
        command::convert_cl_to_conf();

        if (redownload_ || new_)
            common.options.push_back("global/redownload=true");

        if (reextract_ || new_)
            common.options.push_back("global/reextract=true");

        if (reconfigure_ || new_)
            common.options.push_back("global/reconfigure=true");

        if (rebuild_ || new_)
            common.options.push_back("global/rebuild=true");

        if (ignore_uncommitted_)
            common.options.push_back("global/ignore_uncommitted=true");

        if (clean_) {
            if (*clean_)
                common.options.push_back("global/clean_task=true");
            else
                common.options.push_back("global/clean_task=false");
        }

        if (fetch_) {
            if (*fetch_)
                common.options.push_back("global/fetch_task=true");
            else
                common.options.push_back("global/fetch_task=false");
        }

        if (build_) {
            if (*build_)
                common.options.push_back("global/build_task=true");
            else
                common.options.push_back("global/build_task=false");
        }

        if (nopull_) {
            if (*nopull_)
                common.options.push_back("_override:task/no_pull=true");
            else
                common.options.push_back("_override:task/no_pull=false");
        }

        if (revert_ts_) {
            if (*revert_ts_)
                common.options.push_back("_override:task/revert_ts=true");
            else
                common.options.push_back("_override:task/revert_ts=false");
        }

        if (!tasks_.empty())
            set_task_enabled_flags(tasks_);
    }

    int build_command::do_run()
    {
        try {
            create_prefix_ini();

            task_manager::instance().run_all();

            if (!keep_msbuild_)
                terminate_msbuild();

            mob::gcx().info(mob::context::generic, "mob done");
            return 0;
        }
        catch (bailed&) {
            gcx().error(context::generic, "bailing out");
            return 1;
        }
    }

    void build_command::create_prefix_ini()
    {
        const auto prefix = conf().path().prefix();

        // creating prefix
        if (!exists(prefix))
            op::create_directories(gcx(), prefix);

        const auto ini = prefix / default_ini_filename();
        if (!exists(ini)) {
            std::ofstream(ini) << "[paths]\n"
                               << "prefix = .\n";
        }
    }

}  // namespace mob
