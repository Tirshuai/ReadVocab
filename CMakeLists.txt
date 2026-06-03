cmake_minimum_required(VERSION 3.16)

project(wordlearner VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 强制用 Qt6，并包含 Widgets 和 Sql 模块
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql Network)

# 你的所有源码文件
add_executable(wordlearner
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    src/learnnewwidget.cpp
    src/learnnewwidget.h
    src/fastmodewidget.cpp
    src/fastmodewidget.h
    src/scenariomodewidget.cpp
    src/scenariomodewidget.h
    src/wordlistwidget.cpp
    src/wordlistwidget.h
    src/wordrelaterwidget.cpp
    src/wordrelaterwidget.h
    src/relatepage.cpp
    src/relatepage.h
    src/aitextgenerator.cpp
    src/aitextgenerator.h
    src/reviewwidget.cpp
    src/reviewwidget.h
    src/fastreviewwidget.cpp
    src/fastreviewwidget.h
    src/reviewsituationwidget.cpp
    src/reviewsituationwidget.h
    src/wordbookwidget.cpp
    src/wordbookwidget.h
)

# 链接 Qt6 模块
target_link_libraries(wordlearner
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Sql
        Qt6::Network
)

# 处理 Windows 下的控制台窗口（可选）
if(WIN32)
    set_target_properties(wordlearner PROPERTIES WIN32_EXECUTABLE TRUE)
endif()