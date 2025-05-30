#include "../pch.h"
#include "../tools.h"

namespace mob {

    const std::map<cmake::generators, cmake::gen_info>& cmake::all_generators()
    {
        static const std::map<generators, gen_info> map = {
            {default_generator, {"build", "", "", ""}},
            {make, {"build", "Unix Makefiles", "", ""}},
            {ninja, {"build", "Ninja", "", ""}},
            {ninjaMultiConfig, {"build", "Ninja Multi-Config", "", ""}},
        };
        return map;
    }

}  // namespace mob