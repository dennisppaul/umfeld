cmake_minimum_required(VERSION 3.20)

project(umfeld-lib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "----------------------------------------")
message(STATUS "UMFELD LIBRARY")
message(STATUS "----------------------------------------")

# TODO devide for either 'disable' or 'enable' options
option(ENABLE_PORTAUDIO "Enable PortAudio output" ON)
option(ENABLE_ANGLE "Enable ANGLE OpenGL ES emulation on macOS ( apple silicon )" OFF)
option(DISABLE_PDF "Disable PDF export ( cairo )" OFF)
option(DISABLE_NCURSES "Disable ncurses support" OFF)
option(DISABLE_VIDEO "Disable video ( ffmpeg ) support" OFF)
option(DISABLE_AUDIO "Disable audio ( SDL3 + portaudio ) support" OFF)

# TODO add more options
#option(DISABLE_OPENGL "Disable OpenGL support" OFF)
#option(DISABLE_GRAPHICS "Disable graphics support" OFF) # same as DISABLE_OPENGL?
#option(DISABLE_FONT "Disable font rendering ( harfbuzz + freetype ) support" OFF)
#option(DISABLE_MAIN "Disable main function" OFF)

##########################################################################################################
### CHECK FOR HOMEBREW                                                                                 ###
##########################################################################################################
find_program(BREW_FOUND brew)
if (APPLE)
    if (BREW_FOUND)
        execute_process(COMMAND brew --prefix
                OUTPUT_VARIABLE HOMEBREW_PREFIX
                OUTPUT_STRIP_TRAILING_WHITESPACE)
        set(HOMEBREW_LIB_PATH "${HOMEBREW_PREFIX}/lib")
        message(STATUS "HOMEBREW LIBS : ${HOMEBREW_LIB_PATH}")
    else ()
        message(WARNING "HOMEBREW not found.")
    endif ()
endif (APPLE)

##########################################################################################################
### CHECK FOR MACPORTS                                                                                 ###
##########################################################################################################
if (APPLE AND NOT BREW_FOUND)
    if (EXISTS "/opt/local/bin/port")
        message(STATUS "MACPORTS found, setting paths.")
        # 1. MacPorts 라이브러리 경로를 link_directories에 추가
        link_directories("/opt/local/lib")

        # 2. pkg-config가 MacPorts 라이브러리를 찾을 수 있도록 환경 변수 설정
        set(ENV{PKG_CONFIG_PATH} "/opt/local/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")

        # 참고: MacPorts 헤더 파일 경로 추가
        include_directories("/opt/local/include")
    else()
        message(WARNING "Neither HOMEBREW nor MACPORTS found. Build may fail.")
    endif()
endif(APPLE AND NOT BREW_FOUND)

##########################################################################################################
### SUPPLEMENT LIBRARY SEARCH PATH                                                                     ###
##########################################################################################################

# NOTE this does not work when executed after `add_executable` or `add_library` … which is exactly what happens :(
# if (APPLE)
#     # NOTE add default search path for libraries on macOS e.g for default Homebrew installation
#     link_directories("/usr/local/lib")
#     link_directories("/opt/homebrew/lib")
#     message(STATUS "GLOBAL LIB    : adding `/usr/local/lib` + `/opt/homebrew/lib` to library search path")
# elseif (UNIX)
# elseif (WIN32)
# else ()
# endif ()

##########################################################################################################
### ADD SOURCE + HEADER FILES                                                                          ###
##########################################################################################################

