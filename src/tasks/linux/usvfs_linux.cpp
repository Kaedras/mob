#include "../tasks.h"

namespace mob::tasks {

    void usvfs::do_clean(clean c)
    {
        // delete the whole directory
        if (is_set(c, clean::reclone)) {
            git_wrap::delete_directory(cx(), source_path());

            // nothing more to do
            return;
        }

        if (is_set(c, clean::reconfigure)) {
            run_tool(create_cmake_tool());
        }

        if (is_set(c, clean::rebuild)) {
            run_tool(create_cmake_tool(cmake::clean));
        }
    }

    void usvfs::build_and_install_from_source()
    {
        run_tool(cmake(cmake::generate)
                     .generator(cmake::ninjaMultiConfig)
                     .def("CMAKE_INSTALL_PREFIX:PATH", conf().path().install())
                     .preset("linux")
                     .root(source_path()));

        run_tool(cmake(cmake::build).root(source_path()));

        run_tool(cmake(cmake::install).root(source_path()));
    }

    cmake usvfs::create_cmake_tool(cmake::ops o) const
    {
        return std::move(cmake(o)
                             .root(source_path())
                             .def("CMAKE_INSTALL_PREFIX:PATH", conf().path().install())
                             .generator(cmake::ninjaMultiConfig)
                             .preset("linux")
                             .arg("-DBUILD_TESTING=OFF"));
    }

}  // namespace mob::tasks
