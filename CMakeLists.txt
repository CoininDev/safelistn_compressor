cmake_minimum_required(VERSION 3.10)
set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

find_path(PULSEAUDIO_INCLUDE_DIR
        NAMES pulse/pulseaudio.h
        DOC "The PulseAudio include directory"
        )

find_library(PULSEAUDIO_LIBRARY
        NAMES pulse
        DOC "The PulseAudio library"
        )


project(safelistn)

include_directories(/usr/include)

add_executable(safelistn main.c)
target_link_libraries(safelistn m rt asound jack portaudio pulse-simple pulse )