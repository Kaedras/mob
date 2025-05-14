#include "../pch.h"
#include "../fs.h"
#include "../../core/conf.h"
#include "../../core/context.h"

namespace mob {

    fs::path make_temp_file()
    {
        static fs::path dir = conf().path().temp_dir();

        std::string templ = dir / "tmpXXXXXX";

        // mkstemp requires a modifiable char array
        char filename[templ.length() + 1];
        strcpy(filename, templ.c_str());

        int fd = mkstemp(filename);

        if (fd == -1) {
            const auto e = errno;

            gcx().bail_out(context::conf, "can't create temp file in {}, {}", dir,
                           error_message(e));
        }

        // Close the file descriptor since we only need the name
        close(fd);

        return std::filesystem::path(filename);
    }

}  // namespace mob