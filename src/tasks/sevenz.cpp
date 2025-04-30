#include "pch.h"
#include "tasks.h"

#ifdef __unix__
static constexpr std::string SEVENZ_OUTPUT_PATH = "_o";
static constexpr std::string LIB_EXT = "so";
#else
static constexpr std::string SEVENZ_OUTPUT_PATH = "x64";
static constexpr std::string LIB_EXT = "dll";
#endif

namespace mob::tasks {

    namespace {

        std::string version_for_url()
        {
            return replace_all(sevenz::version(), ".", "");
        }

        url source_url()
        {
            return "https://www.7-zip.org/a/7z" + version_for_url() + "-src.7z";
        }

        // 7z has a bunch of modules, like the gui, etc., just build the dll
        //
        fs::path module_to_build()
        {
            return sevenz::source_path() / "CPP" / "7zip" / "Bundles" / "Format7zF";
        }

    }  // namespace

    sevenz::sevenz() : basic_task("7z", "sevenz") {}

    std::string sevenz::version()
    {
        return conf().version().get("sevenz");
    }

    bool sevenz::prebuilt()
    {
        return false;
    }

    fs::path sevenz::source_path()
    {
        return conf().path().build() / ("7zip-" + version());
    }

    void sevenz::do_clean(clean c)
    {
        // delete download
        if (is_set(c, clean::redownload))
            run_tool(downloader(source_url(), downloader::clean));

        // delete whole directory
        if (is_set(c, clean::reextract)) {
            cx().trace(context::reextract, "deleting {}", source_path());
            op::delete_directory(cx(), source_path(), op::optional);

            // no need to do anything else
            return;
        }

        // delete whole output directory
        if (is_set(c, clean::rebuild))
            op::delete_directory(cx(), module_to_build() / SEVENZ_OUTPUT_PATH, op::optional);
    }

    void sevenz::do_fetch()
    {
        const auto file = run_tool(downloader(source_url()));

        run_tool(extractor().file(file).output(source_path()));
    }

    void sevenz::do_build_and_install()
    {
        build();

        // copy 7z.dll to install/bin/dlls
        op::copy_file_to_dir_if_better(cx(), module_to_build() / SEVENZ_OUTPUT_PATH / ("7z." + LIB_EXT),
                                       conf().path().install_dlls());
    }
}  // namespace mob::tasks
