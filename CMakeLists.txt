cmake_minimum_required(VERSION 3.27)

project(TempEngineName C)

set(CMAKE_C_STANDARD 17)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(_DEBUG)
endif()

macro(recursive_add_all)
    #include all source files into main list
    file(GLOB_RECURSE LOCAL_PROJECT_SOURCES CONFIGURE_DEPENDS *.h *.c)
    set(PROJECT_SOURCES ${PROJECT_SOURCES}${LOCAL_PROJECT_SOURCES}\; PARENT_SCOPE)

    #include all subdirectories
    file(GLOB_RECURSE FILE_LIST LIST_DIRECTORIES true CONFIGURE_DEPENDS *)
    foreach(DIR ${FILE_LIST})
        if(IS_DIRECTORY ${DIR})
            set(LOCAL_DIRECTORIES ${LOCAL_DIRECTORIES}${DIR}\;)
        endif()
    endforeach()

    #include this directory
    set(LOCAL_DIRECTORIES ${LOCAL_DIRECTORIES}${CMAKE_CURRENT_LIST_DIR}\;)

    set(PROJECT_DIRECTORIES ${PROJECT_DIRECTORIES}\;${LOCAL_DIRECTORIES} PARENT_SCOPE)
endmacro()

add_subdirectory(./source/BLoader)
add_subdirectory(./source/Constructure)
add_subdirectory(./source/MokyoMidi)
add_subdirectory(./source/Trifecta)
add_subdirectory(./source/Unknown)
add_subdirectory(./source/WindECS)
add_subdirectory(./source/ZMath)
add_subdirectory(./source/AAAgame)

include_directories(${PROJECT_DIRECTORIES})

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)