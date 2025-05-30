#include "pch.h"
#include "../tools.h"

namespace mob {

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
