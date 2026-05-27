file(GLOB_RECURSE source_files CONFIGURE_DEPENDS "*.cpp")
file(GLOB_RECURSE header_files CONFIGURE_DEPENDS "*.h")

# exclude os specific files
if (UNIX)
    set(EXCLUDE_FILTER "${PROJECT_SOURCE_DIR}.*/win32/")
else ()
    set(EXCLUDE_FILTER "${PROJECT_SOURCE_DIR}.*/linux/")
endif ()
list(FILTER source_files EXCLUDE REGEX ${EXCLUDE_FILTER})
list(FILTER header_files EXCLUDE REGEX ${EXCLUDE_FILTER})

add_executable(mob ${source_files} ${header_files})
target_precompile_headers(mob PRIVATE pch.h)

if(WIN32)
    target_compile_definitions(
      mob PRIVATE _WIN32_WINNT=0x0A00 NTDDI_VERSION=0x0A000007 WIN32_LEAN_AND_MEAN
                  NOMINMAX NOCOMM)

    target_link_libraries(mob PRIVATE dbghelp shlwapi version)
endif()

target_link_libraries(mob PRIVATE clipp::clipp nlohmann_json::nlohmann_json
        CURL::libcurl)

source_group(
  TREE ${CMAKE_CURRENT_SOURCE_DIR}
  PREFIX src
  FILES ${source_files} ${header_files})
