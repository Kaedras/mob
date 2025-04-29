#include "../pch.h"
#include "../../core/process.h"
#include "../tools.h"

namespace mob {

    cmake::cmake(ops o)
        : basic_process_runner("cmake"), op_(o), gen_(ninja), arch_(arch::def)
    {
    }

    const std::map<cmake::generators, cmake::gen_info>& cmake::all_generators()
    {
        static const std::map<generators, gen_info> map = {
            {make, {"build", "Unix Makefiles", "", ""}},
            {ninja, {"build", "Ninja", "", ""}},
            {ninjaMultiConfig, {"build", "Ninja Multi-Config", "", ""}},
        };
        return map;
    }

}