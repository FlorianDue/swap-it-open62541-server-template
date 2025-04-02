..
    Licensed under the MIT License.
    For details on the licensing terms, see the LICENSE file.
    SPDX-License-Identifier: MIT

   Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)

.. _Register and Unregister Methods:

===============================
Register and Unregister Methods
===============================

.. code-block:: c

    /* pre-defined method callback to register a server in the Device Registry */
    UA_StatusCode register_method_callback(UA_Server *server,
                                                  const UA_NodeId *sessionId, void *sessionHandle,
                                                  const UA_NodeId *methodId, void *methodContext,
                                                  const UA_NodeId *objectId, void *objectContext,
                                                  size_t inputSize, const UA_Variant *input,
                                                  size_t outputSize, UA_Variant *output);

    /* pre-defined method callback to unregister a server from the Device Registry */
    UA_StatusCode unregister_method_callback(UA_Server *server,
                                             const UA_NodeId *sessionId, void *sessionHandle,
                                             const UA_NodeId *methodId, void *methodContext,
                                             const UA_NodeId *objectId, void *objectContext,
                                             size_t inputSize, const UA_Variant *input,
                                             size_t outputSize, UA_Variant *output);

    /* Link the register and unregister method callbacks to the corresponding methods
     * Arguments:
     * UA_Server *server: server instance
     * UA_NodeId *register_function_nodeId: Pointer to a NodeId which will return thr NodeId of the register method
     * UA_NodeId *unregister_function_nodeId: Pointer to a NodeId which will return thr NodeId of the unregister method
     */
    UA_StatusCode add_register_methods(UA_Server *server,
                                       UA_NodeId *register_function_nodeId,
                                       UA_NodeId *unregister_function_nodeId);

    /* searches a Method Node based on its BrowseName.Name. As the ModuleType only features HasComponent, Organizes and HasProperty references
     * the browsing for nodes will only respect these references
     * Arguments:
     * UA_Server *server: server instance
     * char *methodname: BrowseName.Name of the method
     * UA_NodeId *service_method_nodeId: Pointer to a NodeId which will return thr NodeId of the requested method
     */
    UA_StatusCode find_method(UA_Server *server,
                              char *methodname,
                              UA_NodeId *service_method_nodeId);

    /* Links a method callback to the service method of a server
     * Arguments:
     * UA_Server *server: server instance
     * UA_service_server_interpreter *service_server: server configuration, including the BrowseName of the service node
     * UA_MethodCallback callback: callback to be linked to the service method
     */
    UA_StatusCode add_method_callback(UA_Server *server, UA_service_server_interpreter *service_server, UA_MethodCallback callback);
