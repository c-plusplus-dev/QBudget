cmake_minimum_required(VERSION 4.0)
project(QBudget)

set(CMAKE_CXX_STANDARD 20)

find_package(QTip REQUIRED)
find_package(QTipUIKit REQUIRED)

add_executable(QBudget
        main.cpp
        src/Storage/Block.cpp
        src/Storage/Block.h
        src/IO/Reader.cpp
        src/IO/Reader.h
        src/IO/Writer.cpp
        src/IO/Writer.h
        src/Storage/Database.cpp
        src/Storage/Database.h
        src/UI/BlockView.cpp
        src/UI/BlockView.h
        src/Helpers.h
        src/UI/Spacer.cpp
        src/UI/Spacer.h
        src/UI/ItemView.cpp
        src/UI/ItemView.h
        src/UI/BlocksView.cpp
        src/UI/BlocksView.h
)

target_include_directories(QBudget PUBLIC
    src
)

target_link_libraries(QBudget
        QTip::QTip
        QTipUIKit::QTipUIKit
)
