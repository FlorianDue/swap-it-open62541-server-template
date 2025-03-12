/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 * Copyright 2024 (c) Fraunhofer IWU (Author: Thorben Schnellhardt)
 */

#ifndef OPEN62541_QUEUE_HANDLER_H
#define OPEN62541_QUEUE_HANDLER_H

#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include "types_common_generated_handling.h"

typedef struct{
    UA_Queue_Data_Type *item_list;
    size_t nbr_items;
    UA_DataType Queue_Variable_Data_Type;
} UA_queue_container;

typedef struct{
    UA_Queue_State_Variable_Type next_state;
    UA_String client_identifier;
    UA_String Service_UUID;
}UA_change_queue_element_state_container;

typedef struct{
    size_t nbr_entries;
    UA_change_queue_element_state_container *entries;
}UA_Change_entry_list;

typedef struct{
    UA_Boolean write_struct_values;
    UA_Change_entry_list entry_list;
    UA_queue_container add_items;
    UA_queue_container remove_items;
    UA_Server *server;
    UA_NodeId Module_Object_NodeId;
}UA_Queue_handle;

typedef struct{
    UA_Queue_handle *handler;
    UA_Change_entry_list *entry_list;
    UA_queue_container *add_items;
    UA_queue_container *remove_items;
    UA_Boolean *running;
    pthread_t *thread_id;
}UA_Queue_observer;

UA_StatusCode add_queue_handler(UA_Server *server, char *method_node,
                  UA_queue_container *method_context);

UA_StatusCode set_queue_entry_state_methodCallback(UA_Server *server,
                                     const UA_NodeId *sessionId, void *sessionHandle,
                                     const UA_NodeId *methodId, void *methodContext,
                                     const UA_NodeId *objectId, void *objectContext,
                                     size_t inputSize, const UA_Variant *input,
                                     size_t outputSize, UA_Variant *output);

UA_StatusCode copy_and_clear_queue_handler_lists(UA_queue_container *add_items,
                                   UA_queue_container *remove_item, UA_Queue_handle *handler, UA_Change_entry_list *entry_list);

void *handle_queue(void *args);

void *observer_queue(void *args);

UA_StatusCode start_queue_handler(UA_Server *server, UA_NodeId module_object_nodeId, UA_Boolean *running);

#endif  // OPEN62541_QUEUE_HANDLER_H
