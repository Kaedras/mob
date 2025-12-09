cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE source_files *.cpp)
file(GLOB_RECURSE header_files *.h)

# exclude os specific files
if (UNIX)
    set(EXCLUDE_FILTER "${PROJECT_SOURCE_DIR}.*/win32/")
else ()
    set(EXCLUDE_FILTER "${PROJECT_SOURCE_DIR}.*/linux/")
endif ()
list(FILTER source_files EXCLUDE REGEX ${EXCLUDE_FILTER})
list(FILTER header_files EXCLUDE REGEX ${EXCLUDE_FILTER})

add_executable(mob ${source_files} ${header_files})
set_target_properties(mob PROPERTIES CXX_STANDARD 20)
target_precompile_headers(mob PRIVATE pch.h)

target_include_directories(mob PUBLIC ${CMAKE_SOURCE_DIR}/third-party/include)
if (UNIX)
    option(ENABLE_ASAN "Enable address sanitizer" ON)
    find_package(ZLIB REQUIRED)
    find_package(CURL REQUIRED)
    if(ENABLE_ASAN)
        target_link_libraries(mob PUBLIC asan)
        target_compile_options(mob PRIVATE -fno-omit-frame-pointer -fsanitize=address)
    endif()
    target_link_libraries(mob PUBLIC curl z)
else ()
    target_compile_definitions(mob PUBLIC NOMINMAX)
    target_compile_options(mob PUBLIC "/MT")
    target_link_libraries(mob PUBLIC
        wsock32 ws2_32 crypt32 wldap32 dbghelp shlwapi version
        optimized ${CMAKE_SOURCE_DIR}/third-party/lib/libcurl.lib
        debug ${CMAKE_SOURCE_DIR}/third-party/lib/libcurl-d.lib
        optimized ${CMAKE_SOURCE_DIR}/third-party/lib/zlib.lib
        debug ${CMAKE_SOURCE_DIR}/third-party/lib/zlibd.lib)
endif ()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}
    PREFIX src
    FILES ${source_files} ${header_files})
if (WIN32)
    set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT mob)
endif ()
