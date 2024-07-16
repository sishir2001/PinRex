   cmake_minimum_required(VERSION 3.10)
   project(pinrex)

   set(CMAKE_CXX_STANDARD 17)
   set(CMAKE_CXX_STANDARD_REQUIRED True)

   include_directories(include)

   # Fetch nlohmann/json from its Github repository
   include(FetchContent)
   FetchContent_Declare(
      json
      GIT_REPOSITORY https://github.com/nlohmann/json.git
      # GIT_TAG 3.11.0
   )
   FetchContent_MakeAvailable(json)

   add_executable(pinrex src/main.cpp)

   # Link the json library to your executable
   target_link_libraries(pinrex PRIVATE nlohmann_json::nlohmann_json)
   # Specify the installation rules
   install(TARGETS pinrex DESTINATION bin)  # This line installs the executable to the bin directory#include <algorithm>