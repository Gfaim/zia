cmake_minimum_required(VERSION 3.17)

include(FetchContent)

FetchContent_Declare(
    yamlcpp
    GIT_REPOSITORY  "https://github.com/jbeder/yaml-cpp"
    GIT_TAG         "yaml-cpp-0.7.0"
)

set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(yamlcpp)
include_directories(${yamlcpp_SOURCE_DIR}/include)

if(NOT yamlcpp_POPULATED)
  FetchContent_Populate(yamlcpp)
  add_subdirectory(yamlcpp)
endif()

link_directories(${yamlcpp_INSTALL_DIR}/src/libyaml-cpp-build)