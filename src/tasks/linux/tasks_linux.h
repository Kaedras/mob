#pragma once

#include "../../core/conf.h"
#include "../../core/op.h"
#include "../../net.h"
#include "../../tools/tools.h"
#include "../../utility.h"
#include <set>

namespace mob::tasks {
    // header containing linux specific tasks

    class directxmath : public basic_task<directxmath> {
    public:
        directxmath();

        static std::string version();
        static bool prebuilt();
        static fs::path source_path();

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
        void do_build_and_install() override;
    };

    class directxheaders : public basic_task<directxheaders> {
    public:
        directxheaders();

        static std::string version();
        static bool prebuilt();
        static fs::path source_path();

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
        void do_build_and_install() override;
    };

}  // namespace mob::tasks
