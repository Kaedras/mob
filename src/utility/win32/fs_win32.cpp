#include "../pch.h"
#include "../fs.h"
#include "../../core/conf.h"
#include "../../core/context.h"

namespace mob {

    fs::path make_temp_file()
    {
        static fs::path dir = conf().path().temp_dir();

        wchar_t name[MAX_PATH + 1] = {};
        if (GetTempFileNameW(dir.native().c_str(), L"mob", 0, name) == 0) {
            const auto e = GetLastError();

            gcx().bail_out(context::conf, "can't create temp file in {}, {}", dir,
                           error_message(e));
        }

        return dir / name;
    }

}  // namespace mob