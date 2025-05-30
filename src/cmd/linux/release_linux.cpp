#include "pch.h"
#include "../commands.h"

namespace mob {
    std::string release_command::version_from_exe() const
    {
        std::cout << "release_command::version_from_exe(): STUB\n";
        // const auto exe = conf().path().install_bin() / "ModOrganizer.exe";
        return "9.9.9";
    }

}  // namespace mob
