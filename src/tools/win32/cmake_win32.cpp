#include "../pch.h"
#include "../tools.h"

namespace mob {

    cmake::cmake(ops o)
        : basic_process_runner("cmake"), op_(o), gen_(jom), arch_(arch::def),
          config_(config::release)
    {
    }

    const std::map<cmake::generators, cmake::gen_info>& cmake::all_generators()
    {
        static const std::map<generators, gen_info> map = {
            // jom doesn't need -A for architectures
            {generators::jom, {"build", "NMake Makefiles JOM", "", ""}},

            {generators::vs,
             {"vsbuild", "Visual Studio " + vs::version() + " " + vs::year(), "Win32",
              "x64"}}};
        return map;
    }

}  // namespace mob