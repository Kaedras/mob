#pragma once

#include "../../core/conf.h"
#include "../../core/op.h"
#include "../../net.h"
#include "../../tools/tools.h"
#include "../../utility.h"
#include <set>

namespace mob::tasks {
    // header containing linux-specific tasks

    class overlayfs : public basic_task<overlayfs> {
    public:
        overlayfs();

        static std::string version();
        static bool prebuilt();
        static fs::path source_path();

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
        void do_build_and_install() override;
    };

}  // namespace mob::tasks
