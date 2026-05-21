#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "gdk::green_gdk" for configuration "Release"
set_property(TARGET gdk::green_gdk APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(gdk::green_gdk PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libgreen_gdk.a"
  )

list(APPEND _cmake_import_check_targets gdk::green_gdk )
list(APPEND _cmake_import_check_files_for_gdk::green_gdk "${_IMPORT_PREFIX}/lib/libgreen_gdk.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
