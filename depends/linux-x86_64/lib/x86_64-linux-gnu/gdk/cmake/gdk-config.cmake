
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was gdk-config.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################


set_and_check(GDK_LIB_DIR "${PACKAGE_PREFIX_DIR}/lib/x86_64-linux-gnu/gdk")

if("green_gdk" IN_LIST gdk_FIND_COMPONENTS)
    include("${GDK_LIB_DIR}/cmake/green_gdk-targets.cmake")
    foreach(_target IN LISTS _IMPORT_CHECK_FILES_FOR_gdk::green_gdk)
        if(NOT EXIST ${_target})
            message(FATAL_ERROR "missing library ${_target}, please ``cmake install --component gdk-runtime")
        endif()
    endforeach()
    set(gdk_green_gdk_FOUND TRUE)
endif()

if("green_gdk_full" IN_LIST gdk_FIND_COMPONENTS)
    include("${GDK_LIB_DIR}/cmake/green_gdk_full-targets.cmake")
    foreach(_target IN LISTS _IMPORT_CHECK_FILES_FOR_gdk::green_gdk_full)
        if(NOT EXIST ${_target})
            message(FATAL_ERROR "missing library ${_target}, please ``cmake install --component gdk-dev")
        endif()
    endforeach()
    set(gdk_green_gdk_full_FOUND TRUE)
endif()
set(gdk_COMPONENT_FOUND TRUE)
check_required_components(gdk)
