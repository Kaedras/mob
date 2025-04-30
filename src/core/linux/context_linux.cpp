#include "../pch.h"
#include "../context.h"

namespace mob {

    // retrieves the error message from the system for the given id
    //
    std::string error_message(int id)
    {
        return strerror(id);
    }

}  // namespace mob