cmake_minimum_required(VERSION 3.17)

file(GLOB modules_folders LIST_DIRECTORIES true src/*)
foreach(dir ${modules_folders})
    if(IS_DIRECTORY ${dir})
        file(GLOB mod_src CONFIGURE_DEPENDS "${dir}/*.cpp")
        get_filename_component(mod_name ${dir} NAME_WLE)
        add_library(${mod_name} SHARED ${mod_src})
        target_include_directories(${mod_name} PRIVATE include/${mod_name} CONAN_INCLUDE_DIRS_ASIO CONAN_INCLUDE_DIRS_OPENSSL)
        target_link_libraries(${mod_name} PRIVATE CONAN_PKG::asio CONAN_PKG::openssl)
        set_target_properties(${mod_name} PROPERTIES PREFIX "")
    endif()
endforeach()