# Add library target
if (APPLE)
    file(GLOB UMFELD_SOURCE_FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.mm
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/*.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/libtess2/*.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/clipper/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/ip/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/ip/posix/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/osc/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/pffft/*.c
    )
elseif (UNIX)
    file(GLOB UMFELD_SOURCE_FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/*.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/libtess2/*.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/clipper/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/ip/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/ip/posix/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/osc/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/pffft/*.c
    )
elseif (WIN32)
    file(GLOB UMFELD_SOURCE_FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/*.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/libtess2/*.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/clipper/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/ip/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/ip/win32/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/osc/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/pffft/*.c
    )
else ()
    file(GLOB UMFELD_SOURCE_FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/*.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/libtess2/*.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/clipper/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/ip/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/ip/posix/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/osc/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/libraries/pffft/*.c
    )
endif ()

if (NOT TARGET umfeld-lib)
    add_library(umfeld-lib ${UMFELD_SOURCE_FILES})
endif ()

# Specify include directories for the library
target_include_directories(umfeld-lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/shaders
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libraries
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libraries/dr_libs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libraries/ip
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libraries/libtess2
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libraries/osc
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libraries/pffft
)

##########################################################################################################
### GLOBAL VARIABLES + EN/DISABLE LIBRARIES                                                            ###
##########################################################################################################

if (NOT TARGET umfeld-lib-interface)
    add_library(umfeld-lib-interface INTERFACE) # this enables `glm::aligned_vec*` types
endif ()

# application name

if (NOT DEFINED UMFELD_APP_NAME)
    set(UMFELD_APP_NAME "${CMAKE_PROJECT_NAME}")
endif ()
target_compile_definitions(${PROJECT_NAME} PUBLIC UMFELD_APP_NAME=\"${UMFELD_APP_NAME}\")
message(STATUS "APP NAME      : ${UMFELD_APP_NAME}")

if (NOT DISABLE_MAIN)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SDL_MAIN_USE_CALLBACKS)
else ()
    message(STATUS "DISABLE_MAIN  : no `main` function entry point. must be defined in project.")
endif (NOT DISABLE_MAIN)

if (DISABLE_GRAPHICS)
    target_compile_definitions(umfeld-lib-interface INTERFACE DISABLE_GRAPHICS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC DISABLE_GRAPHICS)
    message(STATUS "DISABLING     : graphics")
endif (DISABLE_GRAPHICS)

if (DISABLE_AUDIO)
    target_compile_definitions(umfeld-lib-interface INTERFACE DISABLE_AUDIO)
    target_compile_definitions(${PROJECT_NAME} PUBLIC DISABLE_AUDIO)
    message(STATUS "DISABLING     : audio")
endif (DISABLE_AUDIO)

if (DISABLE_VIDEO)
    target_compile_definitions(umfeld-lib-interface INTERFACE DISABLE_VIDEO)
    target_compile_definitions(${PROJECT_NAME} PUBLIC DISABLE_VIDEO)
    message(STATUS "DISABLING     : video")
endif (DISABLE_VIDEO)

if (DISABLE_PDF)
    target_compile_definitions(umfeld-lib-interface INTERFACE DISABLE_PDF)
    target_compile_definitions(${PROJECT_NAME} PUBLIC DISABLE_PDF)
    message(STATUS "DISABLING     : pdf")
endif (DISABLE_PDF)

if (ENABLE_PORTAUDIO)
    target_compile_definitions(umfeld-lib-interface INTERFACE ENABLE_PORTAUDIO)
    target_compile_definitions(${PROJECT_NAME} PUBLIC ENABLE_PORTAUDIO)
    message(STATUS "ENABLING      : PortAudio Driver")
endif (ENABLE_PORTAUDIO)

if (ENABLE_CAPTURE)
    target_compile_definitions(umfeld-lib-interface INTERFACE ENABLE_CAPTURE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC ENABLE_CAPTURE)
    message(STATUS "ENABLING      : Capture")
endif (ENABLE_CAPTURE)

##########################################################################################################
### CHECK SYSTEM TYPE                                                                                  ###
##########################################################################################################

if (APPLE)
    message(STATUS "SYSTEM        : macOS")
    target_compile_definitions(umfeld-lib-interface INTERFACE SYSTEM_MACOS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SYSTEM_MACOS)
elseif (UNIX)
    # default to UNIX
    set(SYSTEM_TYPE "UNIX")
    set(SYSTEM_DEFINE SYSTEM_UNIX)
    # try to detect Raspberry Pi
    if (EXISTS "/proc/device-tree/model")
        file(READ "/proc/device-tree/model" DEVICE_MODEL)
        string(TOLOWER "${DEVICE_MODEL}" DEVICE_MODEL_LOWER)
        if (DEVICE_MODEL_LOWER MATCHES "raspberry")
            set(RASPBERRY_PI TRUE)
        endif ()
    endif ()
    if (RASPBERRY_PI)
        message(STATUS "SYSTEM        : ${SYSTEM_TYPE} ( on Raspberry Pi )")
    else ()
        message(STATUS "SYSTEM        : ${SYSTEM_TYPE}")
    endif ()
    target_compile_definitions(umfeld-lib-interface INTERFACE ${SYSTEM_DEFINE})
    target_compile_definitions(${PROJECT_NAME} PUBLIC ${SYSTEM_DEFINE})
elseif (WIN32)
    message(STATUS "SYSTEM        : Windows")
    target_compile_definitions(umfeld-lib-interface INTERFACE SYSTEM_WINDOWS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SYSTEM_WINDOWS)
else ()
    message(STATUS "SYSTEM        : Undefined, Good Luck!")
    target_compile_definitions(umfeld-lib-interface INTERFACE SYSTEM_UNDEFINED)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SYSTEM_UNDEFINED)
endif ()

##########################################################################################################
### OPEN_GL VERSION + GLAD                                                                             ###
##########################################################################################################

if (DEFINED SYSTEM_DEFINE AND RASPBERRY_PI)
    set(UMFELD_DEFAULT_OPENGL_VERSION "OPENGL_ES_3_0" CACHE STRING "OpenGL version used in umfeld")
else ()
    set(UMFELD_DEFAULT_OPENGL_VERSION "OPENGL_3_3_CORE" CACHE STRING "OpenGL version used in umfeld")
endif ()
# NOTE auto-generated files with GLAD 2 ( on macOS ):
#    brew install glad2
#    glad --api="gl:core=3.3" --out-path=external/glad_opengl_33 c
#    glad --api="gl:compatibility=2.0" --out-path=external/glad_opengl_20 c
#    glad --api="gles2=3.0" --out-path=external/glad_opengles_30 c
# TODO maybe move this into library folder(s)?
set(UMFELD_GLAD_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external" CACHE STRING "GLAD base directory")

# if OpenGL not set use default
if (NOT UMFELD_OPENGL_VERSION)
    set(UMFELD_OPENGL_VERSION "${UMFELD_DEFAULT_OPENGL_VERSION}")
    message(STATUS "OPEN_GL(app)  : OPENGL_VERSION not set, using default: ${UMFELD_DEFAULT_OPENGL_VERSION}")
else ()
    message(STATUS "OPEN_GL(app)  : ${UMFELD_OPENGL_VERSION}")
endif ()

if (UMFELD_OPENGL_VERSION STREQUAL "OPENGL_3_3_CORE" OR UMFELD_OPENGL_VERSION STREQUAL "3.3")
    set(OPENGL_3_3_CORE TRUE)
elseif (UMFELD_OPENGL_VERSION STREQUAL "OPENGL_2_0")
    set(OPENGL_2_0 TRUE)
elseif (UMFELD_OPENGL_VERSION STREQUAL "OPENGL_ES_3_0" OR UMFELD_OPENGL_VERSION STREQUAL "OPENGL_ES_3_0")
    set(OPENGL_ES_3_0 TRUE)
else ()
    message(STATUS "Invalid OPENGL_VERSION: ${UMFELD_OPENGL_VERSION} must be `OPENGL_3_3_CORE`, `OPENGL_2_0`, or `OPENGL_ES_3_0`")
    message(STATUS "using default: ${UMFELD_DEFAULT_OPENGL_VERSION}")
    set(OPENGL_3_3_CORE TRUE)
endif ()

# set GLAD path and define
if (OPENGL_3_3_CORE)
    target_compile_definitions(umfeld-lib-interface INTERFACE OPENGL_3_3_CORE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OPENGL_3_3_CORE)
    set(GLAD_DIR "${UMFELD_GLAD_BASE_DIR}/glad_opengl_33")
    message(STATUS "OPEN_GL       : OPENGL_3_3_CORE")
elseif (OPENGL_2_0)
    target_compile_definitions(umfeld-lib-interface INTERFACE OPENGL_2_0)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OPENGL_2_0)
    set(GLAD_DIR "${UMFELD_GLAD_BASE_DIR}/glad_opengl_20")
    message(STATUS "OPEN_GL       : OPENGL_2_0")
elseif (OPENGL_ES_3_0)
    target_compile_definitions(umfeld-lib-interface INTERFACE OPENGL_ES_3_0)
    target_compile_definitions(${PROJECT_NAME} PUBLIC OPENGL_ES_3_0)
    set(GLAD_DIR "${UMFELD_GLAD_BASE_DIR}/glad_opengles_30")
    message(STATUS "OPEN_GL       : OPENGL_ES_3_0")
endif ()

message(STATUS "GLAD_DIR      : ${GLAD_DIR}")

# add GLAD as library
file(GLOB GLAD_SRC "${GLAD_DIR}/../*.c")
add_library(glad STATIC ${GLAD_SRC})
if (OPENGL_3_3_CORE)
    target_compile_definitions(glad PRIVATE OPENGL_3_3_CORE)
elseif (OPENGL_2_0)
    target_compile_definitions(glad PRIVATE OPENGL_2_0)
elseif (OPENGL_ES_3_0)
    target_compile_definitions(glad PRIVATE OPENGL_ES_3_0)
else ()
    message(WARNING "OPENGL_VERSION not defined. for GLAD to work it must be either OPENGL_2_0, OPENGL_ES_3_0 or OPENGL_3_3_CORE.")
endif ()
target_include_directories(glad PUBLIC "${GLAD_DIR}/include")

##########################################################################################################
### CREATE EXECUTABLE                                                                                  ###
##########################################################################################################

#if (BUILD_STANDALONE)
#    add_executable(${PROJECT_NAME} ${UMFELD_SOURCE_FILES})
#    target_compile_definitions(${PROJECT_NAME} PUBLIC UMFELD_STANDALONE)
#else (BUILD_STANDALONE)
target_sources(${PROJECT_NAME}
        PUBLIC
        ${UMFELD_SOURCE_FILES}
)
#endif (BUILD_STANDALONE)

##########################################################################################################
### SUPRESS WARNINGS FOR APPLE                                                                         ###
##########################################################################################################

if (APPLE)
    set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON)
endif (APPLE)

##########################################################################################################
### FIND PACKAGES + LIBRARIES                                                                          ###
##########################################################################################################

find_package(PkgConfig REQUIRED)
if (WIN32)
elseif (APPLE)
elseif (UNIX)
    find_package(Threads REQUIRED)
else ()
endif ()

# GRAPHICS + VIDEO
if (NOT DISABLE_GRAPHICS)
    # OpenGL
    find_package(glm REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(SDL3 CONFIG REQUIRED)

    # font rendering
    find_package(harfbuzz CONFIG REQUIRED)
    pkg_check_modules(FREETYPE REQUIRED freetype2)

    # PDF export
    if (NOT DISABLE_PDF)
        message(STATUS "PDF_EXPORT    : ENABLED")
        pkg_check_modules(CAIRO REQUIRED cairo)
    else (NOT DISABLE_PDF)
        message(STATUS "PDF_EXPORT    : DISABLED")
    endif (NOT DISABLE_PDF)

    # Ncurses
    if (NOT DISABLE_NCURSES)
        message(STATUS "NCURSES(TERM) : ENABLED")
        # find_package(Curses REQUIRED)
        # pkg_check_modules(NCURSES REQUIRED ncurses)
        pkg_check_modules(NCURSES_PKG ncurses)

        if (NCURSES_PKG_FOUND)
            message(STATUS "              : Using pkg-config to find ncurses")
            set(NCURSES_INCLUDE_DIR ${NCURSES_PKG_INCLUDE_DIRS})
            set(NCURSES_LIBRARY ${NCURSES_PKG_LIBRARIES})
        else ()
            message(STATUS "              : Using find_package to find ncurses")
            find_package(Curses REQUIRED)
            set(NCURSES_INCLUDE_DIR ${CURSES_INCLUDE_DIR})
            set(NCURSES_LIBRARY ${CURSES_LIBRARIES})
        endif ()
        target_include_directories(${PROJECT_NAME} PRIVATE ${NCURSES_INCLUDE_DIR})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${NCURSES_LIBRARY})
    else (NOT DISABLE_NCURSES)
        message(STATUS "NCURSES(TERM) : DISABLED")
    endif (NOT DISABLE_NCURSES)

    # TODO check if ncurses needs extra treatment on other platforms
    #if (UNIX)
    #    find_package(Curses REQUIRED)
    #    include_directories(${CURSES_INCLUDE_DIR})
    #    target_link_libraries(${PROJECT_NAME} PUBLIC ${CURSES_LIBRARIES})
    #endif (UNIX)
    #if (WIN32)
    #    target_link_libraries(${PROJECT_NAME} PUBLIC pdcurses)
    #endif (WIN32)

    # curl
    pkg_check_modules(CURL REQUIRED libcurl)
    #    find_package(CURL REQUIRED)

    if (APPLE)
    elseif (UNIX)
    elseif (WIN32)
    else ()
        message(STATUS "platform may not be supported.")
    endif ()

    # FFMPEG
    if (NOT DISABLE_VIDEO)
        pkg_check_modules(FFMPEG REQUIRED
                libavcodec
                libavformat
                libavutil
                libswscale
                libswresample
                libavdevice
        )
        if (APPLE)
        elseif (UNIX)
        elseif (WIN32)
        else ()
            message(STATUS "platform may not be supported ... assuming windows.")
        endif ()
    endif (NOT DISABLE_VIDEO)
endif (NOT DISABLE_GRAPHICS)

# AUDIO
if (NOT DISABLE_AUDIO)
    if (ENABLE_PORTAUDIO)
        pkg_search_module(PORTAUDIO REQUIRED portaudio-2.0)
    else (ENABLE_PORTAUDIO)
        find_package(SDL3 CONFIG REQUIRED)
    endif (ENABLE_PORTAUDIO)
endif (NOT DISABLE_AUDIO)

# MIDI
if (NOT DISABLE_MIDI)
    pkg_search_module(RTMIDI REQUIRED rtmidi)
endif (NOT DISABLE_MIDI)

##########################################################################################################
### SUPRESS WARNINGS FOR APPLE                                                                         ###
##########################################################################################################

if (WIN32)
elseif (APPLE)
    set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS OFF)
elseif (UNIX)
else ()
endif ()

##########################################################################################################
### INCLUDE + LINK LIBRARIES                                                                           ###
##########################################################################################################

if (HOMEBREW_LIB_PATH)
    target_link_directories(${PROJECT_NAME} PUBLIC ${HOMEBREW_LIB_PATH})
endif ()

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC wsock32 ws2_32 winmm)
elseif (APPLE)
elseif (UNIX)
    target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)
else ()
endif ()

# GRAPHICS + VIDEO + AUDIO
if (NOT DISABLE_AUDIO)
    if (ENABLE_PORTAUDIO)
        target_link_libraries(${PROJECT_NAME} PUBLIC ${PORTAUDIO_LIBRARIES})
        target_include_directories(${PROJECT_NAME} PUBLIC ${PORTAUDIO_INCLUDE_DIRS})
    endif (ENABLE_PORTAUDIO)
endif (NOT DISABLE_AUDIO)

if (NOT DISABLE_GRAPHICS OR NOT DISABLE_AUDIO)
    target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3) # NOTE not sure about these explicit lib names
    target_include_directories(${PROJECT_NAME} PUBLIC ${SDL3_INCLUDE_DIRS})
endif (NOT DISABLE_GRAPHICS OR NOT DISABLE_AUDIO)

if (NOT DISABLE_GRAPHICS)
    # GLM
    target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_ENABLE_EXPERIMENTAL)
    target_compile_definitions(umfeld-lib-interface INTERFACE GLM_ENABLE_EXPERIMENTAL)
    # OpenGL + GLAD + GLM
    target_link_libraries(${PROJECT_NAME} PUBLIC ${OPENGL_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PUBLIC glad)
    target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)
    # font rendering
    target_link_libraries(${PROJECT_NAME} PUBLIC harfbuzz::harfbuzz)
    target_include_directories(${PROJECT_NAME} PUBLIC ${HARFBUZZ_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${FREETYPE_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PUBLIC ${FREETYPE_INCLUDE_DIRS})
    # FFMPEG
    if (NOT DISABLE_VIDEO)
        target_include_directories(umfeld-lib PUBLIC ${FFMPEG_INCLUDE_DIRS})
        target_link_libraries(umfeld-lib PUBLIC ${FFMPEG_LIBRARIES})
        if (APPLE)
            target_link_libraries(umfeld-lib PUBLIC
                    "-framework AVFoundation"
                    "-framework Foundation"
                    "-framework CoreMedia"
            )
        elseif (UNIX)
        elseif (WIN32)
            message(STATUS "platform may not be supported")
        else ()
            message(STATUS "platform may not be supported ... assuming windows.")
        endif ()
    endif (NOT DISABLE_VIDEO)

    # PDF export
    if (NOT DISABLE_PDF)
        target_include_directories(${PROJECT_NAME} PUBLIC ${CAIRO_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} PUBLIC ${CAIRO_LIBRARIES})
        target_compile_options(${PROJECT_NAME} PUBLIC ${CAIRO_CFLAGS_OTHER})
    endif (NOT DISABLE_PDF)

    if (NOT DISABLE_NCURSES)
        target_include_directories(${PROJECT_NAME} PRIVATE ${NCURSES_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${NCURSES_LIBRARIES})
    endif (NOT DISABLE_NCURSES)

endif (NOT DISABLE_GRAPHICS)

# MIDI
if (NOT DISABLE_MIDI)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${RTMIDI_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PUBLIC ${RTMIDI_INCLUDE_DIRS})
    foreach (flag ${RTMIDI_CFLAGS_OTHER})
        if (flag MATCHES "^-D[^=]+(=.+)?$")  # only pass valid -D macros
            target_compile_definitions(${PROJECT_NAME} PUBLIC "${flag}")
        elseif (flag STREQUAL "-pthread")
            target_compile_options(${PROJECT_NAME} PUBLIC "${flag}")
        endif ()
    endforeach ()
endif (NOT DISABLE_MIDI)

#target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)
target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${CURL_LIBRARIES})

##########################################################################################################
### COMPILER FLAGS                                                                                     ###
##########################################################################################################
function(add_umfeld_libs)
    # Check if umfeld-lib-interface target already exists
    if (NOT TARGET umfeld-lib-interface)
        message(FATAL_ERROR "umfeld-lib-interface target not found. Make sure it is defined before calling add_umfeld_libs.")
    endif ()

    # Check if umfeld-lib target already exists
    if (NOT TARGET umfeld-lib)
        message(FATAL_ERROR "umfeld-lib target not found. Make sure it is defined before calling add_umfeld_libs.")
    endif ()

    # Link the libraries if they exist
    set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(${PROJECT_NAME} PRIVATE umfeld-lib-interface)
    target_link_libraries(${PROJECT_NAME} PRIVATE umfeld-lib)

    # pffft SIMD detection
    # TODO maybe make this more platform specific?
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        message(STATUS "PROCESSOR     : ARM/NEON")
        add_compile_definitions(PFFFT_SIMD_DISABLE) # pffft enables NEON by default
    elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "x86" OR CMAKE_SYSTEM_PROCESSOR MATCHES "amd64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "i686")
        message(STATUS "PROCESSOR     : x86/SSE detected")
        add_compile_definitions(PFFFT_SIMD_ENABLE)
        if (MSVC)
            add_compile_options(/arch:SSE2)
        else ()
            add_compile_options(-msse2 -mfpmath=sse)
        endif ()
    endif ()

    # copy data folder resources

    get_target_property(SKETCH_SOURCE_DIR ${PROJECT_NAME} SOURCE_DIR)
    set(DATA_DIR_SRC "${SKETCH_SOURCE_DIR}/data")
    set(DATA_DIR_DST "${CMAKE_CURRENT_BINARY_DIR}/data")

    if (EXISTS "${DATA_DIR_SRC}")
        message(STATUS "'data' FOLDER : found 'data' folder, copying to build directory from")
        message(STATUS "                '${DATA_DIR_SRC}' to")
        message(STATUS "                '${DATA_DIR_DST}'.")
        add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${DATA_DIR_SRC}" "${DATA_DIR_DST}"
                COMMENT "Copying data folder to output directory"
                COMMAND ${CMAKE_COMMAND} -E echo "--- copied data folder to build directory."
        )
    else ()
        message(STATUS "'data' FOLDER : not found")
    endif ()

    message(STATUS "----------------------------------------")
endfunction()

##########################################################################################################
### TEST                                                                                               ###
##########################################################################################################

option(BUILD_TESTING "Enable tests" OFF)

if (BUILD_TESTING)
    message(STATUS "TESTING       : enabled")

    enable_testing()

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for Mac OS X")

    add_executable(umfeld-tests test/umfeld_test.cpp)
    target_link_libraries(umfeld-tests umfeld-lib umfeld-lib-interface)

    add_test(NAME UmfeldTest COMMAND umfeld-tests)
else ()
endif ()

# run test from CLI with `cmake -B build -DBUILD_TESTING=ON ; cmake --build build ; ctest --test-dir build`

##########################################################################################################
### ANGLE                                                                                              ###
##########################################################################################################

if (ENABLE_ANGLE)
    if (APPLE)
        # TODO maybe move this into library folder(s)?
        set(UMFELD_ANGLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/angle" CACHE STRING "ANGLE directory")
        message(STATUS "ANGLE_SUPPORT : ENABLED")
        message(STATUS "ANGLE_DIR     : ${UMFELD_ANGLE_DIR}")

        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${UMFELD_ANGLE_DIR}/libEGL.dylib
                ${UMFELD_ANGLE_DIR}/libGLESv2.dylib
                # NOTE this relies on the build output of `umfeld-lib` being placed inside a subfolder
                #      like `${CMAKE_BINARY_DIR}/umfeld-lib-${PROJECT_NAME}` so that copying ../ lands in app dir
                $<TARGET_FILE_DIR:${PROJECT_NAME}>/..
        )
    else ()
        message(STATUS "ANGLE_SUPPORT : DISABLED ( not supported on this platform )")
    endif (APPLE)
endif (ENABLE_ANGLE)

if (APPLE AND UMFELD_OPENGL_VERSION STREQUAL "OPENGL_ES_3_0" AND NOT ENABLE_ANGLE)
    message(WARNING "OpenGL ES 3.0 requires `ENABLE_ANGLE` set to `ON` on macOS")
endif ()

##########################################################################################################
### END OF SCRIPT                                                                                      ###
##########################################################################################################
