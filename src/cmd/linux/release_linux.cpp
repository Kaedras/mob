#include "../../core/conf.h"
#include "../../core/context.h"
#include "../../core/op.h"
#include "../commands.h"

namespace mob {

    std::string release_command::version_from_exe() const
    {
#warning STUB!
        std::cout << "release_command::version_from_exe(): STUB\n";
        // const auto exe = conf().path().install_bin() / "ModOrganizer.exe";
        return "0.0.0";
    }

    void release_command::make_appimage()
    {
        const auto file = "ModOrganizer-x86_64.AppImage";
        const auto src  = conf().path().install_appimage() / file;
        const auto dest = out_;

        u8cout << "copying appimage " << file << "\n";

        op::copy_file_to_dir_if_better(gcx(), src, dest);
    }

}  // namespace mob
