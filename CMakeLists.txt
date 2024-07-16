   cmake_minimum_required(VERSION 3.10)
   project(pinrex)

   set(CMAKE_CXX_STANDARD 17)
   set(CMAKE_CXX_STANDARD_REQUIRED True)

   # Include the generated files for the dependencies
   include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
   include_directories(include)

   add_executable(pinrex src/main.cpp)

   # Find the json package and link it
   find_package(nlohmann_json REQUIRED)
   target_link_libraries(pinrex nlohmann_json::nlohmann_json)