#pragma once

#include "../../tools/tools.h"
#include "../../utility.h"

namespace mob::tasks {

    // header containing windows-specific tasks

    class explorerpp : public basic_task<explorerpp> {
    public:
        explorerpp();

        static std::string version();
        static bool prebuilt();
        static fs::path source_path();

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
    };

}  // namespace mob::tasks
