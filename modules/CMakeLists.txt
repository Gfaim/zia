cmake_minimum_required(VERSION 3.17)

file(GLOB modules_folders LIST_DIRECTORIES true src/*)
foreach(dir ${modules_folders})
    if(IS_DIRECTORY ${dir})
        file(GLOB mod_src CONFIGURE_DEPENDS "${dir}/*.cpp")
        get_filename_component(mod_name ${dir} NAME_WLE)

        add_library(${mod_name} SHARED ${mod_src})
        target_include_directories(${mod_name} PRIVATE include)
        add_dependencies(${mod_name} ziapi)
        set_target_properties(${mod_name} PROPERTIES PREFIX "")
    endif()
endforeach()