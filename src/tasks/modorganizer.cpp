#include "pch.h"
#include "tasks.h"

namespace mob::tasks {

    // given a vector of names (some projects have more than one, see add_tasks() in
    // main.cpp), this prepends the simplified name to the vector and returns it
    //
    // most MO project names are something like "modorganizer-uibase" on github and
    // the simplified name is used for two main reasons:
    //
    //  1) individual directories in modorganizer_super have historically used the
    //     simplified name only
    //
    //  2) it's useful to have an simplified name for use on the command line
    //
    std::vector<std::string> make_names(std::vector<std::string> names)
    {
        // first name in the list might be a "modorganizer-something"
        const auto main_name = names[0];

        const auto dash = main_name.find("-");
        if (dash != std::string::npos) {
            // remove the part before the dash and the dash
            names.insert(names.begin(), main_name.substr(dash + 1));
        }

        return names;
    }

    // creates the repo in modorganizer_super, used to add submodules
    //
    // only one task will end up past the mutex and the flag, so it's only done
    // once
    //
    void initialize_super(context& cx, const fs::path& super_root)
    {
        static std::mutex mutex;
        static bool initialized = false;

        std::scoped_lock lock(mutex);
        if (initialized)
            return;

        initialized = true;

        cx.trace(context::generic, "checking super");

        git_wrap g(super_root);

        // happens when running mob again in the same build tree
        if (g.is_git_repo()) {
            cx.debug(context::generic, "super already initialized");
            return;
        }

        // create empty repo
        cx.trace(context::generic, "initializing super");
        g.init_repo();
    }

    modorganizer::modorganizer(std::string long_name, flags f)
        : modorganizer(std::vector<std::string>{long_name}, f)
    {
    }

    modorganizer::modorganizer(std::vector<const char*> names, flags f)
        : modorganizer(std::vector<std::string>(names.begin(), names.end()), f)
    {
    }

    modorganizer::modorganizer(std::vector<std::string> names, flags f)
        : task(make_names(names)), repo_(names[0]), flags_(f)
    {
        if (names.size() > 1) {
            project_ = names[1];
        }
        else {
            project_ = make_names(names)[0];
        }
    }

    bool modorganizer::is_gamebryo_plugin() const
    {
        return is_set(flags_, gamebryo);
    }

    bool modorganizer::is_nuget_plugin() const
    {
        return is_set(flags_, nuget);
    }

    fs::path modorganizer::source_path() const
    {
        // something like build/modorganizer_super/uibase
        return super_path() / name();
    }

    fs::path modorganizer::super_path()
    {
        return conf().path().build() / "modorganizer_super";
    }

    url modorganizer::git_url() const
    {
        return make_git_url(task_conf().mo_org(), repo_);
    }

    std::string modorganizer::org() const
    {
        return task_conf().mo_org();
    }

    std::string modorganizer::repo() const
    {
        return repo_;
    }

    void modorganizer::do_clean(clean c)
    {
        // delete the whole directory
        if (is_set(c, clean::reclone)) {
            git_wrap::delete_directory(cx(), source_path());

            // no need to do anything else
            return;
        }

        // cmake clean
        if (is_set(c, clean::reconfigure))
            run_tool(create_cmake_tool(cmake::clean));
    }

    void modorganizer::do_fetch()
    {
        // make sure the super directory is initialized, only done once
        initialize_super(cx(), super_path());

        // find the best suitable branch
        const auto fallback = task_conf().mo_fallback_branch();
        auto branch         = task_conf().mo_branch();
        if (!fallback.empty() && !git_wrap::remote_branch_exists(git_url(), branch)) {
            cx().warning(context::generic,
                         "{} has no remote {} branch, switching to {}", repo_, branch,
                         fallback);
            branch = fallback;
        }

        // clone/pull
        run_tool(make_git().url(git_url()).branch(branch).root(source_path()));
    }

    cmake modorganizer::create_cmake_tool(cmake::ops o)
    {
        return create_cmake_tool(source_path(), o, task_conf().configuration());
    }

}  // namespace mob::tasks
