#include "tasks.h"

#ifdef __unix__
static inline constexpr auto repoName = "usvfs-fuse";
#else
static inline constexpr auto repoName = "usvfs";
#endif

namespace mob::tasks {

    usvfs::usvfs() : basic_task("usvfs") {}

    std::string usvfs::version()
    {
        return conf().version().get("usvfs");
    }

    bool usvfs::prebuilt()
    {
        return false;
    }

    fs::path usvfs::source_path()
    {
        return conf().path().build() / "usvfs";
    }

    void usvfs::do_fetch()
    {
        fetch_from_source();
    }

    void usvfs::do_build_and_install()
    {
        build_and_install_from_source();
    }

    void usvfs::fetch_from_source()
    {
        run_tool(make_git()
                     .url(make_git_url(task_conf().mo_org(), repoName))
                     .branch(version())
                     .root(source_path()));
    }

}  // namespace mob::tasks
