   cmake_minimum_required(VERSION 3.10)
   project(pinrex)

   set(CMAKE_CXX_STANDARD 17)
   set(CMAKE_CXX_STANDARD_REQUIRED True)

   # Include the generated files for the dependencies
   include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
   include_directories(include)

   add_executable(pinrex src/main.cpp)

   # Link the json library to your executable
   target_link_libraries(pinrex PRIVATE nlohmann_json::nlohmann_json)

   # Specify the installation rules
   install(TARGETS pinrex DESTINATION bin)  # This line installs the executable to the bin directory