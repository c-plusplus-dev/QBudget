#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "QTip::QTip" for configuration "Debug"
set_property(TARGET QTip::QTip APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(QTip::QTip PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libQTip.a"
  )

list(APPEND _cmake_import_check_targets QTip::QTip )
list(APPEND _cmake_import_check_files_for_QTip::QTip "${_IMPORT_PREFIX}/lib/libQTip.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
