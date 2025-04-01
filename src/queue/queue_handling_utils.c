/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 */

#include "queue_handling_utils.h"
#include "stdio.h"

UA_Queue_List_Element *get_list_element(UA_Queue_Data *queue_handler_list, UA_String orderId, UA_String serviceUUID){
    UA_Queue_List_Element *current, *next_element;
    SLIST_FOREACH_SAFE(current, &queue_handler_list->queue_element_list, next, next_element){
        if (UA_String_equal(&orderId, &current->queue_element.orderId) && UA_String_equal(&serviceUUID, &current->queue_element.service_UUID))
            return current;
    }
    return current;
}

void create_queue_from_linked_list(UA_Queue_Data *queue_handler_list){
    UA_Queue_Data_Type temp_queue_list[1000];
    if(queue_handler_list->ll_length != queue_handler_list->queue_size){
        size_t ctr = 0;
        UA_Queue_List_Element *current, *next_element;
        SLIST_FOREACH_SAFE(current, &queue_handler_list->queue_element_list, next, next_element){
            temp_queue_list[ctr] = current->queue_element;
            ctr++;
        }
        UA_Array_delete(queue_handler_list->queue, queue_handler_list->queue_size, &UA_TYPES_COMMON[UA_TYPES_COMMON_QUEUE_DATA_TYPE]);
        queue_handler_list->queue = UA_Array_new(queue_handler_list->ll_length, &UA_TYPES_COMMON[UA_TYPES_COMMON_QUEUE_DATA_TYPE]);
        ctr = 1;
        for(size_t i=queue_handler_list->ll_length; i>0; i--){
            UA_Queue_Data_Type_copy(&temp_queue_list[i-1], &queue_handler_list->queue[ctr-1]);
            queue_handler_list->queue[ctr-1].entry_Number = (UA_Int32) ctr;
            ctr++;
        }
        queue_handler_list->queue_size = queue_handler_list->ll_length;
    }
}


