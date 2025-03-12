/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 * Copyright 2024 (c) Fraunhofer IWU (Author: Thorben Schnellhardt)
 */

#include "swap_it.h"
#include "queue_handler.h"
#include "register_callbacks.h"
#include "server_internal.h"
#include "node_finder.h"
#include <stdio.h>
#include "signal.h"
#include "unistd.h"

UA_StatusCode UA_server_swap_it(UA_Server *server,
                                UA_ByteString json,
                                UA_MethodCallback callback,
                                UA_Boolean default_behavior,
                                UA_Boolean *running,
                                UA_Boolean register_agent_in_registry,
                                UA_service_server_interpreter *swap_server){

    UA_StatusCode retval = get_server_dict(swap_server, json);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to add the SWAP Configuration with Statuscode %s.", UA_StatusCode_name(retval));
        clear_swap_server(swap_server, UA_FALSE, server);
        return retval;
    }

    UA_ServerConfig *conf = UA_Server_getConfig(server);
    UA_ServerConfig_setMinimal(conf, swap_server->port, NULL);
    //todo check if i can only change the port number -> maby also define port and application name as optional feature
    UA_LocalizedText_clear(&conf->applicationDescription.applicationName);
    conf->applicationDescription.applicationName = UA_LOCALIZEDTEXT_ALLOC("en", swap_server->server_name);
    if(swap_server->sessions != 0)
        conf->maxSessions = swap_server->sessions;
    if(swap_server->channels !=0 )
        conf->maxSecureChannels = swap_server->channels;
    UA_Server_run_startup(server);
    UA_Server_run_iterate(server, running);
    sleep(5);
    // add the module type
    UA_NodeId module_object_nodeId;
    UA_NodeId_init(&module_object_nodeId);
    retval = instantiate_module_type(server, swap_server, &module_object_nodeId);
    if(retval != UA_STATUSCODE_GOOD){
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to add the ModuleType with Statuscode %s. Please check previous output", UA_StatusCode_name(retval));
        clear_swap_server(swap_server, UA_FALSE, server);
        UA_NodeId_clear(&module_object_nodeId);
        return retval;
    }
    //add the service method callback
    retval = add_method_callback(server, swap_server, callback);
    if(retval != UA_STATUSCODE_GOOD){
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to link the Methodcallback with the Method Node with Statuscode %s. Please check previous output", UA_StatusCode_name(retval));
        clear_swap_server(swap_server, UA_FALSE, server);
        UA_NodeId_clear(&module_object_nodeId);
        return retval;
    }
    UA_Server_run_iterate(server, true);
    //add the register/unregister callbacks
    UA_NodeId register_function_id;
    UA_NodeId unregister_function_id;
    UA_NodeId_init(&register_function_id);
    UA_NodeId_init(&unregister_function_id);
    retval = add_register_methods(server, &register_function_id, &unregister_function_id);
    if(retval != UA_STATUSCODE_GOOD){
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to link the register/unregister Methods with Statuscode %s. Please check previous output", UA_StatusCode_name(retval));
        clear_swap_server(swap_server, UA_FALSE, server);
        UA_NodeId_clear(&register_function_id);
        UA_NodeId_clear(&unregister_function_id);
        UA_NodeId_clear(&module_object_nodeId);
        return retval;
    }
    UA_Server_run_iterate(server, true);
    /*set the service name as Node Context*/
    UA_register_method_context *context = (UA_register_method_context*) UA_calloc(1, sizeof(UA_register_method_context));
    context->service_name = UA_String_fromChars(swap_server->service_name);
    context->module_type = UA_String_fromChars(swap_server->module_name);
    UA_Server_setNodeContext(server, register_function_id, (void*) context);
    UA_Server_setNodeContext(server, unregister_function_id, (void*) context);


    //set initial variable values (state variable, registered variable)
    retval = write_state_variable(server);
    if(retval != UA_STATUSCODE_GOOD){
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to write the state variable with Statuscode %s.", UA_StatusCode_name(retval));
        clear_swap_server(swap_server, UA_FALSE, server);
        UA_NodeId_clear(&register_function_id);
        UA_NodeId_clear(&unregister_function_id);
        UA_NodeId_clear(&module_object_nodeId);
        return retval;
    }
    UA_Server_run_iterate(server, true);
    //add subscription objects from config
    retval = add_subscription_objects(server, swap_server, module_object_nodeId);
    if(retval != UA_STATUSCODE_GOOD){
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to link the register/unregister Methods with Statuscode %s. Please check previous output", UA_StatusCode_name(retval));
        clear_swap_server(swap_server, UA_FALSE, server);
        UA_NodeId_clear(&register_function_id);
        UA_NodeId_clear(&unregister_function_id);
        UA_NodeId_clear(&module_object_nodeId);
        return retval;
    }
    UA_Server_run_iterate(server, true);
    //add capability objects from json config
    retval = add_capability_nodes(server, swap_server, module_object_nodeId);
    if(retval != UA_STATUSCODE_GOOD){
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to add the capabilities variables with Statuscode %s. Please check previous output", UA_StatusCode_name(retval));
        clear_swap_server(swap_server, UA_FALSE, server);
        UA_NodeId_clear(&register_function_id);
        UA_NodeId_clear(&unregister_function_id);
        UA_NodeId_clear(&module_object_nodeId);
        return retval;
    }
    UA_Server_run_iterate(server, true);
    //activate queue handler
    start_queue_handler(server, module_object_nodeId, running);
    UA_Server_run_iterate(server, true);
    //register the agent
    if(register_agent_in_registry == UA_TRUE){
        UA_Register_function_Input_data *inp = (UA_Register_function_Input_data*) UA_calloc(1, sizeof(UA_Register_function_Input_data));
        inp->Device_registry_url = UA_String_fromChars(swap_server->device_registry_url);
        inp->service_name = UA_String_fromChars(swap_server->service_name);
        inp->moduleType = UA_String_fromChars(swap_server->module_name);
        inp->address = UA_String_fromChars(swap_server->resource_ip);
        char port[128];
        sprintf(port, "%d", swap_server->port);
        inp->port = UA_String_fromChars(port);
        pthread_create(&inp->thread, NULL, register_agent, inp);
        retval = write_registered_variable(server, UA_TRUE);
        if(retval != UA_STATUSCODE_GOOD) {
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,
                         "Failed to write the registered variable to false with Statuscode %s. Please check previous output",
                         UA_StatusCode_name(retval));
            clear_swap_server(swap_server, UA_FALSE, server);
            UA_NodeId_clear(&register_function_id);
            UA_NodeId_clear(&unregister_function_id);
            UA_NodeId_clear(&module_object_nodeId);
            return retval;
        }
    }
    else{
        retval = write_registered_variable(server, UA_FALSE);
        if(retval != UA_STATUSCODE_GOOD){
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Failed to write the registered variable to false with Statuscode %s. Please check previous output", UA_StatusCode_name(retval));
            clear_swap_server(swap_server, UA_TRUE, server);
            UA_NodeId_clear(&register_function_id);
            UA_NodeId_clear(&unregister_function_id);
            UA_NodeId_clear(&module_object_nodeId);
            return retval;
        }
    }
    UA_Server_run_iterate(server, true);
    UA_NodeId_clear(&register_function_id);
    UA_NodeId_clear(&unregister_function_id);
    UA_NodeId_clear(&module_object_nodeId);
    return retval;
}


