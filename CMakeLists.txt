cmake_minimum_required(VERSION 3.18)
project(project C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 2. Enable Link-Time Optimization / Interprocedural Optimization (-flto)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

# 3. Find Dependency Libraries
find_package(PNG REQUIRED)
find_package(OpenCL REQUIRED)

# 4. Define the Executable Target
add_executable(main
    src/main.c
)

# 5. Best-Practice Compiler Warnings & Security Hardening Flags
target_compile_options(main PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wformat=2
    -Wconversion
    -Wshadow
    -fstack-protector-strong
    -D_FORTIFY_SOURCE=3
)
set_target_properties(main PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_options(main PRIVATE
    -Wl,-z,relro
    -Wl,-z,now
    -Wl,-z,noexecstack
)

# 7. Link Dependencies
# This handles both the library (-lpng) and its required include directories automatically
target_link_libraries(main PRIVATE
    PNG::PNG
    OpenCL::OpenCL
)