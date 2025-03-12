/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 * Copyright 2024 (c) Fraunhofer IWU (Author: Thorben Schnellhardt)
 */

#include "queue_handler.h"
#include "server_internal.h"
#include "node_finder.h"
#include <stdio.h>

UA_StatusCode start_queue_handler(UA_Server *server, UA_NodeId module_object_nodeId, UA_Boolean *running){
    UA_queue_container *add_items = (UA_queue_container*) UA_calloc(1, sizeof(UA_queue_container));
    add_queue_handler(server, "add_queue_element", add_items);
    UA_queue_container *remove_item = (UA_queue_container*) UA_calloc(1, sizeof(UA_queue_container));
    add_queue_handler(server, "remove_queue_element", remove_item);

    //add queue state handler
    UA_NodeId methodNodeId;
    UA_NodeId_init(&methodNodeId);
    UA_StatusCode retval = find_method(server, "set_queue_element_state", &methodNodeId);
    if(retval!= UA_STATUSCODE_GOOD){
        printf("failed to find the set_queue_element_state method \n");
        UA_NodeId_clear(&methodNodeId);
        return retval;
    }
    UA_Change_entry_list *entry_list = (UA_Change_entry_list*) UA_calloc(1, sizeof(UA_Change_entry_list));
    entry_list->nbr_entries = 0;
    entry_list->entries = (UA_change_queue_element_state_container*) UA_calloc(0, sizeof(UA_change_queue_element_state_container));
    UA_Server_setNodeContext(server, methodNodeId, (void*) entry_list);
    UA_Server_setMethodNodeCallback(server, methodNodeId, (UA_MethodCallback) set_queue_entry_state_methodCallback);

    /* init handler variables */
    UA_Queue_handle *handler = (UA_Queue_handle*) UA_calloc(1, sizeof(UA_Queue_handle));
    UA_NodeId_copy(&module_object_nodeId, &handler->Module_Object_NodeId);
    handler->write_struct_values = UA_FALSE;
    handler->server = server;

    /* init observer variables */
    UA_Queue_observer *observer = (UA_Queue_observer*) UA_calloc(1, sizeof(UA_Queue_observer));
    observer->handler = handler;
    observer->remove_items = remove_item;
    observer->add_items = add_items;
    observer->entry_list = entry_list;
    observer->running = running;

    pthread_t queue_observer_thread;
    pthread_create(&queue_observer_thread, NULL, observer_queue, observer);
    pthread_detach(queue_observer_thread);

    UA_NodeId_clear(&methodNodeId);
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode add_queue_function_handler(UA_Server *server, UA_NodeId methodNodeId, UA_MethodCallback methodcallback,
                           UA_queue_container *item_list){
    UA_StatusCode retval = UA_Server_setMethodNodeCallback(server, methodNodeId, (UA_MethodCallback) methodcallback);
    if(retval != UA_STATUSCODE_GOOD){
        printf("failed to add the method callback\n");
        return retval;
    }
    retval = UA_Server_setNodeContext(server, methodNodeId, (void*) item_list);
    if(retval != UA_STATUSCODE_GOOD){
        printf("failed to add the method callback\n");
        return retval;
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
queue_handler_method_callback(UA_Server *server,
                             const UA_NodeId *sessionId, void *sessionHandle,
                             const UA_NodeId *methodId, void *methodContext,
                             const UA_NodeId *objectId, void *objectContext,
                             size_t inputSize, const UA_Variant *input,
                             size_t outputSize, UA_Variant *output){

    UA_Queue_Data_Type inp = *(UA_Queue_Data_Type*) input->data;
    UA_Server_getNodeContext(server, *methodId, &methodContext);
    UA_queue_container *queue_handler_list = (UA_queue_container *) methodContext;
    UA_StatusCode retval = UA_Array_resize((void**) &(*queue_handler_list).item_list, &queue_handler_list->nbr_items, queue_handler_list->nbr_items+1, &queue_handler_list->Queue_Variable_Data_Type);
    if(retval != UA_STATUSCODE_GOOD){
        printf("failes to resize the add_to_queu_item_list \n");
        return retval;
    }
    memset(&(*queue_handler_list).item_list[queue_handler_list->nbr_items-1], 0, sizeof(UA_Queue_Data_Type));
    retval = UA_Queue_Data_Type_copy(&inp, &queue_handler_list->item_list[queue_handler_list->nbr_items-1]);
    if(retval != UA_STATUSCODE_GOOD){
        printf("failed to copy the queue data type\n");
        return retval;
    }
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode add_queue_handler(UA_Server *server, char *method_node,
                  UA_queue_container *method_context){
    UA_NodeId method_nodeId;
    UA_NodeId_init(&method_nodeId);
    UA_StatusCode retval = find_method(server, method_node, &method_nodeId);
    if(retval!= UA_STATUSCODE_GOOD){
        printf("failed to find method %s \n", method_node);
        UA_NodeId_clear(&method_nodeId);
        return retval;
    }
    method_context->Queue_Variable_Data_Type = UA_TYPES_COMMON[UA_TYPES_COMMON_QUEUE_DATA_TYPE];
    method_context->item_list = (UA_Queue_Data_Type*) UA_Array_new(0, &method_context->Queue_Variable_Data_Type);
    method_context->nbr_items = 0;
    retval = add_queue_function_handler(server, method_nodeId,
                               (UA_MethodCallback)queue_handler_method_callback,
                               method_context);
    if(retval != UA_STATUSCODE_GOOD){
        printf("failed to add the queue function handler\n");
        UA_NodeId_clear(&method_nodeId);
        return retval;
    }
    UA_NodeId_clear(&method_nodeId);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode set_queue_entry_state_methodCallback(UA_Server *server,
                                    const UA_NodeId *sessionId, void *sessionHandle,
                                    const UA_NodeId *methodId, void *methodContext,
                                    const UA_NodeId *objectId, void *objectContext,
                                    size_t inputSize, const UA_Variant *input,
                                    size_t outputSize, UA_Variant *output){
    UA_Server_getNodeContext(server, *methodId, &methodContext);
    UA_Change_entry_list *curr_list = (UA_Change_entry_list*) methodContext;
    curr_list->nbr_entries+=1;
    curr_list->entries = (UA_change_queue_element_state_container*) UA_realloc(curr_list->entries, curr_list->nbr_entries*sizeof(UA_change_queue_element_state_container));
    memset(&curr_list->entries[curr_list->nbr_entries-1], 0, sizeof(UA_change_queue_element_state_container));
    UA_Queue_State_Variable_Type_copy((UA_Queue_State_Variable_Type*) input[2].data, &curr_list->entries->next_state);
    UA_String_init(&curr_list->entries[curr_list->nbr_entries-1].client_identifier);
    UA_String_init(&curr_list->entries[curr_list->nbr_entries-1].Service_UUID);
    UA_String_copy((UA_String*) input[0].data, &curr_list->entries[curr_list->nbr_entries-1].client_identifier);
    UA_String_copy((UA_String*) input[1].data, &curr_list->entries[curr_list->nbr_entries-1].Service_UUID);
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode copy_and_clear_add_queue_container(UA_queue_container **items,
                                   UA_queue_container *handler_list){
    memcpy(&handler_list->nbr_items, &(*items)->nbr_items, sizeof(size_t));
    memcpy(&handler_list->Queue_Variable_Data_Type, &(*items)->Queue_Variable_Data_Type, sizeof(UA_DataType));
    UA_StatusCode retval = UA_Array_copy((*items)->item_list, (*items)->nbr_items, (void**) &handler_list->item_list, &(*items)->Queue_Variable_Data_Type);
    if(retval != UA_STATUSCODE_GOOD){
        printf("failed to copy the add items array with error %s \n", UA_StatusCode_name(retval));
        return retval;
    }
    else{
        retval = UA_Array_resize((void**) &(*items)->item_list, &(*items)->nbr_items, 0, &(*items)->Queue_Variable_Data_Type);
        if(retval != UA_STATUSCODE_GOOD){
            printf("failed to resize the add items array with error %s \n", UA_StatusCode_name(retval));
            return retval;
        }
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode copy_and_clear_entry_list(UA_Change_entry_list **entry_list, UA_Change_entry_list *handler_entry_list){
    memcpy(&handler_entry_list->nbr_entries, &(*entry_list)->nbr_entries, sizeof(size_t));
    handler_entry_list->entries = (UA_change_queue_element_state_container *) UA_calloc(handler_entry_list->nbr_entries, sizeof(UA_change_queue_element_state_container));
    for(size_t i=0; i< (*entry_list)->nbr_entries; i++){
        UA_String_init(&handler_entry_list->entries[i].client_identifier);
        UA_String_copy(&(*entry_list)->entries[i].client_identifier, &handler_entry_list->entries[i].client_identifier);
        UA_String_init(&handler_entry_list->entries[i].Service_UUID);
        UA_String_copy(&(*entry_list)->entries[i].Service_UUID, &handler_entry_list->entries[i].Service_UUID);
        UA_Queue_State_Variable_Type_init(&handler_entry_list->entries[i].next_state);
        UA_Queue_State_Variable_Type_copy(&(*entry_list)->entries[i].next_state, &handler_entry_list->entries[i].next_state);
    }
    for(size_t i=0; i< (*entry_list)->nbr_entries; i++){
        UA_String_clear(&(*entry_list)->entries[i].client_identifier);
        UA_String_clear(&(*entry_list)->entries[i].Service_UUID);
        UA_Queue_State_Variable_Type_clear(&(*entry_list)->entries[i].next_state);
    }
    (*entry_list)->entries = (UA_change_queue_element_state_container*) UA_realloc((*entry_list)->entries, 0);
    (*entry_list)->nbr_entries = 0;
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode copy_and_clear_queue_handler_lists(UA_queue_container *add_items,
                                   UA_queue_container *remove_item, UA_Queue_handle *handler, UA_Change_entry_list *entry_list){
    //add item list
    UA_StatusCode retval = copy_and_clear_add_queue_container(&add_items, &handler->add_items);
    if(retval !=UA_STATUSCODE_GOOD){
        printf("failed to copy the add items list %s \n", UA_StatusCode_name(retval));
        return retval;
    }
    //remove item list
    retval = copy_and_clear_add_queue_container(&remove_item, &handler->remove_items);
    if(retval !=UA_STATUSCODE_GOOD){
        printf("failed to copy the remove items list %s \n", UA_StatusCode_name(retval));
        return retval;
    }
    //set state list
    retval = copy_and_clear_entry_list(&entry_list, &handler->entry_list);
    if(retval != UA_STATUSCODE_GOOD){
        printf("failed to copy the entry list %s \n", UA_StatusCode_name(retval));
        return retval;
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode change_queue_entry_state(UA_Server *server, size_t nbr_current_entries, UA_Queue_Data_Type *current_items, UA_Change_entry_list *change_state, UA_NodeId queue_variable_nodeId){
    for(size_t i=0; i< change_state->nbr_entries; i++){
        for(size_t j=0; j< nbr_current_entries; j++){
            if(UA_String_equal(&current_items[j].service_UUID, &change_state->entries[i].Service_UUID) &&
               UA_String_equal(&current_items[j].client_Identifier, &change_state->entries[i].client_identifier) &&
               current_items[j].queue_Element_State != change_state->entries[i].next_state){
                current_items[j].queue_Element_State = change_state->entries[i].next_state;
                break;
            }
        }
    }
    UA_Variant new_val;
    UA_Variant_init(&new_val);
    UA_Variant_setArrayCopy(&new_val, current_items, nbr_current_entries, &UA_TYPES_COMMON[UA_TYPES_COMMON_QUEUE_DATA_TYPE]);
    UA_StatusCode retval = UA_Server_writeValue(server, queue_variable_nodeId, new_val);
    if(retval != UA_STATUSCODE_GOOD){
        printf("failed to write the new state entry list to the server %s \n", UA_StatusCode_name(retval));
        return retval;
    }
    UA_Variant_clear(&new_val);
    change_state->entries = (UA_change_queue_element_state_container *) UA_realloc(change_state->entries, 0);
    change_state->nbr_entries = 0;
    return UA_STATUSCODE_GOOD;
}

static void set_first_ertry_state(UA_Queue_Data_Type *queue_array, size_t nbr_queue_entries){
    for(size_t i=0; i< nbr_queue_entries; i++){
        if(queue_array[i].entry_Number == 1){
            queue_array[i].queue_Element_State = UA_QUEUE_STATE_VARIABLE_TYPE_READY_FOR_EXECUTION;
        }
    }
}

static UA_StatusCode add_queue_elements(UA_Server *server, size_t nbr_current_entries, UA_Queue_Data_Type *current_items, UA_queue_container *add_item_list, UA_NodeId queue_variable_nodeId){
    for(size_t i=0; i< add_item_list->nbr_items; i++){
        add_item_list->item_list[i].entry_Number = (UA_Int16)(add_item_list->item_list[i].entry_Number + (UA_Int16) nbr_current_entries + (UA_Int16) i + 1);
    }
    UA_Queue_Data_Type *new_val = (UA_Queue_Data_Type*) UA_Array_new(nbr_current_entries+add_item_list->nbr_items, &add_item_list->Queue_Variable_Data_Type);
    for(size_t i=0; i< nbr_current_entries+add_item_list->nbr_items; i++){
        if(i < nbr_current_entries){
            UA_Queue_Data_Type_copy(&current_items[i], &new_val[i]);
        }
        else{
            UA_Queue_Data_Type_copy(&add_item_list->item_list[i-nbr_current_entries], &new_val[i]);
        }
    }
    UA_Variant val;
    UA_Variant_init(&val);
    set_first_ertry_state(new_val, nbr_current_entries+add_item_list->nbr_items);
    UA_Variant_setArrayCopy(&val, new_val, nbr_current_entries+add_item_list->nbr_items, &add_item_list->Queue_Variable_Data_Type);
    UA_Server_writeValue(server, queue_variable_nodeId, val);
    UA_Array_delete(new_val, nbr_current_entries+add_item_list->nbr_items, &add_item_list->Queue_Variable_Data_Type);
    UA_Variant_clear(&val);
    UA_StatusCode retval = UA_Array_resize((void **)&add_item_list->item_list, &add_item_list->nbr_items, 0, &add_item_list->Queue_Variable_Data_Type);
    if(retval != UA_STATUSCODE_GOOD){
        printf("failed to resize the add items array with error %s \n", UA_StatusCode_name(retval));
        return retval;
    }
    add_item_list->nbr_items = 0;
    return retval;
}

static void check_items_to_be_removed(size_t *elements_to_be_removed,
                          UA_queue_container *remove_item_list, size_t nbr_current_entries, UA_Queue_Data_Type *current_items){
    for (size_t i=0; i< remove_item_list->nbr_items; i++){
        for(size_t j=0; j< nbr_current_entries; j++){
            if(UA_String_equal(&current_items[j].service_UUID, &remove_item_list->item_list[i].service_UUID) &&
               UA_String_equal(&current_items[j].client_Identifier, &remove_item_list->item_list[i].client_Identifier)){
                (*elements_to_be_removed)++;
                break;
            }
        }
    }
}

static UA_StatusCode get_element_position_by_entry_number(size_t nbr_current_entries, UA_Queue_Data_Type *current_items, UA_Int16 entry_number, size_t *list_position){
    for(size_t i=0; i<nbr_current_entries; i++){
        if(current_items[i].entry_Number == entry_number){
            *list_position = i;
            return UA_STATUSCODE_GOOD;
        }
    }
    return UA_STATUSCODE_BAD;
}

static UA_StatusCode remove_queue_elements(UA_Server *server, size_t nbr_current_entries, UA_Queue_Data_Type *current_items,
                      UA_queue_container *remove_item_list, UA_NodeId queue_variable_nodeId){
    UA_Variant value;
    UA_Variant_init(&value);
    size_t elements_to_be_removed =0;
    if(nbr_current_entries < remove_item_list->nbr_items){
        //todo remove existing elements
        printf("more items should be removed than exists in the queue list \n");
    }
    if(nbr_current_entries - remove_item_list->nbr_items == 0){
        check_items_to_be_removed(&elements_to_be_removed, remove_item_list, nbr_current_entries, current_items);
        if(elements_to_be_removed == nbr_current_entries){
            UA_Queue_Data_Type new_val;
            UA_Queue_Data_Type_init(&new_val);
            UA_Variant_setScalarCopy(&value, &new_val, &remove_item_list->Queue_Variable_Data_Type);
            UA_Server_writeValue(server, queue_variable_nodeId, value);
            UA_Queue_Data_Type_clear(&new_val);
        }
        //non existing items are reffered to in the remove item list
        else{
            //todo remove existing elements
            printf("empty queue remaining\n");
            printf("non existing elements should be removed\n");
        }
    }
    else{
        check_items_to_be_removed(&elements_to_be_removed, remove_item_list, nbr_current_entries, current_items);
        if(elements_to_be_removed == remove_item_list->nbr_items){
            size_t new_size = nbr_current_entries - remove_item_list->nbr_items;
            UA_Queue_Data_Type *new_val = (UA_Queue_Data_Type*) UA_Array_new(new_size, &remove_item_list->Queue_Variable_Data_Type);
            UA_Int16 old_entry_number = 1, new_entry_number = 1;
            for(size_t i=0; i < nbr_current_entries; i++){
                size_t curr_element;
                UA_Boolean found = UA_FALSE;
                UA_StatusCode retval = get_element_position_by_entry_number(nbr_current_entries, current_items, old_entry_number, &curr_element);
                if(retval!= UA_STATUSCODE_GOOD){
                    UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "ERROR! Non existing queue element should be removed");
                    return retval;
                }
                for(size_t j=0; j< remove_item_list->nbr_items; j++){
                    if(UA_String_equal(&current_items[curr_element].service_UUID, &remove_item_list->item_list[j].service_UUID) &&
                       UA_String_equal(&current_items[curr_element].client_Identifier, &remove_item_list->item_list[j].client_Identifier)){
                        found = UA_TRUE;
                        break;
                    }
                }
                if(found == UA_FALSE){
                    UA_Queue_Data_Type_copy(&current_items[curr_element], &new_val[new_entry_number-1]);
                    new_val[new_entry_number-1].entry_Number = new_entry_number;
                    new_entry_number++;
                }
                old_entry_number++;
            }
            set_first_ertry_state(new_val, nbr_current_entries - remove_item_list->nbr_items);
            UA_Variant_setArrayCopy(&value, new_val, new_size, &remove_item_list->Queue_Variable_Data_Type);
            UA_Server_writeValue(server, queue_variable_nodeId, value);
            UA_Variant_clear(&value);
            UA_Array_delete(new_val, new_size, &remove_item_list->Queue_Variable_Data_Type);
        }
        else{
            printf("non existing elements should be removed\n");
        }
    }
    UA_Variant_clear(&value);
    return UA_STATUSCODE_GOOD;
}

static void check_queue_variable(UA_Server *server, UA_NodeId *queue_variable_nodeId, UA_NodeId Module_Object_NodeId){
    get_single_node(server, UA_QUALIFIEDNAME(0,"queue_variable"), queue_variable_nodeId);
    UA_NodeId temp = UA_NODEID_NULL;
    if(UA_NodeId_equal(&temp, queue_variable_nodeId)){
        printf("error no queue variable found\n");
        /*add_queue_variable(server, "Queue_Data_Type", Module_Object_NodeId);
        get_single_node(server, UA_QUALIFIEDNAME(0,"queue_variable"), queue_variable_nodeId);*/
    }
    /*else{
        UA_String out = UA_STRING_NULL;
        UA_print(queue_variable_nodeId, &UA_TYPES[UA_TYPES_NODEID], &out);
        printf("%.*s\n", (int)out.length, out.data);
        UA_String_clear(&out);
    }*/
}

static UA_Queue_Data_Type *get_current_queue(UA_Server *server, size_t *value_list_size, UA_NodeId queue_variable_nodeId){
    UA_Variant queue_variable_value;
    UA_Variant_init(&queue_variable_value);
    UA_Server_readValue(server, queue_variable_nodeId, &queue_variable_value);
    *value_list_size = queue_variable_value.arrayLength;
    UA_Queue_Data_Type *value = (UA_Queue_Data_Type*) queue_variable_value.data;
    return value;
}

void *handle_queue(void *args){
    UA_Queue_handle *handler = (UA_Queue_handle*) args;
    UA_NodeId queue_variable_nodeId;
    UA_NodeId_init(&queue_variable_nodeId);
    check_queue_variable(handler->server, &queue_variable_nodeId, handler->Module_Object_NodeId);
    size_t value_list_size;
    //add new elements to queue
    if(handler->add_items.nbr_items > 0){
        UA_Queue_Data_Type *value = get_current_queue(handler->server, &value_list_size, queue_variable_nodeId);
        add_queue_elements(handler->server, value_list_size, value, &handler->add_items, queue_variable_nodeId);
        UA_Array_delete(value, value_list_size, &handler->add_items.Queue_Variable_Data_Type);
    }
    //change queue entries
    if(handler->entry_list.nbr_entries > 0){
        UA_Queue_Data_Type *value = get_current_queue(handler->server, &value_list_size, queue_variable_nodeId);
        if((value_list_size == 1 && value[0].entry_Number != 0) || (value_list_size > 1)){
            change_queue_entry_state(handler->server, value_list_size, value, &handler->entry_list, queue_variable_nodeId);
        }
        UA_Array_delete(value, value_list_size, &handler->add_items.Queue_Variable_Data_Type);
    }
    //remove elements from queue
    if(handler->remove_items.nbr_items > 0){
        UA_Queue_Data_Type *value = get_current_queue(handler->server, &value_list_size, queue_variable_nodeId);
        if((value_list_size == 1 && value->entry_Number != 0) || (value_list_size > 1)  ) {
            remove_queue_elements(handler->server, value_list_size, value, &handler->remove_items, queue_variable_nodeId);
        }
        UA_Array_delete(value, value_list_size, &handler->add_items.Queue_Variable_Data_Type);
    }
    handler->write_struct_values = UA_FALSE;
    if(handler->add_items.nbr_items > 0)
        UA_Array_delete(handler->add_items.item_list, handler->add_items.nbr_items, &handler->add_items.Queue_Variable_Data_Type);
    if(handler->remove_items.nbr_items > 0)
        UA_Array_delete(handler->remove_items.item_list, handler->remove_items.nbr_items, &handler->remove_items.Queue_Variable_Data_Type);
    if(handler->entry_list.nbr_entries > 0){
        for(size_t i= 0; i<handler->entry_list.nbr_entries; i++){
            UA_String_clear(&handler->entry_list.entries[i].client_identifier);
            UA_String_clear(&handler->entry_list.entries[i].Service_UUID);
        }
    }
    free(handler->entry_list.entries);
    return handler;
}

void *observer_queue(void *args){
    UA_Queue_observer *observer = (UA_Queue_observer*) args;
    pthread_t thread;
    while(*(observer->running) == UA_TRUE){
        if(observer->handler->write_struct_values == false){
            if(observer->entry_list->nbr_entries > 0 || observer->add_items->nbr_items > 0 || observer->remove_items->nbr_items > 0){
                copy_and_clear_queue_handler_lists(observer->add_items, observer->remove_items, observer->handler, observer->entry_list);
                observer->handler->write_struct_values = UA_TRUE;
                pthread_create(&thread, NULL, handle_queue, observer->handler);
            }
        }
        struct timespec ts;
        ts.tv_sec = 1;
        ts.tv_nsec = 10;
        nanosleep(&ts, NULL);
    }
    printf("shut down the observer thread\n");
    free(observer->handler);
    free(observer->remove_items);
    free(observer->add_items);
    if(observer->entry_list->nbr_entries > 0){
        for(size_t i=0; i< observer->entry_list->nbr_entries; i++){
            UA_String_clear(&observer->entry_list->entries[i].client_identifier);
            UA_String_clear(&observer->entry_list->entries[i].Service_UUID);
            free(&observer->entry_list->entries[i]);
        }
    }
    else{
        free(observer->entry_list->entries);
    }
    free(observer->entry_list);
    free(observer);
    return NULL;
}
