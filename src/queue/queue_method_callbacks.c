/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 */

#include "queue_method_callbacks.h"
#include "stdio.h"

UA_StatusCode add_to_queue_method_callback(UA_Server *server,
                             const UA_NodeId *sessionId, void *sessionHandle,
                             const UA_NodeId *methodId, void *methodContext,
                             const UA_NodeId *objectId, void *objectContext,
                             size_t inputSize, const UA_Variant *input,
                             size_t outputSize, UA_Variant *output){
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_Queue_Data *queue_handler_list = (UA_Queue_Data *) methodContext;
    UA_Queue_List_Element *list = UA_calloc(1, sizeof(UA_Queue_List_Element));
    retval = UA_Queue_Data_Type_copy((UA_Queue_Data_Type*) input->data, &list->queue_element);
    if(retval != UA_STATUSCODE_GOOD){
        UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to copy the Queue Element");
        return retval;
    }
    /*add the next element to the head of the list*/
    SLIST_INSERT_HEAD(&queue_handler_list->queue_element_list, list, next);
    queue_handler_list->ll_length++;
    return retval;
}

UA_StatusCode remove_from_queue_method_callback(UA_Server *server,
                             const UA_NodeId *sessionId, void *sessionHandle,
                             const UA_NodeId *methodId, void *methodContext,
                             const UA_NodeId *objectId, void *objectContext,
                             size_t inputSize, const UA_Variant *input,
                             size_t outputSize, UA_Variant *output){
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_Queue_Data_Type element = *(UA_Queue_Data_Type *) input->data;
    UA_Queue_Data *queue_handler_list = (UA_Queue_Data *) methodContext;
    /*iterate through list and remove element*/
    UA_Queue_List_Element *current, *next_element;
    UA_Boolean found = false;
    SLIST_FOREACH_SAFE(current, &queue_handler_list->queue_element_list, next, next_element){
        if (UA_String_equal(&element.service_UUID, &current->queue_element.service_UUID) && UA_String_equal(&element.orderId, &current->queue_element.orderId)){
            UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Remove element with service uuid %.*s and order Id %.*s",(int) element.service_UUID.length, (char*) element.service_UUID.data, (int) element.orderId.length, (char*) element.orderId.data);
            found = true;
            SLIST_REMOVE(&queue_handler_list->queue_element_list, current, UA_Queue_List_Element, next);
            queue_handler_list->ll_length--;
            UA_Queue_Data_Type_clear(&current->queue_element);
            free(current);
            break;
        }
    }
    if (!found){
        UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to remove  element with service uuid %.*s and order Id %.*s", (int) element.service_UUID.length, (char*) element.service_UUID.data, (int) element.orderId.length, (char*) element.orderId.data);
    }
    return retval;
}

UA_StatusCode set_queue_element_state_method_callback(UA_Server *server,
                             const UA_NodeId *sessionId, void *sessionHandle,
                             const UA_NodeId *methodId, void *methodContext,
                             const UA_NodeId *objectId, void *objectContext,
                             size_t inputSize, const UA_Variant *input,
                             size_t outputSize, UA_Variant *output){
    UA_Queue_Data *queue_handler_list = (UA_Queue_Data *) methodContext;
    if (queue_handler_list->set_state)
        return UA_STATUSCODE_BADWAITINGFORRESPONSE;
    UA_StatusCode retval = UA_Set_Queue_Element_State_Data_Type_copy((UA_Set_Queue_Element_State_Data_Type *) input->data, &queue_handler_list->element_state);
    if (retval != UA_STATUSCODE_GOOD)
        return retval;
    queue_handler_list->set_state = true;
    UA_Variant out;
    UA_Variant_init(&out);
    UA_Server_readValue(server, queue_handler_list->queue_variable, &out);
    UA_Variant_clear(&out);
    return retval;
}

UA_StatusCode move_queue_element_method_callback(UA_Server *server,
                             const UA_NodeId *sessionId, void *sessionHandle,
                             const UA_NodeId *methodId, void *methodContext,
                             const UA_NodeId *objectId, void *objectContext,
                             size_t inputSize, const UA_Variant *input,
                             size_t outputSize, UA_Variant *output){

    UA_Queue_Data *queue_handler_list = (UA_Queue_Data *) methodContext;
    if(queue_handler_list->move)
        return UA_STATUSCODE_BADWAITINGFORRESPONSE;
    UA_StatusCode retval = UA_Move_Queue_Data_Type_copy((UA_Move_Queue_Data_Type*) input->data, &queue_handler_list->move_element);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    queue_handler_list->move = true;
    UA_Variant *out = UA_Variant_new();
    UA_Server_readValue(server, queue_handler_list->queue_variable, out);
    UA_Variant_delete(out);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode sort_queue_elements_method_callback(UA_Server *server,
                             const UA_NodeId *sessionId, void *sessionHandle,
                             const UA_NodeId *methodId, void *methodContext,
                             const UA_NodeId *objectId, void *objectContext,
                             size_t inputSize, const UA_Variant *input,
                             size_t outputSize, UA_Variant *output){
    UA_Queue_Data *queue_handler_list = (UA_Queue_Data *) methodContext;
    if(queue_handler_list->prioritization)
        return UA_STATUSCODE_BADWAITINGFORRESPONSE;
    UA_StatusCode retval = UA_Array_copy((UA_Change_Queue_Data_Type*) input->data, input->arrayLength, (void**) &queue_handler_list->prioritization_list,  &UA_TYPES_COMMON[UA_TYPES_COMMON_CHANGE_QUEUE_DATA_TYPE]);
    /*UA_StatusCode retval = UA_Change_Queue_Data_Type_copy((UA_Change_Queue_Data_Type*) input->data, &queue_handler_list->prioritization_list);*/
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    queue_handler_list->prioritization = true;
    queue_handler_list->priorization_list_size = input->arrayLength;
    UA_Variant *out = UA_Variant_new();
    UA_Server_readValue(server, queue_handler_list->queue_variable, out);
    UA_Variant_delete(out);
    return UA_STATUSCODE_GOOD;
}