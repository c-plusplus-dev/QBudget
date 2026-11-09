
cmake_minimum_required(VERSION 4.0)

project(QBudget
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(QTip REQUIRED)
find_package(QTipUIKit REQUIRED)

set(QBUDGET_SOURCES
    main.cpp

    src/Platform.h

    src/Storage/Block.cpp
    src/Storage/Block.h
    src/Storage/Database.cpp
    src/Storage/Database.h
    src/Storage/DatabaseManager.cpp
    src/Storage/DatabaseManager.h

    src/IO/Reader.cpp
    src/IO/Reader.h
    src/IO/Writer.cpp
    src/IO/Writer.h

    src/UI/BlockView.cpp
    src/UI/BlockView.h
    src/UI/Textures.h
    src/UI/Visibility.h
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

    src/Helpers.h
)

add_executable(QBudget ${QBUDGET_SOURCES})

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

# macOS application bundle
if(APPLE)
    set_target_properties(QBudget PROPERTIES
        MACOSX_BUNDLE TRUE

        MACOSX_BUNDLE_BUNDLE_NAME "QBudget"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.cplusplusdev.qbudget"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright © c-plusplus-dev"
    )

    # Place assets inside Contents/Resources/assets
    add_custom_command(TARGET QBudget POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_BUNDLE_DIR:QBudget>/Contents/Resources/assets"

        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_BUNDLE_DIR:QBudget>/Contents/Resources/assets"

        VERBATIM
    )

else()
    # Windows and Linux: Keep assets beside the executable.
    add_custom_command(TARGET QBudget POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:QBudget>/assets"

        VERBATIM
    )
endif()

# Installation
include(GNUInstallDirs)

if(APPLE)
    install(TARGETS QBudget
        BUNDLE DESTINATION .
    )
else()
    install(TARGETS QBudget
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(DIRECTORY assets/
        DESTINATION ${CMAKE_INSTALL_BINDIR}/assets
    )
endif()

# Packaging
if(WIN32)
    set(CPACK_GENERATOR "NSIS")

    set(CPACK_PACKAGE_NAME "QBudget")
    set(CPACK_PACKAGE_VENDOR "c-plusplus-dev")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
        "A simple budgeting application."
    )
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")

    set(CPACK_PACKAGE_INSTALL_DIRECTORY "QBudget")

    set(CPACK_NSIS_DISPLAY_NAME "QBudget")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH OFF)

    include(CPack)

elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_PACKAGE_NAME "QBudget")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")

    include(CPack)
endif()
