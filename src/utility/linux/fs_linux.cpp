#include "../../core/conf.h"
#include "../../core/context.h"
#include "../fs.h"

using namespace std;

namespace mob {

    fs::path make_temp_file()
    {
        static fs::path dir = conf().path().temp_dir();

        const string templateString = dir / "tmpXXXXXX";

        // mkstemp requires a modifiable char array
        const auto path = make_unique<char[]>(templateString.length() + 1);
        strcpy(path.get(), templateString.c_str());

        const int fd = mkstemp(path.get());

        if (fd == -1) {
            const auto e = errno;

            gcx().bail_out(context::conf, "can't create temp file in {}, {}", dir,
                           error_message(e));
        }

        // Close the file descriptor since we only need the name
        close(fd);

        return std::filesystem::path(path.get());
    }

}  // namespace mob
