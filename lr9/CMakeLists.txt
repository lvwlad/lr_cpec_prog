cmake_minimum_required(VERSION 3.5)
project(BluetoothEmulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

add_executable(BluetoothEmulator
    main.cpp
    mainwindow.cpp
    deviceemulator.cpp
    deviceselectiondialog.cpp
    magma.cpp
)

target_link_libraries(BluetoothEmulator PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)
