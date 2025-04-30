#include "../pch.h"
#include "../fs.h"
#include "../../core/conf.h"
#include "../../core/context.h"

namespace mob {

    fs::path make_temp_file()
    {
        static fs::path dir = conf().path().temp_dir();

        char* name = std::tmpnam(nullptr);
        if (name == nullptr) {
            const auto e = GetLastError();

            gcx().bail_out(context::conf, "can't create temp file in {}, {}", dir,
                           error_message(e));
        }

        return dir / name;
    }

}  // namespace mob