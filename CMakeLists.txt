cmake_minimum_required(VERSION 4.0)
project(QBudget LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(QTip REQUIRED)
find_package(QTipUIKit REQUIRED)

add_executable(QBudget
        main.cpp
        src/Platform.h
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
        src/UI/Textures.h
        src/UI/Visibility.h
        src/Helpers.h
        src/UI/Spacer.cpp
        src/UI/Spacer.h
        src/UI/ItemView.cpp
        src/UI/ItemView.h
        src/UI/BlocksView.cpp
        src/UI/BlocksView.h
        src/UI/PieView.cpp
        src/UI/PieView.h
        src/UI/MonthsView.cpp
        src/UI/MonthsView.h
        src/UI/MonthPalette.h
        src/UI/BarView.cpp
        src/UI/BarView.h
        src/Storage/DatabaseManager.cpp
        src/Storage/DatabaseManager.h
)

target_include_directories(QBudget PRIVATE
    src
)

target_link_libraries(QBudget PRIVATE
    QTip::QTip
    QTipUIKit::QTipUIKit
)

if(WIN32)
    target_compile_definitions(QBudget PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
endif()


# Keep runtime assets beside the executable on Windows, macOS and Linux.
add_custom_command(TARGET QBudget POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:QBudget>/assets"
    VERBATIM
)

include(GNUInstallDirs)
install(TARGETS QBudget
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY assets/
    DESTINATION ${CMAKE_INSTALL_BINDIR}/assets
)
