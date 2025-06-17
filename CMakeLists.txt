# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.16)
project(Kontainer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
include(GNUInstallDirs)

# === ECM & KDEClangFormat & KI18n ===
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
find_package(KF6 REQUIRED COMPONENTS I18n)
include(KDEClangFormat)
include(KDEGitCommitHooks)
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Concurrent)

file(GLOB RES_CONTENT "${CMAKE_SOURCE_DIR}/res/*")

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/res" OR RES_CONTENT STREQUAL "")
    message(STATUS "Cloning kontainer-logos-trademarks into ${CMAKE_SOURCE_DIR}/res")
    execute_process(
        COMMAND git clone https://invent.kde.org/silverhadch/kontainer-logos-trademarks.git res
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE result
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Failed to clone kontainer-logos-trademarks repository")
    endif()
else()
    message(STATUS "Skipping clone of kontainer-logos-trademarks – 'res' already exists and is not empty")
endif()


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
    res/resources.qrc
)

add_executable(kontainer ${SOURCES} ${HEADERS} ${RESOURCES})

target_link_libraries(kontainer PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    KF6::I18n
)

ki18n_install(po)

# === clang-format Target ===
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

# === Flatpak Install Target ===
install(TARGETS kontainer RUNTIME DESTINATION bin)
install(FILES org.kde.kontainer.desktop DESTINATION share/applications)
