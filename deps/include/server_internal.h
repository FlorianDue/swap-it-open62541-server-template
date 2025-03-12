/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 * Copyright 2024 (c) Fraunhofer IWU (Author: Thorben Schnellhardt)
 */

#ifndef OPEN62541_SERVER_INTERNAL_H
#define OPEN62541_SERVER_INTERNAL_H
#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include "types_common_generated.h"
#include "types_common_generated_handling.h"
#include "config_interpreter.h"

UA_StatusCode add_register_methods(UA_Server *server, UA_NodeId *register_function_nodeId, UA_NodeId *unregister_function_nodeId);
UA_StatusCode find_method(UA_Server *server, char *methodname, UA_NodeId *service_method_nodeId);
UA_StatusCode instantiate_module_type(UA_Server *server, UA_service_server_interpreter *service_server, UA_NodeId *module_type_instance_nodeId);
UA_StatusCode add_capability_nodes(UA_Server *server, UA_service_server_interpreter *service_server, UA_NodeId moduletype_instance_nodeId);
UA_StatusCode write_state_variable(UA_Server *server);
UA_StatusCode write_registered_variable(UA_Server *server, UA_Boolean value);
UA_StatusCode add_subscription_objects(UA_Server *server, UA_service_server_interpreter *service_server, UA_NodeId moduletype_instance_nodeId);
UA_StatusCode add_method_callback(UA_Server *server, UA_service_server_interpreter *service_server, UA_MethodCallback callback);

#endif  // OPEN62541_SERVER_INTERNAL_H
