cmake_minimum_required(VERSION 3.17)

include(FetchContent)

FetchContent_Declare(
    cxxopts
    GIT_REPOSITORY  "https://github.com/jarro2783/cxxopts"
    GIT_TAG         "v3.0.0"
)

FetchContent_MakeAvailable(cxxopts)
include_directories(${cxxopts_SOURCE_DIR}/include)