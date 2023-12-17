# This build file generates the executables for each of the
# Quic simulation scenarios

set(target_prefix scenario_)

function(create_scenario_exec source_files)
  # Return early if no sources in the subdirectory
  list(LENGTH source_files number_sources)
  if(number_sources EQUAL 0)
    return()
  endif()

  # If the scenario has more than a source file, we need to find the source with
  # the main function
  set(scenario_src)
  foreach(source_file ${source_files})
    file(READ ${source_file} source_file_contents)
    string(REGEX MATCHALL "main[(| (]" main_position "${source_file_contents}")
    if(CMAKE_MATCH_0)
      set(scenario_src ${source_file})
    endif()
  endforeach()

  if(NOT scenario_src)
    return()
  endif()

  # Get parent directory name
  get_filename_component(scenario_dirname ${scenario_src} DIRECTORY)
  string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" scenario_dirname
                 "${scenario_dirname}"
  )
  string(REPLACE "/" "_" scenario_dirname "${scenario_dirname}")

  # Get source name
  if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.14.0")
    get_filename_component(scenario_name ${scenario_src} NAME_WLE)
  else()
    get_filename_component(scenario_name ${scenario_src} NAME)
    string(FIND "${scenario_name}" "." ext_position REVERSE)
    if(${ext_position} EQUAL -1)
      message(FATAL_ERROR "Source file has no extension: ${scenario_src}")
    else()
      string(SUBSTRING "${scenario_name}" 0 ${ext_position} scenario_name)
    endif()
  endif()

  set(target_prefix scenario_)
  if(scenario_dirname)
    # Join the names together if dirname is not the scenario folder
    set(target_prefix scenario${scenario_dirname}_)
  endif()

  # Get source absolute path and transform into relative path
  get_filename_component(scenario_src ${scenario_src} ABSOLUTE)
  get_filename_component(scenario_absolute_directory ${scenario_src} DIRECTORY)
  string(REPLACE "${PROJECT_SOURCE_DIR}" "${CMAKE_OUTPUT_DIRECTORY}"
                 scenario_directory ${scenario_absolute_directory}
  )
  build_exec(
          EXECNAME ${scenario_name}
          EXECNAME_PREFIX ${target_prefix}
          SOURCE_FILES "${source_files}"
          LIBRARIES_TO_LINK "${ns3-libs}" "${ns3-contrib-libs}"
          EXECUTABLE_DIRECTORY_PATH ${scenario_directory}/
  )
endfunction()

# Scan *.cc files in subdirectories and build a target for each
# subdirectory
file(
  GLOB_RECURSE scenarios_subdirectories
  CONFIGURE_DEPENDS
  LIST_DIRECTORIES true
  ${CMAKE_CURRENT_SOURCE_DIR}/**
)
# Leave out entries that are not directories
foreach(entry ${scenarios_subdirectories})
  if(NOT (IS_DIRECTORY ${entry}))
    list(REMOVE_ITEM scenarios_subdirectories ${entry})
  endif()
endforeach()

foreach(subdir ${scenarios_subdirectories})
  # pick all the files in the subdirectory
  # and create an executable for them automatically
  file(GLOB scenario_sources CONFIGURE_DEPENDS ${subdir}/[^.]*.cc)
  # add the helper sources
  list(APPEND scenario_sources ${subdir}/../helper/quic-network-simulator-helper.cc)
  list(APPEND scenario_sources ${subdir}/../helper/quic-point-to-point-helper.cc)
  list(APPEND scenario_sources ${subdir}/../helper/quic-packet.cc)
  # create the executable
  create_scenario_exec("${scenario_sources}")
endforeach()
