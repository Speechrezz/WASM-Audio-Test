cmake_minimum_required(VERSION 3.8)

project(WASMAudioTest LANGUAGES C CXX VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 23)

if (DEFINED EMSCRIPTEN)
    add_executable(AudioWorklet
        src/AudioWorklet.cpp
        src/Synthle.cc
    )
    set_target_properties(AudioWorklet PROPERTIES LINK_FLAGS "--bind -sSHARED_MEMORY=1 -sPTHREAD_POOL_SIZE=4 -sAUDIO_WORKLET=1 -sWASM_WORKERS=1 -sWEBAUDIO_DEBUG=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -matomics -mbulk-memory")
else()
    # add_executable(AudioWorklet src/AudioWorklet.cpp)
    set(CLAP_TARGET Synthle)
    set(CLAP_BUNDLE_ID "audio.xynth.Synthle")

    add_library(${CLAP_TARGET} MODULE
        src/ClapEntry.cpp
        src/Synthle.cc
        src/CLAP/Gui.mm
    )

    target_link_libraries(${CLAP_TARGET}
        # All needed for macOS GUI.
        "-framework WebKit"
        "-framework Foundation"
        "-framework Cocoa"
        "-framework CoreVideo" # For DisplayLink
    )
    set_target_properties(${CLAP_TARGET} PROPERTIES
        OUTPUT_NAME "${CLAP_TARGET}"
        LIBRARY_OUTPUT_NAME "${CLAP_TARGET}"
        PREFIX ""
        SUFFIX ".clap"
        BUNDLE True
        BUNDLE_EXTENSION clap
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "${CLAP_BUNDLE_ID}"
        MACOSX_BUNDLE_BUNDLE_NAME "${CLAP_TARGET}"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/src/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_VERSION ${CMAKE_PROJECT_VERSION}
        MACOSX_BUNDLE_LONG_VERSION_STRING "v${CMAKE_PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "(C) 2024 Joseph Lyncheski"
        MACOSX_BUNDLE_ICON_FILE "AppIcon"
        MACOSX_BUNDLE_EXECUTABLE_NAME "${CLAP_TARGET}"
        MACOSX_BUNDLE_DISPLAY_NAME "${CLAP_TARGET}"
        XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE "YES"
    )
endif()
