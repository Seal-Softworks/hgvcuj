cmake_minimum_required(VERSION 3.20)

project(hgvcuj LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
    message(FATAL_ERROR "This project requires windows bro")
endif()

add_compile_definitions(
    WINVER=0x0A00
    _WIN32_WINNT=0x0A00
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
)

set(SOURCES
    src/main.cpp
    src/payloads/GDI.cpp
    src/payloads/gdirender/hsv_shift.cpp
    src/bytebeat.cpp
    src/payloads/gdirender/gdieffects.cpp
    src/stages.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    gdi32
    user32
)

if(WIN32 AND MSVC)
    set_target_properties(hgvcuj PROPERTIES
        LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\""
    )
endif()