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
    main.cpp
    mainwindow.cpp
    mainwindow.h
    learnnewwidget.cpp
    fastmodewidget.cpp
    scenariomodewidget.cpp
    wordlistwidget.cpp
    wordrelaterwidget.cpp
    learnnewwidget.h
    fastmodewidget.h
    scenariomodewidget.h
    wordlistwidget.h
    wordrelaterwidget.h
    relatepage.h
    relatepage.cpp
    aitextgenerator.cpp
    aitextgenerator.h
    reviewwidget.h
    reviewwidget.cpp
    fastreviewwidget.h
    fastreviewwidget.cpp
    reviewwidget.cpp
    reviewwidget.h
    fastreviewwidget.cpp
    fastreviewwidget.h
    reviewsituationwidget.h
    reviewsituationwidget.cpp
    reviewsituationwidget.cpp
    reviewsituationwidget.h
    wordbookwidget.h
    wordbookwidget.cpp
    wordbookwidget.cpp
    wordbookwidget.h
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