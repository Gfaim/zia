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
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/import)

option(UNIT_TESTS "When sets to ON, build the unit tests" OFF)

include(FetchZiAPI)
include(FetchYamlCpp)
include(FetchCXXopt)

if(UNIT_TESTS)
    # The bin name of unit tests
    set(BIN unit_tests)
    include(FetchGoogleTest.cmake)
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
    src/RequestManager.cpp
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
    gtest_discover_tests(${BIN} PROPERTIES TIMEOUT 600)
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

if(WIN32)
    target_link_libraries(${BIN} PUBLIC yaml-cppd)
else()
    target_link_libraries(${BIN} PUBLIC yaml-cpp)
endif()

add_subdirectory(modules)
