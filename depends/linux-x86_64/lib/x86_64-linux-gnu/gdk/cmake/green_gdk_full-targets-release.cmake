#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "gdk::green_gdk_full" for configuration "Release"
set_property(TARGET gdk::green_gdk_full APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(gdk::green_gdk_full PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/x86_64-linux-gnu/libgreen_gdk_full.a"
  )

list(APPEND _cmake_import_check_targets gdk::green_gdk_full )
list(APPEND _cmake_import_check_files_for_gdk::green_gdk_full "${_IMPORT_PREFIX}/lib/x86_64-linux-gnu/libgreen_gdk_full.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
