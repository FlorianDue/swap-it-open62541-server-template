/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 */

#ifndef OPEN62541_NODE_FINDER_H
#define OPEN62541_NODE_FINDER_H
#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>

UA_StatusCode server_get_data_type_node(UA_Server *server, UA_String data_type_name, UA_NodeId *data_type_nodeId, UA_String super_type_node);

UA_StatusCode get_object_node(UA_Server *server, UA_NodeId ModuletypeInstanceNodeId, UA_NodeId *StateNodeNodeId, char *name);

UA_StatusCode get_module_type(UA_Server *server, UA_NodeId *module_type_nodeId, UA_String module_type_name, UA_NodeId startNode);

UA_StatusCode get_single_node(UA_Server *server, UA_QualifiedName node_name, UA_NodeId *node_nodeId);

UA_StatusCode client_get_single_node(UA_Client *client, UA_QualifiedName node_name, UA_NodeId *node_nodeId);

UA_StatusCode find_service_event(UA_Server *server, UA_NodeId *EventNodeId, UA_String service_name);

#endif  // OPEN62541_NODE_FINDER_H
