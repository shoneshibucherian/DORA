#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "boots::boots" for configuration ""
set_property(TARGET boots::boots APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(boots::boots PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libboots.so"
  IMPORTED_SONAME_NOCONFIG "libboots.so"
  )

list(APPEND _cmake_import_check_targets boots::boots )
list(APPEND _cmake_import_check_files_for_boots::boots "${_IMPORT_PREFIX}/lib/libboots.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
