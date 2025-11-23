cmake_minimum_required(VERSION 3.21)
project(garden-planner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automoc/uic/rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

qt_add_executable(garden-planner
    src/main.cpp
    src/CropEditDialog.cpp
    src/CropEditDialog.h
    src/CropWindow.h
    src/DataLoader.h
    src/GanttChartWidget.cpp
    src/GanttChartWidget.h
    src/LeftColumnWidget.cpp
    src/LeftColumnWidget.h
    src/SettingsManager.cpp
    src/SettingsManager.h
    src/UnhideCropsDialog.cpp
    src/UnhideCropsDialog.h
)

target_link_libraries(garden-planner PRIVATE Qt6::Widgets Qt6::Sql)
