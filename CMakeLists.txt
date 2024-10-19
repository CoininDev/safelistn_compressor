cmake_minimum_required(VERSION 3.10)
set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

project(safelistn)

include_directories(/usr/include)

add_executable(safelistn main.c)
target_link_libraries(safelistn m rt asound jack portaudio)