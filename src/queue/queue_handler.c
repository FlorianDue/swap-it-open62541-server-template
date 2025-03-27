/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 */
#include "write_queue.h"
#include "stdio.h"

static UA_StatusCode readQueue(UA_Server *server,
                const UA_NodeId *sessionId, void *sessionContext,
                const UA_NodeId *nodeId, void *nodeContext,
                UA_Boolean sourceTimeStamp, const UA_NumericRange *range,
                UA_DataValue *dataValue) {
    UA_Queue_Data *queue_handler_list = (UA_Queue_Data *) nodeContext;
    UA_Queue_List_Element *current;
    /*empty queue or only single element, or nor value change*/
    if(queue_handler_list->ll_length == queue_handler_list->queue_size || queue_handler_list->ll_length == 0 || queue_handler_list->ll_length == 1){
        if(queue_handler_list->ll_length == 0){
            if(queue_handler_list->set_state)
                changeQueueElementState(queue_handler_list, true);
            if(queue_handler_list->move)
                move_single_element(queue_handler_list, true);
            if(queue_handler_list->prioritization)
                sort_elements(queue_handler_list, true);
            //UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Queue is Empty");
            return UA_STATUSCODE_GOOD;
        }
        if(queue_handler_list->ll_length == 1){
            queue_handler_list->queue = UA_realloc(queue_handler_list->queue, queue_handler_list->ll_length * sizeof(UA_Queue_Data_Type));
            memset(queue_handler_list->queue, 0, queue_handler_list->ll_length * sizeof(UA_Queue_Data_Type));
            SLIST_FOREACH(current, &queue_handler_list->queue_element_list, next){
                UA_Queue_Data_Type_copy(&current->queue_element, &queue_handler_list->queue[queue_handler_list->ll_length - 1]);
                queue_handler_list->queue[queue_handler_list->ll_length - 1].entry_Number = 1;
                queue_handler_list->queue_size = queue_handler_list->ll_length;
                if(queue_handler_list->set_state)
                    changeQueueElementState(queue_handler_list, false);
                if(queue_handler_list->move)
                    move_single_element(queue_handler_list, true);
                if(queue_handler_list->prioritization)
                    sort_elements(queue_handler_list, true);
                /*provide the variable value*/
                UA_DataValue_init(dataValue);
                UA_Variant_setScalarCopy(&dataValue->value, &queue_handler_list->queue[0], &UA_TYPES_COMMON[UA_TYPES_COMMON_QUEUE_DATA_TYPE]);
                dataValue->hasValue = true;
            }
            //UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Single element Queue");
            return UA_STATUSCODE_GOOD;
        }
    }
    /*get the new queue from the linked list*/
    create_queue_from_linked_list(queue_handler_list);
    /*move single queue element*/
    if(queue_handler_list->move)
        move_single_element(queue_handler_list, false);
    /*check if an element needs a state update*/
    if(queue_handler_list->set_state)
        changeQueueElementState(queue_handler_list, false);
    /*check if the queu should be sorted*/
    if(queue_handler_list->prioritization)
        sort_elements(queue_handler_list, false);
    /*provide the variable value*/
    UA_DataValue_init(dataValue);
    UA_Variant_setArrayCopy(&dataValue->value, queue_handler_list->queue, queue_handler_list->queue_size, &UA_TYPES_COMMON[UA_TYPES_COMMON_QUEUE_DATA_TYPE]);
    dataValue->hasValue = true;
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode writeQueue(UA_Server *server,
                 const UA_NodeId *sessionId, void *sessionContext,
                 const UA_NodeId *nodeId, void *nodeContext,
                 const UA_NumericRange *range, const UA_DataValue *data) {
    UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "The Queue Variable is read-only. Please use the Methods to change the value");
    return UA_STATUSCODE_BADINTERNALERROR;
}

UA_StatusCode addQueueValueCallback(UA_Server *server, UA_NodeId queueVariableId){
    UA_DataSource source;
    source.read = readQueue;
    source.write = writeQueue;
    UA_StatusCode retval = UA_Server_setVariableNode_dataSource(server, queueVariableId, source);
    if(retval != UA_STATUSCODE_GOOD){
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Unable to set the QueueValueCallback");
        return retval;
    }
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "QueueValueCallback Added");
    return retval;
}

UA_StatusCode init_queue_data(UA_Server *server, UA_Queue_Data *queue_data){
    SLIST_INIT(&queue_data->queue_element_list);
    queue_data->move = false;
    UA_Move_Queue_Data_Type_init(&queue_data->move_element);
    queue_data->prioritization = false;
    queue_data->priorization_list_size = 0;
    queue_data->prioritization_list = UA_Array_new(0, &UA_TYPES_COMMON[UA_TYPES_COMMON_CHANGE_QUEUE_DATA_TYPE]);
    queue_data->set_state = false;
    UA_Set_Queue_Element_State_Data_Type_init(&queue_data->element_state);
    queue_data->ll_length = 0;
    queue_data->queue_size = 0;
    queue_data->queue = UA_Array_new(0, &UA_TYPES_COMMON[UA_TYPES_COMMON_QUEUE_DATA_TYPE]);
    /*add node context to the queue variable*/
    UA_NodeId_init(&queue_data->queue_variable);
    char *queue_name = "queue_variable";
    UA_StatusCode retval = find_method(server, queue_name, &queue_data->queue_variable);
    retval = UA_Server_setNodeContext(server, queue_data->queue_variable, (void*) queue_data);
    if(retval != UA_STATUSCODE_GOOD)
        UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to add the context for Node %s ", queue_name);
    add_queue_to_method_context(server, "add_queue_element", queue_data, add_to_queue_method_callback);
    add_queue_to_method_context(server, "remove_queue_element", queue_data, remove_from_queue_method_callback);
    add_queue_to_method_context(server, "set_queue_element_state", queue_data, set_queue_element_state_method_callback);
    add_queue_to_method_context(server, "move_queue_element", queue_data, move_queue_element_method_callback);
    add_queue_to_method_context(server, "sort_queue_elements", queue_data, sort_queue_elements_method_callback);
    retval = addQueueValueCallback(server, queue_data->queue_variable);
    return retval;
}




