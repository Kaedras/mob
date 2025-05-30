#include "pch.h"
#include "../../core/conf.h"
#include "../commands.h"

namespace mob {
    void build_command::terminate_msbuild()
    {
        if (conf().global().dry())
            return;

        system("taskkill /im msbuild.exe /f > NUL 2>&1");
    }
}  // namespace mob
