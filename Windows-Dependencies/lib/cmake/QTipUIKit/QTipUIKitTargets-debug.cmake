#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "QTipUIKit::QTipUIKit" for configuration "Debug"
set_property(TARGET QTipUIKit::QTipUIKit APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(QTipUIKit::QTipUIKit PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libQTipUIKit.a"
  )

list(APPEND _cmake_import_check_targets QTipUIKit::QTipUIKit )
list(APPEND _cmake_import_check_files_for_QTipUIKit::QTipUIKit "${_IMPORT_PREFIX}/lib/libQTipUIKit.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
