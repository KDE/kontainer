cmake_minimum_required(VERSION 3.16)
project(Kontainer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# === ECM & KDEClangFormat ===
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEClangFormat)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Concurrent)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(SOURCES
    src/appsdialog.cpp
    src/backend.cpp
    src/createcontainerdialog.cpp
    src/main.cpp
    src/mainwindow.cpp
)

set(HEADERS
    include/appsdialog.h
    include/backend.h
    include/createcontainerdialog.h
    include/main.h
    include/mainwindow.h
    include/packagemanager.h
    include/terminalutils.h
)

qt_add_resources(RESOURCES
    resources.qrc
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
)

# === clang-format Target ===
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
