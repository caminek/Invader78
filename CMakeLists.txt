cmake_minimum_required(VERSION 3.15)
project(Invader78)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} main.cpp CPU.cpp CPU.hpp Bus.cpp Bus.hpp Display.cpp Display.hpp Audio.cpp Audio.hpp Controls.cpp Controls.hpp)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMENT "Copying Resources to target directory"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/Resources
        )


find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

if (SDL2_FOUND)
    include_directories(${SDL2_INCLUDE_DIR}
            ${SDL2_IMAGE_INCLUDE_DIR}
            ${SDL2_TTF_INCLUDE_DIR}
            ${SDL2_MIXER_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY}
            ${SDL2_IMAGE_LIBRARIES}
            ${SDL2_TTF_LIBRARIES}
            ${SDL2_MIXER_LIBRARIES})
endif ()