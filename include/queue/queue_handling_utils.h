/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 */

#ifndef QUEUE_METHOD_HANDLER_H
#define QUEUE_METHOD_HANDLER_H

#include "open62541_queue.h"
#include "node_finder.h"
#include "server_internal.h"
#include "types_common_generated.h"
#include "types_common_generated_handling.h"

typedef struct UA_Queue_List_Element{
 SLIST_ENTRY(UA_Queue_List_Element) next;
 UA_Queue_Data_Type queue_element;
} UA_Queue_List_Element;

typedef struct{
 UA_NodeId queue_variable;
 UA_Boolean prioritization;
 UA_Change_Queue_Data_Type *prioritization_list;
 size_t priorization_list_size;
 UA_Boolean move;
 UA_Move_Queue_Data_Type move_element;
 UA_Boolean set_state;
 UA_Set_Queue_Element_State_Data_Type element_state;
 size_t ll_length;
 SLIST_HEAD(, UA_Queue_List_Element) queue_element_list;
 size_t queue_size;
 UA_Queue_Data_Type *queue;
} UA_Queue_Data;

UA_StatusCode changeQueueElementState(UA_Queue_Data *queue_handler_list, UA_Boolean empty_queue);
UA_StatusCode add_queue_to_method_context(UA_Server *server, char *method_node,
                  UA_Queue_Data *method_context, UA_MethodCallback callback);
UA_StatusCode move_single_element(UA_Queue_Data *queue_handler_list, UA_Boolean empty_queue);
UA_StatusCode create_linked_list_from_queue(UA_Queue_Data *queue_handler_list);
void create_queue_from_linked_list(UA_Queue_Data *queue_handler_list);
UA_StatusCode sort_elements(UA_Queue_Data *queue_handler_list, UA_Boolean empty_queue);
#endif //QUEUE_METHOD_HANDLER_H
