#[[
 Licensed under the MIT License.
 For details on the licensing terms, see the LICENSE file.
 SPDX-License-Identifier: MIT

 Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
]]
project(unit_tests)
cmake_minimum_required(VERSION 3.0.0)
find_package(open62541 1.3 REQUIRED)


set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../cmake")
find_package(Check REQUIRED)
set(LIBS ${CHECK_LIBRARIES} ${open62541_LIBRARIES})
include_directories(${CHECK_INCLUDE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests)
set(TESTS_BINARY_DIR ${CMAKE_BINARY_DIR}/bin/tests)

file(MAKE_DIRECTORY "${NODESET_COMPILER_OUTPUT_DIR}")
set(NODESET_COMPILER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/src_generated")
include_directories("${NODESET_COMPILER_OUTPUT_DIR}")

set(INFORMATION_MODEL_DIR ${PROJECT_SOURCE_DIR}/../../model)
ua_generate_nodeset_and_datatypes(
        NAME "common"
        FILE_BSD "${INFORMATION_MODEL_DIR}/SWAP.Fraunhofer.Common.Model.Types.bsd"
        FILE_CSV "${INFORMATION_MODEL_DIR}/CommonModelDesign.csv"
        FILE_NS "${INFORMATION_MODEL_DIR}/SWAP.Fraunhofer.Common.Model.NodeSet2.xml"
        NAMESPACE_MAP "2:http://common.swap.fraunhofer.de"
        OUTPUT_DIR ${GENERATE_OUTPUT_DIR}
        INTERNAL
)
#generate c code from the pfdl types model
ua_generate_nodeset_and_datatypes(
        NAME "pfdl_parameter"
        FILE_BSD "${INFORMATION_MODEL_DIR}/SWAP.Fraunhofer.Demo.Scenario.Model.Types.bsd"
        FILE_CSV "${INFORMATION_MODEL_DIR}/DemoScenarioTypes.ModelDesign.csv"
        FILE_NS "${INFORMATION_MODEL_DIR}/SWAP.Fraunhofer.Demo.Scenario.Model.NodeSet2.xml"
        NAMESPACE_MAP "3:http://swap.demo.scenario.fraunhofer.de"
        OUTPUT_DIR ${GENERATE_OUTPUT_DIR}
        INTERNAL
)
#generate c code from the warehouse model and set the dependencies to the common and pfdl types models
ua_generate_nodeset_and_datatypes(
        NAME "warehouse"
        DEPENDS "common"
        DEPENDS "pfdl_parameter"
        FILE_BSD "${INFORMATION_MODEL_DIR}/SWAP.Fraunhofer.Demo.Scenario.Warehouse.Model.Types.bsd"
        FILE_CSV "${INFORMATION_MODEL_DIR}/DemoScenarioWarehouse.ModelDesign.csv"
        FILE_NS "${INFORMATION_MODEL_DIR}/SWAP.Fraunhofer.Demo.Scenario.Warehouse.Model.NodeSet2.xml"
        IMPORT_BSD "TYPES_COMMON#${INFORMATION_MODEL_DIR}/SWAP.Fraunhofer.Common.Model.Types.bsd"
        IMPORT_BSD "TYPES_PFDL_PARAMETER#${INFORMATION_MODEL_DIR}/SWAP.Fraunhofer.Demo.Scenario.Model.Types.bsd"
        NAMESPACE_MAP "4:http://swap.demo.scenario.warehouse.fraunhofer.de"
        OUTPUT_DIR ${GENERATE_OUTPUT_DIR}
        INTERNAL
)

add_executable(unit_tests
        ../deps/service_callbacks.c
        ../check_server_template.c
        ${UA_NODESET_PFDL_PARAMETER_SOURCES}
        ${UA_TYPES_PFDL_PARAMETER_SOURCES}
        ${UA_NODESET_COMMON_SOURCES}
        ${UA_TYPES_COMMON_SOURCES}
        ${UA_NODESET_WAREHOUSE_SOURCES}
        ${UA_TYPES_WAREHOUSE_SOURCES}
)

add_dependencies(unit_tests  open62541-generator-ns-common
                        open62541-generator-ns-pfdl_parameter
                        open62541-generator-ns-warehouse
)

target_link_libraries(unit_tests swap_server_template)
target_link_libraries(unit_tests open62541::open62541)
target_link_libraries(unit_tests pthread)
target_link_libraries(unit_tests check)
target_link_libraries(unit_tests subunit)
target_link_libraries(unit_tests m)