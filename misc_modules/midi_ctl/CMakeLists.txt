cmake_minimum_required(VERSION 3.13)
project(midi_ctl)

file(GLOB SRC "src/*.cpp")

include(${SDRPP_MODULE_CMAKE})

# Find RtMidi
find_package(PkgConfig)
pkg_check_modules(RTMIDI REQUIRED rtmidi)

# Add source and core directories to includes
target_include_directories(recorder PRIVATE "src/")
target_include_directories(midi_ctl PRIVATE ${RTMIDI_INCLUDE_DIRS})
target_include_directories(midi_ctl PRIVATE "/opt/homebrew/include")

# Link against RtMidi
target_link_libraries(midi_ctl PRIVATE ${RTMIDI_LIBRARIES})
target_link_directories(midi_ctl PRIVATE /opt/homebrew/lib)
target_link_libraries(midi_ctl PRIVATE rtmidi)

# Add CoreMIDI framework on macOS
if(APPLE)
    target_link_libraries(midi_ctl PRIVATE "-framework CoreMIDI")
    set_target_properties(midi_ctl PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "@loader_path/../Frameworks"
    )
endif()