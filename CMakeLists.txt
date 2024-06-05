cmake_minimum_required(VERSION 3.27)

project(OceanEngine C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

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
add_subdirectory(./source/PGUtil)
add_subdirectory(./source/AAAgame)

include_directories(${PROJECT_DIRECTORIES})
include_directories(${CMAKE_SOURCE_DIR}/libraries/glfw/include/GLFW)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)

if(WIN32)
    message("win32 detected")
elseif(APPLE)
    # https://stackoverflow.com/questions/17070101/why-i-cannot-link-the-mac-framework-file-with-cmake
    set(LIBS 
        "-framework CoreServices"
        "-framework AudioUnit"
        "-framework AudioToolbox"
        "-framework IOKit"
        "-framework CoreGraphics"
        "-framework AppKit"
        ${CMAKE_SOURCE_DIR}/libraries/glfw/lib-arm64/libglfw3.a
    )
    target_link_libraries(OceanEngine ${LIBS})
    set_target_properties(OceanEngine PROPERTIES LINK_FLAGS "-Wl,-F/Library/Frameworks")
elseif(UNIX)
    message("unix detected")
endif()