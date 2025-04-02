..
    Licensed under the MIT License.
    For details on the licensing terms, see the LICENSE file.
    SPDX-License-Identifier: MIT

   Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)

.. _Instantiate Module:

==================
Instantiate Module
==================

.. code-block:: c

    /* Method to crate an instance of the ModuleType, specified in the service_server pointer
     * Arguments:
     * UA_Server *server: server instance
     * UA_service_server_interpreter *service_server: server configuration, including the BrowseName of the SubType of the ModuleType,
     * as well as the target browseName of the instance
     * UA_NodeId *module_type_instance_nodeId: Pointer to a NodeId which will return the NodeId of the instance of the ModuleType
     */
    UA_StatusCode instantiate_module_type(UA_Server *server,
                                          UA_service_server_interpreter *service_server,
                                          UA_NodeId *module_type_instance_nodeId);

    /* Method to crate an instance of the ModuleType, specified in the service_server pointer
     * Arguments:
     * UA_Server *server: server instance
     * UA_service_server_interpreter *service_server: server configuration, including a list of capabilities
     * UA_NodeId module_type_instance_nodeId: The NodeId of the instance of the ModuleType
     */
    UA_StatusCode add_capability_nodes(UA_Server *server,
                                       UA_service_server_interpreter *service_server,
                                       UA_NodeId moduletype_instance_nodeId);

    /* Method to that sets the asset's state variable to idle
     * Arguments:
     * UA_Server *server: server instance
     */
    UA_StatusCode write_state_variable(UA_Server *server);

    /* Method to that sets the asset's registered variable to false
     * Arguments:
     * UA_Server *server: server instance
     */
    UA_StatusCode write_registered_variable(UA_Server *server);
