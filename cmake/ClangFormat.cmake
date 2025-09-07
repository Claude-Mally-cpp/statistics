function(add_clang_format_targets_for TARGET_NAME)
  if(NOT TARGET ${TARGET_NAME})
    message(FATAL_ERROR "Target '${TARGET_NAME}' does not exist.")
  endif()

  find_program(CLANG_FORMAT_EXE NAMES clang-format clang-format.exe)
  if(NOT CLANG_FORMAT_EXE)
    message(WARNING "clang-format not found. Skipping format targets for ${TARGET_NAME}")
    return()
  endif()

  # Collect normal SOURCES on the target
  get_target_property(_srcs ${TARGET_NAME} SOURCES)
  if(NOT _srcs)
    set(_srcs "")
  endif()

  get_target_property(_header_sets ${TARGET_NAME} HEADER_SETS)
  if(_header_sets)
    foreach(_set IN LISTS _header_sets)
      get_target_property(_set_files ${TARGET_NAME} HEADER_SET_FILES "${_set}")
      if(_set_files)
        list(APPEND _srcs ${_set_files})
      endif()
    endforeach()
  endif()

  if(NOT _srcs)
    message(STATUS "No sources/headers found on target ${TARGET_NAME} for clang-format.")
    return()
  endif()

  list(REMOVE_DUPLICATES _srcs)
  set(_abs_srcs "")
  foreach(_f IN LISTS _srcs)
    if(NOT IS_ABSOLUTE "${_f}")
      file(REAL_PATH "${_f}" _abs BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
      list(APPEND _abs_srcs "${_abs}")
    else()
      list(APPEND _abs_srcs "${_f}")
    endif()
  endforeach()

  add_custom_target(${TARGET_NAME}-format
    COMMAND "${CLANG_FORMAT_EXE}" -i --style=file ${_abs_srcs}
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Formatting sources of ${TARGET_NAME} with clang-format"
    VERBATIM
  )

  add_custom_target(${TARGET_NAME}-format-check
    COMMAND "${CLANG_FORMAT_EXE}" -n --Werror --style=file ${_abs_srcs}
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Checking format for ${TARGET_NAME} with clang-format"
    VERBATIM
  )
endfunction()
