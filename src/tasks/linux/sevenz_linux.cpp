#include "../pch.h"
#include "../tasks.h"
#include "../../core/process.h"

namespace mob::tasks {
    namespace {
        // 7z has a bunch of modules, like the gui, etc., just build the dll
        //
        fs::path module_to_build()
        {
            return sevenz::source_path() / "CPP" / "7zip" / "Bundles" / "Format7zF";
        }
    }  // namespace

    void sevenz::build()
    {
        std::string command  = "make -f makefile.gcc";
        unsigned int threads = std::thread::hardware_concurrency();
        if (threads > 1) {
            command += " -j" + std::to_string(threads);
        }

        auto p = process::raw(cx(), command);
        p.cwd(module_to_build());
        p.run_and_join();
    }
}  // namespace mob::tasks