UA_StatusCode create_linked_list_from_queue(UA_Queue_Data *queue_handler_list){
    UA_StatusCode retval;
    if(queue_handler_list->queue_size != queue_handler_list->ll_length){
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Different sizes between Queue and Linked List. Unable to generate new Linked List");
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    UA_Queue_List_Element *current, *next_element;
    /*clear the current linked_list*/
    SLIST_FOREACH_SAFE(current, &queue_handler_list->queue_element_list, next, next_element){
        SLIST_REMOVE(&queue_handler_list->queue_element_list, current, UA_Queue_List_Element, next);
        queue_handler_list->ll_length--;
        UA_Queue_Data_Type_clear(&current->queue_element);
        free(current);
    }
    /*create the new linked list*/
    for(size_t i= queue_handler_list->queue_size; i>0; i--){
        UA_Queue_List_Element *list = UA_calloc(1, sizeof(UA_Queue_List_Element));
        retval = UA_Queue_Data_Type_copy(&queue_handler_list->queue[i-1], &list->queue_element);
        queue_handler_list->ll_length++;
        if(retval != UA_STATUSCODE_GOOD){
            UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to copy the Queue Element");
            return retval;
        }
        /*add the next element to the head of the list*/
        SLIST_INSERT_HEAD(&queue_handler_list->queue_element_list, list, next);
    }
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode sort_elements(UA_Queue_Data *queue_handler_list, UA_Boolean empty_queue){
    if(empty_queue == false){
        /*get the highest index of the prio list*/
        size_t highest_val = 0;
        size_t new_list_ctr = 0;
        size_t ctr = 1;
        UA_Boolean skip_first = false;
        size_t *copied_vals = (size_t*) UA_calloc(queue_handler_list->ll_length, sizeof(size_t));
        UA_Queue_Data_Type *new_list = UA_Array_new(queue_handler_list->ll_length, &UA_TYPES_COMMON[UA_TYPES_COMMON_QUEUE_DATA_TYPE]);
        for(size_t i=0; i<queue_handler_list->priorization_list_size; i++){
            highest_val = queue_handler_list->prioritization_list[i].prioritizationValue > highest_val ? queue_handler_list->prioritization_list[i].prioritizationValue : highest_val;
        }
        /*check if the first element is already in execution state, so that it is not considered for the ordering*/
        if(queue_handler_list->queue[0].queue_Element_State == UA_QUEUE_STATE_VARIABLE_TYPE_EXECUTING){
            skip_first = true;
            UA_Queue_Data_Type_copy(&queue_handler_list->queue[0], &new_list[new_list_ctr]);
            copied_vals[new_list_ctr] = 0;
            new_list_ctr++;
        }
        /*copy all priotized elements into the new list store the positions -> values that are not prioritized are copies in a separate loop to the end of the list */
        while(ctr <= highest_val){
            for(size_t i=0; i < queue_handler_list->priorization_list_size; i++){
                if(queue_handler_list->prioritization_list[i].prioritizationValue == ctr){
                    for(size_t j= skip_first == true? 1:0; j< queue_handler_list->queue_size; j++){
                        if(UA_String_equal(&queue_handler_list->prioritization_list[i].orderId, &queue_handler_list->queue[j].orderId)){
                            UA_Queue_Data_Type_copy(&queue_handler_list->queue[j], &new_list[new_list_ctr]);
                            copied_vals[new_list_ctr] = j;
                            new_list_ctr++;
                            break;
                        }
                    }
                }
            }
            ctr++;
        }
        /*copy elements without prioritization into the queue variable*/
        size_t copied_elements = new_list_ctr;
        if(queue_handler_list->queue_size != new_list_ctr){
            for(size_t i=0; i < queue_handler_list->queue_size; i++){
                UA_Boolean found = UA_FALSE;
                for(size_t j=0; j< copied_elements; j++){
                    if (i == copied_vals[j]){
                        found = UA_TRUE;
                        break;
                    }
                }
                if(found == UA_FALSE){
                    UA_Queue_Data_Type_copy(&queue_handler_list->queue[i], &new_list[new_list_ctr]);
                    new_list_ctr++;
                }
            }
        }
        /*correct the entry number*/
        for(size_t i=0; i< queue_handler_list->queue_size; i++){
            new_list[i].entry_Number = (UA_Int16) (i + 1);
        }
        /*clear the current queue*/
        UA_Array_delete(queue_handler_list->queue, queue_handler_list->queue_size, &UA_TYPES_COMMON[UA_TYPES_COMMON_QUEUE_DATA_TYPE]);
        queue_handler_list->queue_size = queue_handler_list->ll_length;
        queue_handler_list->queue = new_list;
        create_linked_list_from_queue(queue_handler_list);
        free(copied_vals);
    }
    UA_Array_resize((void**) &queue_handler_list->prioritization_list, &queue_handler_list->priorization_list_size, 0, &UA_TYPES_COMMON[UA_TYPES_COMMON_CHANGE_QUEUE_DATA_TYPE]);
    queue_handler_list->prioritization_list = UA_Change_Queue_Data_Type_new();
    queue_handler_list->priorization_list_size = 0;
    queue_handler_list->prioritization = false;
    return UA_STATUSCODE_GOOD;

}

UA_StatusCode move_single_element(UA_Queue_Data *queue_handler_list, UA_Boolean empty_queue){
    if(empty_queue == false){
        UA_Queue_Data_Type *new_queue = UA_calloc(queue_handler_list->queue_size, sizeof(UA_Queue_Data_Type));
        /*find the element that should be moved and copy it into the new queue*/
        size_t old_position = 0;
        for(size_t i=0; i<queue_handler_list->queue_size; i++){
            if(UA_String_equal(&queue_handler_list->queue[i].orderId, &queue_handler_list->move_element.orderId) &&
                UA_String_equal(&queue_handler_list->queue[i].service_UUID, &queue_handler_list->move_element.service_uuid)){
                old_position = i;
                /*case the new requested position is equal to the old position*/
                if (old_position == queue_handler_list->move_element.newPosition){
                    UA_Move_Queue_Data_Type_clear(&queue_handler_list->move_element);
                    UA_Move_Queue_Data_Type_init(&queue_handler_list->move_element);
                    queue_handler_list->move = false;
                    free(new_queue);
                    return UA_STATUSCODE_GOOD;
                }
                /*case an element that is currently executed should be replaced*/
                if(queue_handler_list->move_element.newPosition == 1 && queue_handler_list->queue[0].queue_Element_State == UA_QUEUE_STATE_VARIABLE_TYPE_EXECUTING){
                    UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Requested Element with service UUID %s and orderId %s can not replace element"
                                                                       " with element with orderId %s and service UUID %s since it is currently executed.",
                            (char*) queue_handler_list->move_element.orderId.data, (char*) queue_handler_list->move_element.service_uuid.data,
                            (char*) queue_handler_list->queue[0].orderId.data, (char*) queue_handler_list->queue[0].service_UUID.data);
                    free(new_queue);
                    UA_Move_Queue_Data_Type_clear(&queue_handler_list->move_element);
                    UA_Move_Queue_Data_Type_init(&queue_handler_list->move_element);
                    queue_handler_list->move = false;
                    return UA_STATUSCODE_GOOD;
                }
                UA_Queue_Data_Type_copy(&queue_handler_list->queue[i], &new_queue[queue_handler_list->move_element.newPosition-1]);
                new_queue[queue_handler_list->move_element.newPosition-1].entry_Number = queue_handler_list->move_element.newPosition;
                break;
            }
        }
        if(old_position == 0 && queue_handler_list->queue[0].queue_Element_State == UA_QUEUE_STATE_VARIABLE_TYPE_EXECUTING){
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Requested Element with service UUID %s and orderId %s is not in the Queue.",
                (char*) queue_handler_list->move_element.orderId.data, (char*) queue_handler_list->move_element.service_uuid.data);
            UA_Move_Queue_Data_Type_clear(&queue_handler_list->move_element);
            UA_Move_Queue_Data_Type_init(&queue_handler_list->move_element);
            queue_handler_list->move = false;
            free(new_queue);
            return UA_STATUSCODE_GOOD;
        }
        /*copy the remaining elements into the new queue*/
        size_t new_element_entry_nbr = 1;
        printf("old position %zu and new position %d \n", old_position, queue_handler_list->move_element.newPosition-1);
        for(size_t i=0; i<queue_handler_list->queue_size; i++){
            if(i != old_position){
                printf("current position %zu \n", i);
                if(old_position < queue_handler_list->move_element.newPosition-1){
                    UA_Queue_Data_Type_copy(&queue_handler_list->queue[i], &new_queue[(i > queue_handler_list->move_element.newPosition-1 || i < old_position) ? i : i - 1]);
                    new_queue[i].entry_Number = new_element_entry_nbr;
                    UA_Queue_Data_Type_clear(&queue_handler_list->queue[i]);
                }
                else if(old_position > queue_handler_list->move_element.newPosition-1){
                    UA_Queue_Data_Type_copy(&queue_handler_list->queue[i], &new_queue[(i < queue_handler_list->move_element.newPosition-1 || i > old_position ) ? i : i + 1]);
                    new_queue[i].entry_Number = new_element_entry_nbr;
                    UA_Queue_Data_Type_clear(&queue_handler_list->queue[i]);
                }
            }
            new_element_entry_nbr++;
        }
        free(queue_handler_list->queue);
        queue_handler_list->queue = new_queue;
        create_linked_list_from_queue(queue_handler_list);
    }
    UA_Move_Queue_Data_Type_clear(&queue_handler_list->move_element);
    UA_Move_Queue_Data_Type_init(&queue_handler_list->move_element);
    queue_handler_list->move = false;
    return UA_STATUSCODE_GOOD;
}

