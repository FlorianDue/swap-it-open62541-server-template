#[[
Licensed under the MIT License.
For details on the licensing terms, see the LICENSE file.
SPDX-License-Identifier: MIT

Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
]]

cmake_minimum_required(VERSION 3.0.0)
#set name of the CMakeProject
project(example_server)
find_package(open62541 1.3 REQUIRED)
if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # needed or cmake doesn't recognize dependencies of generated files
    set(PROJECT_BINARY_DIR ${CMAKE_BINARY_DIR})
endif()
#define the path to the directory in which the
set(PATH_TO_INFORMATION_MODELS ${PROJECT_SOURCE_DIR}/../model)
#set output directory
set(GENERATE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/src_generated/")
include_directories("${GENERATE_OUTPUT_DIR}")
#generate c code from the common model
ua_generate_nodeset_and_datatypes(
        NAME "common"
        FILE_BSD "${PATH_TO_INFORMATION_MODELS}/SWAP.Fraunhofer.Common.Model.Types.bsd"
        FILE_CSV "${PATH_TO_INFORMATION_MODELS}/CommonModelDesign.csv"
        FILE_NS "${PATH_TO_INFORMATION_MODELS}/SWAP.Fraunhofer.Common.Model.NodeSet2.xml"
        NAMESPACE_MAP "2:http://common.swap.fraunhofer.de"
        OUTPUT_DIR ${GENERATE_OUTPUT_DIR}
        INTERNAL
)
#generate c code from the pfdl types model
ua_generate_nodeset_and_datatypes(
        NAME "pfdl_parameter"
        FILE_BSD "${PATH_TO_INFORMATION_MODELS}/SWAP.Fraunhofer.Demo.Scenario.Model.Types.bsd"
        FILE_CSV "${PATH_TO_INFORMATION_MODELS}/DemoScenarioTypes.ModelDesign.csv"
        FILE_NS "${PATH_TO_INFORMATION_MODELS}/SWAP.Fraunhofer.Demo.Scenario.Model.NodeSet2.xml"
        NAMESPACE_MAP "3:http://swap.demo.scenario.fraunhofer.de"
        OUTPUT_DIR ${GENERATE_OUTPUT_DIR}
        INTERNAL
)
#generate c code from the warehouse model and set the dependencies to the common and pfdl types models
ua_generate_nodeset_and_datatypes(
        NAME "warehouse"
        DEPENDS "common"
        DEPENDS "pfdl_parameter"
        FILE_BSD "${PATH_TO_INFORMATION_MODELS}/SWAP.Fraunhofer.Demo.Scenario.Warehouse.Model.Types.bsd"
        FILE_CSV "${PATH_TO_INFORMATION_MODELS}/DemoScenarioWarehouse.ModelDesign.csv"
        FILE_NS "${PATH_TO_INFORMATION_MODELS}/SWAP.Fraunhofer.Demo.Scenario.Warehouse.Model.NodeSet2.xml"
        IMPORT_BSD "TYPES_COMMON#${PATH_TO_INFORMATION_MODELS}/SWAP.Fraunhofer.Common.Model.Types.bsd"
        IMPORT_BSD "TYPES_PFDL_PARAMETER#${PATH_TO_INFORMATION_MODELS}/SWAP.Fraunhofer.Demo.Scenario.Model.Types.bsd"
        NAMESPACE_MAP "4:http://swap.demo.scenario.warehouse.fraunhofer.de"
        OUTPUT_DIR ${GENERATE_OUTPUT_DIR}
        INTERNAL
)
#adds the executable target
add_executable(example_server
        main.c
        ${UA_NODESET_PFDL_PARAMETER_SOURCES}
        ${UA_TYPES_PFDL_PARAMETER_SOURCES}
        ${UA_NODESET_COMMON_SOURCES}
        ${UA_TYPES_COMMON_SOURCES}
        ${UA_NODESET_WAREHOUSE_SOURCES}
        ${UA_TYPES_WAREHOUSE_SOURCES}
)
#add the dependencies between the beginner tutorial and the generated namespaces
add_dependencies(example_server open62541-generator-ns-common
        open62541-generator-ns-pfdl_parameter
        open62541-generator-ns-warehouse
)
#link the beginner tutorial project with the installed open62541 server template
target_link_libraries(example_server swap_server_template)
#link the beginner tutorial project with the installed open62541 SDK
target_link_libraries(example_server open62541::open62541)
