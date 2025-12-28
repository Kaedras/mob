#pragma once

#include "../../net.h"
#include "../task.h"

namespace mob::tasks {
    // header containing linux-specific tasks

    class appimage : public basic_task<appimage> {
    public:
        appimage();

        static std::string version();
        static bool prebuilt();

        static fs::path source_path();

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
        void do_build_and_install() override;
    };

}  // namespace mob::tasks