/*UA_StatusCode changeQueueElementState(UA_Queue_Data *queue_handler_list, UA_Boolean empty_queue){
    if(empty_queue == false){
        UA_Boolean found_in_queue = false;
        for(int i = 0; i < queue_handler_list->queue_size; i++){
            if(UA_String_equal(&queue_handler_list->queue[i].orderId, &queue_handler_list->element_state.orderId) &&
                UA_String_equal(&queue_handler_list->queue[i].service_UUID, &queue_handler_list->element_state.service_uuid)){
                queue_handler_list->queue[i].queue_Element_State = queue_handler_list->element_state.newState;
                found_in_queue = true;
                }
        }
        if(found_in_queue == false)
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Unable to change the state of element with service_uuid %s and order_id %s. It is not in the queue",
                (char*) queue_handler_list->element_state.orderId.data, (char*) queue_handler_list->element_state.service_uuid.data);
        UA_Boolean found = false;
        UA_Queue_List_Element *current, *next_element;
        SLIST_FOREACH_SAFE(current, &queue_handler_list->queue_element_list, next, next_element){
            if(UA_String_equal(&current->queue_element.orderId, &queue_handler_list->element_state.orderId) &&
                UA_String_equal(&current->queue_element.service_UUID, &queue_handler_list->element_state.service_uuid)){
                current->queue_element.queue_Element_State = queue_handler_list->element_state.newState;
                found = true;
            }
        }
        if(found == false)
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Unable to change the state of element with service_uuid %s and order_id %s. It is not in the linked list",
                (char*) queue_handler_list->element_state.orderId.data, (char*) queue_handler_list->element_state.service_uuid.data);
        if(found == false || found_in_queue == false){
            UA_Set_Queue_Element_State_Data_Type_clear(&queue_handler_list->element_state);
            UA_Set_Queue_Element_State_Data_Type_init(&queue_handler_list->element_state);
            queue_handler_list->set_state = false;
            return UA_STATUSCODE_BADDATALOST;
        }
    }
    UA_Set_Queue_Element_State_Data_Type_clear(&queue_handler_list->element_state);
    UA_Set_Queue_Element_State_Data_Type_init(&queue_handler_list->element_state);
    queue_handler_list->set_state = false;
    return UA_STATUSCODE_GOOD;
}*/

UA_StatusCode add_queue_to_method_context(UA_Server *server, char *method_node,
                  UA_Queue_Data *method_context, UA_MethodCallback callback){
    UA_NodeId method_nodeId;
    UA_NodeId_init(&method_nodeId);
    UA_StatusCode retval = find_method(server, method_node, &method_nodeId);
    if(retval!= UA_STATUSCODE_GOOD){
        UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to find the Method %s in the Address Space ", method_node);
        UA_NodeId_clear(&method_nodeId);
        return retval;
    }
    retval = UA_Server_setMethodNodeCallback(server, method_nodeId, (UA_MethodCallback) callback);
    if(retval != UA_STATUSCODE_GOOD){
        UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to add the method context for Method %s ", method_node);
        return retval;
    }
    retval = UA_Server_setNodeContext(server, method_nodeId, (void*) method_context);
    if(retval != UA_STATUSCODE_GOOD){
        UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to add the method callback for Method %s ", method_node);
        return retval;
    }
    UA_NodeId_clear(&method_nodeId);
    return UA_STATUSCODE_GOOD;
}