#include "../tasks.h"

#include <sys/stat.h>

namespace mob::tasks {

    namespace {

        url source_url()
        {
            return "https://github.com/linuxdeploy/linuxdeploy/releases/download/" +
                   conf().version().get("linuxdeploy") + "/linuxdeploy-x86_64.AppImage";
        }

    }  // namespace

    appimage::appimage() : basic_task("appimage") {}

    std::string appimage::version()
    {
        return {};
    }

    bool appimage::prebuilt()
    {
        return true;
    }

    fs::path appimage::source_path()
    {
        return modorganizer::super_path() / "appimage";
    }

    void appimage::do_clean(clean c)
    {
        op::delete_directory(cx(), conf().path().install_appimage());
    }

    void appimage::do_fetch()
    {
        // fetch linuxdeploy-x86_64.AppImage
        const auto file = run_tool(downloader(source_url()));

        // chmod u+x
        struct stat st{};
        if (stat(file.c_str(), &st) == -1) {
            const int e = errno;
            cx().bail_out(context::reason::cmd, "stat() failed: {}", strerror(e));
        }
        if (chmod(file.c_str(), st.st_mode | S_IXUSR) == -1) {
            const int e = errno;
            cx().bail_out(context::reason::cmd, "chmod() failed: {}", strerror(e));
        }

        // copy to mob prefix dir
        op::copy_file_to_dir_if_better(cx(), file, conf().path().prefix());
    }

    void appimage::do_build_and_install()
    {
        run_tool(linuxdeploy());
    }

}  // namespace mob::tasks
