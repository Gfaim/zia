cmake_minimum_required(VERSION 3.17)

# The name of the CMake project
project(Zia)

# The C++ standard you want to use for your project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

set(CONAN_DISABLE_CHECK_COMPILER 1)

conan_basic_setup(TARGETS NO_OUTPUT_DIRS)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/modules/lib)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})

option(UNIT_TESTS "When sets to ON, build the unit tests" OFF)

if(UNIT_TESTS)
    # The bin name of unit tests
    set(BIN unit_tests)
    include(import/FetchGoogleTest.cmake)
else()
    # The bin name of the project
    set(BIN zia)
endif()

# The build flags
if(UNIX)
    add_compile_options(-Wall -Wextra -Weffc++)
elseif(WIN32)
    add_compile_options(/W4)
endif()

# The location of the main
set(MAIN src/main.cpp)

# The list of source files
set(SOURCES
    src/config/ParseYAML.cpp
    src/Zia.cpp
    src/ModuleLoader.cpp
    src/ModulePipeline.cpp
    src/ConfigLoader.cpp
    src/CLI.cpp
    # fill here
)

# The list of tests source files
set(TEST_SOURCES
    tests/YamlParserString.cpp
    tests/YamlParser.cpp
    tests/test.cpp
    tests/queues.cpp
)

if(UNIT_TESTS)
    add_executable(${BIN}
        ${SOURCES}
        ${TEST_SOURCES}
    )
    enable_testing()
    include(GoogleTest)
    gtest_discover_tests(${BIN})
    target_link_libraries(${BIN} PRIVATE gtest_main)
else()
    add_executable(${BIN}
        ${MAIN}
        ${SOURCES}
    )
endif()

# The include path of the project
target_include_directories(${BIN} PRIVATE
include
CONAN_INCLUDE_DIRS_ASIO
)
target_include_directories(${BIN} PRIVATE
    src
)

# The lib links of the project
if(UNIX)
    target_link_libraries(${BIN} PRIVATE pthread dl)
endif()

target_link_libraries(${BIN} PRIVATE CONAN_PKG::asio)

include(ExternalProject)

ExternalProject_Add(
    ziapi
    GIT_REPOSITORY  https://github.com/martin-olivier/ZiAPI.git
    GIT_TAG         v3.0.0
    INSTALL_COMMAND ""
    TEST_COMMAND    ""
)

ExternalProject_Add(
    libyaml-cpp
    GIT_REPOSITORY  https://github.com/jbeder/yaml-cpp
    GIT_TAG         yaml-cpp-0.7.0
    BUILD_COMMAND   ${CMAKE_COMMAND} -DYAML_CPP_BUILD_TESTS=OFF .
    COMMAND         ${CMAKE_COMMAND} --build .
    INSTALL_COMMAND ""
    TEST_COMMAND    ""
)

ExternalProject_Add(
    cxxopt
    GIT_REPOSITORY  https://github.com/jarro2783/cxxopts.git
    GIT_TAG         v3.0.0
    INSTALL_COMMAND ""
    TEST_COMMAND    ""
)

add_dependencies(${BIN} libyaml-cpp)
add_dependencies(${BIN} ziapi)
add_dependencies(${BIN} cxxopt)

ExternalProject_Get_Property(libyaml-cpp INSTALL_DIR)
include_directories(${INSTALL_DIR}/src/libyaml-cpp/include)
target_link_directories(${BIN} PUBLIC ${INSTALL_DIR}/src/libyaml-cpp-build)
target_link_libraries(${BIN} PUBLIC yaml-cpp)

ExternalProject_Get_Property(cxxopt SOURCE_DIR)
include_directories(${SOURCE_DIR}/include)

ExternalProject_Get_Property(ziapi SOURCE_DIR)
include_directories(${SOURCE_DIR}/include)


add_subdirectory(modules)
