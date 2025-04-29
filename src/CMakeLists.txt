cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE source_files *.cpp)
file(GLOB_RECURSE header_files *.h)

# exclude os specific files
if (UNIX)
    set(EXCLUDE_FILTER "win32")
else ()
    set(EXCLUDE_FILTER "linux")
endif ()
list(FILTER source_files EXCLUDE REGEX ${EXCLUDE_FILTER})
list(FILTER header_files EXCLUDE REGEX ${EXCLUDE_FILTER})

add_executable(mob ${source_files} ${header_files})
set_target_properties(mob PROPERTIES CXX_STANDARD 20)

target_include_directories(mob PUBLIC ${CMAKE_SOURCE_DIR}/third-party/include)
if(UNIX)
    find_package(ZLIB)
    find_package(CURL)
    target_link_libraries(mob PUBLIC curl z)
else()
    target_compile_definitions(mob PUBLIC NOMINMAX)
    target_compile_options(mob PUBLIC "/MT")
    target_link_libraries(mob PUBLIC
        wsock32 ws2_32 crypt32 wldap32 dbghelp shlwapi version
        optimized ${CMAKE_SOURCE_DIR}/third-party/lib/libcurl.lib
        debug ${CMAKE_SOURCE_DIR}/third-party/lib/libcurl-d.lib
        optimized ${CMAKE_SOURCE_DIR}/third-party/lib/zlib.lib
        debug ${CMAKE_SOURCE_DIR}/third-party/lib/zlibd.lib)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}
    PREFIX src
    FILES ${source_files} ${header_files})
if(WIN32)
    set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT mob)
endif()