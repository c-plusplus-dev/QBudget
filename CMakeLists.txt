cmake_minimum_required(VERSION 4.0)
project(QBudget)

set(CMAKE_CXX_STANDARD 20)

find_package(QTip REQUIRED)
find_package(QTipUIKit REQUIRED)

add_executable(QBudget
        main.cpp
        src/Block.cpp
        src/Block.h
        src/Reader.cpp
        src/Reader.h
        src/Writer.cpp
        src/Writer.h
        src/Database.cpp
        src/Database.h
)

target_include_directories(QBudget PUBLIC
    src
)

target_link_libraries(QBudget
        QTip::QTip
        QTipUIKit::QTipUIKit
)
