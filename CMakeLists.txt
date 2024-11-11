cmake_minimum_required(VERSION 3.10)
project(UltimoProyectoDatos)

set(CMAKE_CXX_STANDARD 11)

# Add the source files
set(SOURCE_FILES código/proyecto-2.cpp)

# Include directories
include_directories(header)

# Add the executable
add_executable(UltimoProyectoDatos ${SOURCE_FILES})

# Link libraries if needed (e.g., pthread)
# target_link_libraries(UltimoProyectoDatos pthread)