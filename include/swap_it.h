/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 * Copyright 2024 (c) Fraunhofer IWU (Author: Thorben Schnellhardt)
 */

#ifndef SWAP_TEMPLATE_UTILITY_FUNCTION_SWAP_IT_H
#define SWAP_TEMPLATE_UTILITY_FUNCTION_SWAP_IT_H

#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include "config_interpreter.h"

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
*/

UA_StatusCode UA_server_swap_it(UA_Server *server,
                                UA_ByteString json,
                                UA_MethodCallback callback,
                                UA_Boolean default_behavior,
                                UA_Boolean *running,
                                UA_Boolean register_agent_in_registry,
                                UA_service_server_interpreter *swap_server);

/*
 * UA_service_server_interpreter *server_info       Filled structure from the function UA_server_swap_it
 * UA_Boolean unregister                            Boolean value that determines whether the server should
 *                                                  unregister itself from the device registry
 * UA_Server *server                                the server instance
*/

void clear_swap_server(UA_service_server_interpreter *server_info, UA_Boolean unregister, UA_Server *server);

#endif //SWAP_TEMPLATE_UTILITY_FUNCTION_SWAP_IT_H