/*todo remove the instatiated subtype of the module type, add boolean clear method context and remove module type*/
void clear_swap_server(UA_service_server_interpreter *server_info, UA_Boolean unregister, UA_Server *server){
    if(unregister == UA_TRUE){
        UA_Register_function_Input_data *unregister = (UA_Register_function_Input_data*) UA_calloc(1, sizeof(UA_Register_function_Input_data));
        unregister->service_name = UA_String_fromChars(server_info->service_name);
        char port[128];
        sprintf(port, "%d", server_info->port);
        unregister->port = UA_String_fromChars(port);
        unregister->Device_registry_url = UA_String_fromChars(server_info->device_registry_url);
        unregister->address = UA_String_fromChars(server_info->resource_ip);
        pthread_create(&unregister->thread, NULL, unregister_agent, unregister);
        write_registered_variable(server, UA_FALSE);
    }
    /*free memory from method context*/
    UA_StatusCode retval = write_registered_variable(server, UA_FALSE);
    if(retval == UA_STATUSCODE_GOOD){
        UA_NodeId registerId = UA_NODEID_NULL;
        get_single_node(server, UA_QUALIFIEDNAME(2, "register"), &registerId);
        void *val;
        UA_Server_getNodeContext(server, registerId, &val);
        UA_register_method_context *temp = (UA_register_method_context*) val;
        UA_String_clear(&temp->module_type);
        UA_String_clear(&temp->service_name);
        free(temp);
    }
    UA_UInt16_clear(&server_info->port);
    UA_UInt16_clear(&server_info->channels);
    UA_UInt16_clear(&server_info->sessions);
    free(server_info->server_name);
    free(server_info->resource_ip);
    free(server_info->module_type);
    free(server_info->module_name);
    free(server_info->service_name);
    free(server_info->device_registry_url);
    for(size_t i=0; i< server_info->nbr_registry_subscriptions; i++){
        free(server_info->registry_subscriptions[i]);
    }
    free(server_info->registry_subscriptions);
    free(server_info->capabilities);
    server_info->nbr_registry_subscriptions = 0;
    server_info->nbr_capabilities = 0;
}
