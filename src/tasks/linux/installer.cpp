#include "../tasks.h"

namespace mob::tasks {

    installer::installer() : basic_task("installer") {}

    bool installer::prebuilt()
    {
        return false;
    }

    std::string installer::version()
    {
        return {};
    }

    fs::path installer::source_path()
    {
        return modorganizer::super_path() / "installer";
    }

    void installer::do_clean(clean)
    {
        // no-op
    }

    void installer::do_fetch()
    {
        // no-op
    }

    void installer::do_build_and_install()
    {
        // no-op
    }

}  // namespace mob::tasks
