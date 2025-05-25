cmake_minimum_required(VERSION 3.5)
project(Kontainer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/backend.cpp
    src/createcontainerdialog.cpp
)

set(HEADERS
    include/mainwindow.h
    include/backend.h
    include/createcontainerdialog.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
)

