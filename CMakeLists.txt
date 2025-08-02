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
find_package(ECM  6.16.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
find_package(KF6  6.17.0 REQUIRED COMPONENTS I18n)
find_package(KF6KIO 6.17 REQUIRED)
include(KDEClangFormat)
include(KDEGitCommitHooks)
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

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
    include/toolboximages.h
)

qt_add_resources(RESOURCES
    res/resources.qrc
)

add_executable(kontainer ${SOURCES} ${HEADERS} ${RESOURCES})

target_link_libraries(kontainer PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Concurrent
    KF6::I18n
    KF6::KIOGui
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
