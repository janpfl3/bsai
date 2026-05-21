#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "crashpad::compat" for configuration "Release"
set_property(TARGET crashpad::compat APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(crashpad::compat PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcrashpad_compat.a"
  )

list(APPEND _cmake_import_check_targets crashpad::compat )
list(APPEND _cmake_import_check_files_for_crashpad::compat "${_IMPORT_PREFIX}/lib/libcrashpad_compat.a" )

# Import target "crashpad::minidump" for configuration "Release"
set_property(TARGET crashpad::minidump APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(crashpad::minidump PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcrashpad_minidump.a"
  )

list(APPEND _cmake_import_check_targets crashpad::minidump )
list(APPEND _cmake_import_check_files_for_crashpad::minidump "${_IMPORT_PREFIX}/lib/libcrashpad_minidump.a" )

# Import target "crashpad::snapshot" for configuration "Release"
set_property(TARGET crashpad::snapshot APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(crashpad::snapshot PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcrashpad_snapshot.a"
  )

list(APPEND _cmake_import_check_targets crashpad::snapshot )
list(APPEND _cmake_import_check_files_for_crashpad::snapshot "${_IMPORT_PREFIX}/lib/libcrashpad_snapshot.a" )

# Import target "crashpad::util" for configuration "Release"
set_property(TARGET crashpad::util APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(crashpad::util PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "ASM;CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcrashpad_util.a"
  )

list(APPEND _cmake_import_check_targets crashpad::util )
list(APPEND _cmake_import_check_files_for_crashpad::util "${_IMPORT_PREFIX}/lib/libcrashpad_util.a" )

# Import target "crashpad::mini_chromium" for configuration "Release"
set_property(TARGET crashpad::mini_chromium APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(crashpad::mini_chromium PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libmini_chromium.a"
  )

list(APPEND _cmake_import_check_targets crashpad::mini_chromium )
list(APPEND _cmake_import_check_files_for_crashpad::mini_chromium "${_IMPORT_PREFIX}/lib/libmini_chromium.a" )

# Import target "crashpad::client" for configuration "Release"
set_property(TARGET crashpad::client APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(crashpad::client PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "ASM;CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcrashpad_client.a"
  )

list(APPEND _cmake_import_check_targets crashpad::client )
list(APPEND _cmake_import_check_files_for_crashpad::client "${_IMPORT_PREFIX}/lib/libcrashpad_client.a" )

# Import target "crashpad::zlib" for configuration "Release"
set_property(TARGET crashpad::zlib APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(crashpad::zlib PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcrashpad_zlib.a"
  )

list(APPEND _cmake_import_check_targets crashpad::zlib )
list(APPEND _cmake_import_check_files_for_crashpad::zlib "${_IMPORT_PREFIX}/lib/libcrashpad_zlib.a" )

# Import target "crashpad::mpack" for configuration "Release"
set_property(TARGET crashpad::mpack APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(crashpad::mpack PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcrashpad_mpack.a"
  )

list(APPEND _cmake_import_check_targets crashpad::mpack )
list(APPEND _cmake_import_check_files_for_crashpad::mpack "${_IMPORT_PREFIX}/lib/libcrashpad_mpack.a" )

# Import target "crashpad::tools" for configuration "Release"
set_property(TARGET crashpad::tools APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(crashpad::tools PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcrashpad_tools.a"
  )

list(APPEND _cmake_import_check_targets crashpad::tools )
list(APPEND _cmake_import_check_files_for_crashpad::tools "${_IMPORT_PREFIX}/lib/libcrashpad_tools.a" )

# Import target "crashpad::handler" for configuration "Release"
set_property(TARGET crashpad::handler APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(crashpad::handler PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcrashpad_handler_lib.a"
  )

list(APPEND _cmake_import_check_targets crashpad::handler )
list(APPEND _cmake_import_check_files_for_crashpad::handler "${_IMPORT_PREFIX}/lib/libcrashpad_handler_lib.a" )

# Import target "crashpad::crashpad_handler" for configuration "Release"
set_property(TARGET crashpad::crashpad_handler APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(crashpad::crashpad_handler PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/crashpad_handler"
  )

list(APPEND _cmake_import_check_targets crashpad::crashpad_handler )
list(APPEND _cmake_import_check_files_for_crashpad::crashpad_handler "${_IMPORT_PREFIX}/bin/crashpad_handler" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
