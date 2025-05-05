#include "../pch.h"
#include "../tasks.h"

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
        build_loop(cx(), [&](bool mp) {
            const int exit_code = run_tool(nmake()
                                               .path(module_to_build())
                                               .def("CPU=x64")
                                               .def("NEW_COMPILER=1")
                                               .def("MY_STATIC_LINK=1")
                                               .def("NO_BUFFEROVERFLOWU=1"));

            return (exit_code == 0);
        });
    }
}  // namespace mob::tasks