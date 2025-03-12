/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 * Copyright 2024 (c) Fraunhofer IWU (Author: Thorben Schnellhardt)
 */

#include "node_finder.h"
#include <open62541/client_highlevel.h>

typedef struct{
    UA_QualifiedName variable_name;
    UA_NodeId variable_nodeId;
    UA_Server *server;
}UA_get_variable;

typedef struct{
    UA_QualifiedName variable_name;
    UA_NodeId variable_nodeId;
    UA_Client *client;
}UA_client_get_variable;

typedef struct{
    UA_NodeId object_type_id;
    UA_String object_type_name;
    UA_Server *server;
} UA_object_type_handler;

typedef struct{
    UA_NodeId data_type_id;
    UA_String data_type_name;
    UA_String data_type_parent_name;
    UA_Server *server;
}UA_data_type_handler;

static UA_StatusCode check_children(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
    if(isInverse){
        return UA_STATUSCODE_GOOD;
    }
    UA_object_type_handler *node_handler = (UA_object_type_handler *) handle;
    UA_QualifiedName bn;
    UA_Server_readBrowseName(node_handler->server, childId, &bn);
    if(UA_String_equal(&bn.name, &node_handler->object_type_name)){
        UA_NodeId_copy(&childId, &node_handler->object_type_id);
    }
    UA_QualifiedName_clear(&bn);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode get_module_type(UA_Server *server, UA_NodeId *module_type_nodeId, UA_String module_type_name, UA_NodeId startNode){
    UA_object_type_handler handler;
    UA_String_copy(&module_type_name, &handler.object_type_name);
    handler.server = server;
    UA_Server_forEachChildNodeCall(server, startNode, check_children, (void*) &handler);
    UA_NodeId_copy(&handler.object_type_id, module_type_nodeId);
    UA_String_clear(&handler.object_type_name);
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode get_event(UA_Server *server, UA_NodeId *event_type_nodeId, UA_String event_name, UA_NodeId startNode){
    UA_object_type_handler handler;
    UA_String_copy(&event_name, &handler.object_type_name);
    handler.server = server;
    UA_Server_forEachChildNodeCall(server, startNode, check_children, (void*) &handler);
    UA_NodeId_copy(&handler.object_type_id, event_type_nodeId);
    UA_String_clear(&handler.object_type_name);
    UA_NodeId_clear(&handler.object_type_id);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode find_service_event(UA_Server *server, UA_NodeId *EventNodeId, UA_String service_name){
    UA_String event_name = UA_String_fromChars("ServiceFinishedEventType");
    UA_NodeId service_finished_event_nodeId;
    get_event(server, &service_finished_event_nodeId, event_name, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE));
    get_event(server, EventNodeId, service_name, service_finished_event_nodeId);
    UA_String_clear(&event_name);
    UA_NodeId_clear(&service_finished_event_nodeId);
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode get_variable(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
    if(isInverse){
        return UA_STATUSCODE_GOOD;
    }
    UA_NodeId has_component_nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    UA_NodeId organizes_nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    if(!UA_NodeId_equal(&referenceTypeId, &has_component_nodeId)){
        if(!UA_NodeId_equal(&referenceTypeId, &organizes_nodeId)){
            UA_NodeId_clear(&has_component_nodeId);
            UA_NodeId_clear(&organizes_nodeId);
            return UA_STATUSCODE_GOOD;
        }
    }
    if(!UA_NodeId_equal(&referenceTypeId, &organizes_nodeId)){
        if(!UA_NodeId_equal(&referenceTypeId, &has_component_nodeId)){
            UA_NodeId_clear(&has_component_nodeId);
            UA_NodeId_clear(&organizes_nodeId);
            return UA_STATUSCODE_GOOD;
        }
    }
    UA_NodeId server_nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER);
    UA_NodeId aliases_nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ALIASES);
    /*UA_NodeId location_nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_LOCATIONS);*/
    if(UA_NodeId_equal(&childId, &server_nodeId) || UA_NodeId_equal(&childId, &aliases_nodeId) /*|| UA_NodeId_equal(&childId, &location_nodeId)*/){
        UA_NodeId_clear(&has_component_nodeId);
        UA_NodeId_clear(&organizes_nodeId);
        UA_NodeId_clear(&server_nodeId);
        UA_NodeId_clear(&aliases_nodeId);
        /*UA_NodeId_clear(&location_nodeId);*/
        return UA_STATUSCODE_GOOD;
    }
    UA_get_variable *node_handler = (UA_get_variable*) handle;
    UA_QualifiedName bn;
    UA_Server_readBrowseName(node_handler->server, childId, &bn);
    if(UA_String_equal(&bn.name, &node_handler->variable_name.name)){
        UA_NodeId_copy(&childId, &node_handler->variable_nodeId);
    }
    else{
        UA_Server_forEachChildNodeCall(node_handler->server, childId, get_variable, (void*) node_handler);
    }
    UA_QualifiedName_clear(&bn);
    UA_NodeId_clear(&has_component_nodeId);
    UA_NodeId_clear(&organizes_nodeId);
    UA_NodeId_clear(&server_nodeId);
    UA_NodeId_clear(&aliases_nodeId);
    /*UA_NodeId_clear(&location_nodeId);*/
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode client_get_variable(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
    if(isInverse){
        return UA_STATUSCODE_GOOD;
    }
    UA_NodeId has_component_nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    UA_NodeId organizes_nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    if(!UA_NodeId_equal(&referenceTypeId, &has_component_nodeId)){
        if(!UA_NodeId_equal(&referenceTypeId, &organizes_nodeId)){
            UA_NodeId_clear(&has_component_nodeId);
            UA_NodeId_clear(&organizes_nodeId);
            return UA_STATUSCODE_GOOD;
        }
    }
    if(!UA_NodeId_equal(&referenceTypeId, &organizes_nodeId)){
        if(!UA_NodeId_equal(&referenceTypeId, &has_component_nodeId)){
            UA_NodeId_clear(&has_component_nodeId);
            UA_NodeId_clear(&organizes_nodeId);
            return UA_STATUSCODE_GOOD;
        }
    }
    UA_NodeId server_nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER);
    UA_NodeId aliases_nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ALIASES);
    /*UA_NodeId location_nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_LOCATIONS);*/
    if(UA_NodeId_equal(&childId, &server_nodeId) || UA_NodeId_equal(&childId, &aliases_nodeId) /*|| UA_NodeId_equal(&childId, &location_nodeId)*/){
        UA_NodeId_clear(&has_component_nodeId);
        UA_NodeId_clear(&organizes_nodeId);
        UA_NodeId_clear(&server_nodeId);
        UA_NodeId_clear(&aliases_nodeId);
        /*UA_NodeId_clear(&location_nodeId);*/
        return UA_STATUSCODE_GOOD;
    }
    UA_client_get_variable *node_handler = (UA_client_get_variable*) handle;
    UA_QualifiedName bn;
    UA_QualifiedName_init(&bn);
    UA_Client_readBrowseNameAttribute(node_handler->client, childId, &bn);
    if(UA_String_equal(&bn.name, &node_handler->variable_name.name)){
        UA_NodeId_copy(&childId, &node_handler->variable_nodeId);
    }
    else{
        UA_Client_forEachChildNodeCall(node_handler->client, childId, client_get_variable, (void*) node_handler);
    }
    UA_QualifiedName_clear(&bn);
    UA_NodeId_clear(&has_component_nodeId);
    UA_NodeId_clear(&organizes_nodeId);
    UA_NodeId_clear(&server_nodeId);
    UA_NodeId_clear(&aliases_nodeId);
    /*UA_NodeId_clear(&location_nodeId);*/
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode get_single_node(UA_Server *server, UA_QualifiedName node_name, UA_NodeId *node_nodeId){
    UA_get_variable handle;
    memset(&handle,0, sizeof(UA_get_variable));
    handle.server = server;
    UA_QualifiedName_copy(&node_name, &handle.variable_name);
    UA_Server_forEachChildNodeCall(server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), get_variable, (void*) &handle);
    UA_NodeId_copy(&handle.variable_nodeId, node_nodeId);
    UA_QualifiedName_clear(&handle.variable_name);
    UA_NodeId_clear(&handle.variable_nodeId);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode client_get_single_node(UA_Client *client, UA_QualifiedName node_name, UA_NodeId *node_nodeId){
    UA_client_get_variable handle;
    memset(&handle,0, sizeof(UA_client_get_variable));
    handle.client = client;
    UA_QualifiedName_copy(&node_name, &handle.variable_name);
    UA_Client_forEachChildNodeCall(client, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), client_get_variable, (void*) &handle);
    UA_NodeId_copy(&handle.variable_nodeId, node_nodeId);
    UA_QualifiedName_clear(&handle.variable_name);
    UA_NodeId_clear(&handle.variable_nodeId);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode get_object_node(UA_Server *server, UA_NodeId ModuletypeInstanceNodeId, UA_NodeId *ObjectNodeNodeId, char *name){
    UA_object_type_handler handler;
    UA_String node_name = UA_String_fromChars(name);
    UA_String_copy(&node_name, &handler.object_type_name);
    handler.server = server;
    UA_Server_forEachChildNodeCall(server, ModuletypeInstanceNodeId, check_children, (void*) &handler);
    UA_NodeId_copy(&handler.object_type_id, ObjectNodeNodeId);
    UA_NodeId_clear(&handler.object_type_id);
    UA_String_clear(&handler.object_type_name);
    UA_String_clear(&node_name);
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode get_structure_data_type(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
    UA_NodeId subtype_nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE);
    if(!UA_NodeId_equal(&referenceTypeId, &subtype_nodeId)){
        UA_NodeId_clear(&subtype_nodeId);
        return UA_STATUSCODE_GOOD;
    }
    UA_data_type_handler *handler = (UA_data_type_handler*) handle;
    UA_QualifiedName qname;
    UA_QualifiedName_init(&qname);
    UA_Server_readBrowseName(handler->server, childId, &qname);
    if(UA_String_equal(&handler->data_type_name, &qname.name)){
        UA_NodeId_copy(&childId, &handler->data_type_id);
    }
    else if(UA_String_equal(&handler->data_type_parent_name, &qname.name)){
        UA_Server_forEachChildNodeCall(handler->server, childId, get_structure_data_type, handler);
    }
    UA_NodeId_clear(&subtype_nodeId);
    UA_QualifiedName_clear(&qname);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode server_get_data_type_node(UA_Server *server, UA_String data_type_name, UA_NodeId *data_type_nodeId, UA_String super_type_node){
    UA_data_type_handler handler;
    memset(&handler, 0, sizeof(UA_data_type_handler));
    UA_String_copy(&data_type_name, &handler.data_type_name);
    UA_String_copy(&super_type_node, &handler.data_type_parent_name);
    handler.server = server;
    UA_Server_forEachChildNodeCall(server, UA_NODEID_NUMERIC(0, UA_NS0ID_STRUCTURE), get_structure_data_type, &handler);
    UA_NodeId_copy(&handler.data_type_id, data_type_nodeId);
    UA_String_clear(&handler.data_type_parent_name);
    UA_String_clear(&handler.data_type_name);
    UA_NodeId_clear(&handler.data_type_id);
    return UA_STATUSCODE_GOOD;
}