cmake_minimum_required(VERSION 3.17)

include(FetchContent)

FetchContent_Declare(
    ziapi
    GIT_REPOSITORY      "https://github.com/martin-olivier/ZiAPI"
    GIT_TAG             "v4.0.0"
)

FetchContent_MakeAvailable(ziapi)
include_directories(${ziapi_SOURCE_DIR}/include)