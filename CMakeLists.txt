#[[
 Licensed under the MIT License.
 For details on the licensing terms, see the LICENSE file.
 SPDX-License-Identifier: MIT

 Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
]]

cmake_minimum_required(VERSION 3.0.0)
project(swap_server_template VERSION 1.0.0 DESCRIPTION "Library to make open62541 server compatible with Process Agents")
find_package(open62541 1.3 REQUIRED)



if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # needed or cmake doesn't recognize dependencies of generated files
    set(PROJECT_BINARY_DIR ${CMAKE_BINARY_DIR})
endif()

file(MAKE_DIRECTORY "${GENERATE_OUTPUT_DIR}")
set(GENERATE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/src_generated/")
include_directories("${GENERATE_OUTPUT_DIR}")
set(INFORMATION_MODEL_DIR ${PROJECT_SOURCE_DIR}/model)

ua_generate_nodeset_and_datatypes(
        NAME "common"
        FILE_BSD "${INFORMATION_MODEL_DIR}/SWAP.Fraunhofer.Common.Model.Types.bsd"
        FILE_CSV "${INFORMATION_MODEL_DIR}/CommonModelDesign.csv"
        FILE_NS "${INFORMATION_MODEL_DIR}/SWAP.Fraunhofer.Common.Model.NodeSet2.xml"
        NAMESPACE_MAP "2:http://common.swap.fraunhofer.de"
        OUTPUT_DIR ${GENERATE_OUTPUT_DIR}
        INTERNAL
)

set(PATH_TO_HEADERS  ${PROJECT_SOURCE_DIR}/deps/include)
set(PATH_TO_C_FILES ${PROJECT_SOURCE_DIR}/deps/src)

set(exported_headers
        ${PATH_TO_HEADERS}/server_internal.h
        ${PATH_TO_HEADERS}/node_finder.h
        ${PATH_TO_HEADERS}/config_interpreter.h
        ${PATH_TO_HEADERS}/queue_handler.h
        ${PATH_TO_HEADERS}/register_callbacks.h
        ${PATH_TO_HEADERS}/parse_num.h
        ${PATH_TO_HEADERS}/cj5.h
        ${PATH_TO_HEADERS}/swap_it.h
        ${GENERATE_OUTPUT_DIR}/types_common_generated.h
        ${GENERATE_OUTPUT_DIR}/types_common_generated_handling.h
)


#src: https://stackoverflow.com/questions/17511496/how-to-create-a-shared-library-with-cmake, last checked on 07.11.2023:16.17
include(GNUInstallDirs)
add_library(swap_server_template SHARED
        ${PATH_TO_C_FILES}/swap_it.c
        ${PATH_TO_C_FILES}/config_interpreter.c
        ${PATH_TO_C_FILES}/node_finder.c
        ${PATH_TO_C_FILES}/queue_handler.c
        ${PATH_TO_C_FILES}/register_callbacks.c
        ${PATH_TO_C_FILES}/server_internal.c
        ${PATH_TO_C_FILES}/cj5.c
        ${PATH_TO_C_FILES}/parse_num.c
        ${UA_NODESET_COMMON_SOURCES}
        ${UA_TYPES_COMMON_SOURCES}
          )

add_library(swap_server_template::swap_server_template ALIAS swap_server_template)
set_target_properties(swap_server_template PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)

set(FILES_TO_INSTALL ${exported_headers})
foreach ( file ${FILES_TO_INSTALL} )
    set(full_path ${file})
    install(FILES ${full_path} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endforeach()

configure_file(swap_server_template.pc.in swap_server_template.pc @ONLY)
target_include_directories(swap_server_template PRIVATE ${PROJECT_SOURCE_DIR}/deps/template/include)
install(TARGETS swap_server_template
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


install(FILES ${CMAKE_BINARY_DIR}/swap_server_template.pc
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)





