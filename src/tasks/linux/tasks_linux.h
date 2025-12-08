#pragma once

#include "../../net.h"
#include "../task.h"

namespace mob::tasks {
    // header containing linux-specific tasks

    class overlayfs : public basic_task<overlayfs> {
    public:
        // build CMAKE_PREFIX_PATH for MO2 tasks
        //
        static std::string cmake_prefix_path();

        overlayfs();

        static std::string version();
        static bool prebuilt();

        // url to the git repo
        //
        url git_url() const;

        // `mo_org` setting from the ini (typically ModOrganizer2)
        //
        std::string org() const;

        // name of the repo on github (first name given in the constructor,
        // something like "cmake_common" or "modorganizer-uibase")
        //
        std::string repo() const;

        static fs::path source_path();

    protected:
        void do_clean(clean c) override;
        void do_fetch() override;
        void do_build_and_install() override;

    private:
        std::string repo_;
        std::string project_;
    };

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
