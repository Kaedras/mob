#include "../pch.h"
#include "../tools.h"

namespace mob {

    cmake::cmake(ops o)
        : basic_process_runner("cmake"), op_(o), gen_(default_generator),
          arch_(arch::dont_care), config_(config::release)
    {
    }

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