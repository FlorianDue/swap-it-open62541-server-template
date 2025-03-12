..
    Licensed under the MIT License.
    For details on the licensing terms, see the LICENSE file.
    SPDX-License-Identifier: MIT

   Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)

=================
UA_server_swap_it
=================

Instantiates the SubType of the ModuleType as configured in the JSON file. All functionalities, such as the :ref:`Register and Unregister Methods`,
the :ref:`Queue Handler` and the ServiceMethod callback are created in accordance, so that an automated configuration of the server for the SWAP-IT software is accomplished.
**Important Note**: The open62541 server template includes the Common Information Model, however if additional custom information models are required, the must be loaded into the server before invoking
the UA_server_swap_it function.

.. code-block:: c


    /*
     * Method Arguments:
     * UA_Server *server:                           the server instance
     * UA_ByteString json                           json configuration of the server
     * UA_MethodCallback callback:                  callback of for the service method
     * UA_Boolean default_behavior:                 boolean that determines whether a default information
     *                                              model will be instantiated based on the json configuration
     * UA_Boolean *running:                         running variable from the server's main function
     * UA_Boolean register_agent_in_registry:       Boolean that determines whether the Asset should register itself in a device registry
     * UA_service_server_interpreter *swap_server   Empty structure that stores the interpreted json configuration. The stucture is filled
     *                                              when interpreting the json config within the function and can then be used for the function
     *                                              clear_swap_server
     * */


    UA_StatusCode UA_server_swap_it(UA_Server *server,
                                UA_ByteString json,
                                UA_MethodCallback callback,
                                UA_Boolean default_behavior,
                                UA_Boolean *running,
                                UA_Boolean register_agent_in_registry,
                                UA_service_server_interpreter *swap_server);

=================
clear_swap_server
=================

Function that clears the swap server and in addition, can be used to unregister the server from a device registry

.. code-block:: c

    /*
     * UA_service_server_interpreter *server_info       Filled structure from the function UA_server_swap_it
     * UA_Boolean unregister                            Boolean value that determines whether the server should
     *                                                  unregister itself from the device registry
     * UA_Server *server                                the server instance
     */

    void clear_swap_server(UA_service_server_interpreter *server_info,
                           UA_Boolean unregister,
                           UA_Server *server);