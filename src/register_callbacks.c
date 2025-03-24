/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 * Copyright 2024 (c) Fraunhofer IWU (Author: Thorben Schnellhardt)
 */

#include "register_callbacks.h"
#include "node_finder.h"
#include <stdio.h>
#include "pthread.h"


static void extract_dr_and_server_url(UA_String *dr_url, UA_String *address, UA_String *port, const UA_Variant *input){
    /*get device registry url*/
    UA_String url = *(UA_String*) input[0].data;
    char *dr_ip = "opc.tcp://";
    char *inp_data = (char*) url.data;
    size_t size = 1+strlen(dr_ip)+url.length;
    char *dr_url_temp = (char *) UA_calloc(size, sizeof(char));
    strcpy(dr_url_temp, dr_ip);
    strcat(dr_url_temp, inp_data);
    dr_url_temp[size-1] = '\0';
    *dr_url = UA_String_fromChars(dr_url_temp);
    free(dr_url_temp);
    /*get server address and port*/
    char *token;
    UA_String *tempstr = (UA_String*) input[1].data;
    char val[256];
    const char *temp = (const char*) tempstr->data;
    strcpy(val, temp);
    val[tempstr->length+1] = '\0';
    char server_address_info[2][256];
    token = strtok(val , ":");
    for(size_t i=0; i< 2;i++){
        strcpy(server_address_info[i],token);
        token = strtok( NULL, ":");
    }
    *address = UA_String_fromChars(server_address_info[0]);
    *port = UA_String_fromChars(server_address_info[1]);
}

//function that creates a client which connects to the device registry and registeres the agent
void * register_agent(void *input){
    UA_Register_function_Input_data *inp = (UA_Register_function_Input_data*) input;
    UA_Client *client = UA_Client_new();
    UA_ClientConfig *conf = UA_Client_getConfig(client);
    UA_ClientConfig_setDefault(conf);
    char *url = (char*) UA_calloc(inp->Device_registry_url.length+1, sizeof(char));
    memcpy(url, (char*) inp->Device_registry_url.data, inp->Device_registry_url.length);
    url[inp->Device_registry_url.length] = '\0';
    UA_StatusCode retval = UA_Client_connect(client, url);
    printf("register agent: Client connection to %s is %s \n", url, UA_StatusCode_name(retval));
    if(retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,
                       "The client failed to reach the device registry with url: %s and Statuscode %s", url, UA_StatusCode_name(retval));
        UA_String_clear(&inp->Device_registry_url);
        UA_String_clear(&inp->address);
        UA_String_clear(&inp->port);
        UA_String_clear(&inp->moduleType);
        UA_String_clear(&inp->service_name);
        free(url);
        free(inp);
        return NULL;
    }
    else{
        UA_NodeId service_methodid, service_parent_object_id;
        UA_NodeId_init(&service_methodid);
        UA_NodeId_init(&service_parent_object_id);
        client_get_single_node(client, UA_QUALIFIEDNAME(0, "Add_Agent_Server"), &service_methodid);
        client_get_single_node(client, UA_QUALIFIEDNAME(0, "PFDLServiceAgents"), &service_parent_object_id);
        UA_Variant *var = (UA_Variant *) UA_Array_new(4, &UA_TYPES[UA_TYPES_VARIANT]);
        UA_Variant_setScalarCopy(&var[0], &inp->service_name, &UA_TYPES[UA_TYPES_STRING]);
        UA_Variant_setScalarCopy(&var[1], &inp->address, &UA_TYPES[UA_TYPES_STRING]);
        UA_Variant_setScalarCopy(&var[2], &inp->port, &UA_TYPES[UA_TYPES_STRING]);
        UA_Variant_setScalarCopy(&var[3], &inp->moduleType, &UA_TYPES[UA_TYPES_STRING]);

        retval = UA_Client_call(client, service_parent_object_id, service_methodid, 4, var, NULL, NULL);
        if(retval != UA_STATUSCODE_GOOD)
            UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,
                           "The client failed to call the add agent method %s", UA_StatusCode_name(retval));
        UA_Array_delete(var, 4, &UA_TYPES[UA_TYPES_VARIANT]);
        UA_Client_delete(client);
        UA_String_clear(&inp->Device_registry_url);
        UA_String_clear(&inp->address);
        UA_String_clear(&inp->port);
        UA_String_clear(&inp->moduleType);
        UA_String_clear(&inp->service_name);
        UA_NodeId_clear(&service_methodid);
        UA_NodeId_clear(&service_parent_object_id);
        free(url);
        free(inp);
        return UA_STATUSCODE_GOOD;
    }
}

//register agent callback
UA_StatusCode register_method_callback(UA_Server *server,
                                       const UA_NodeId *sessionId, void *sessionHandle,
                                       const UA_NodeId *methodId, void *methodContext,
                                       const UA_NodeId *objectId, void *objectContext,
                                       size_t inputSize, const UA_Variant *input,
                                       size_t outputSize, UA_Variant *output){
    UA_NodeId registered_variableid;
    UA_NodeId_init(&registered_variableid);
    get_single_node(server, UA_QUALIFIEDNAME(0, "registered"), &registered_variableid);
    //get the nodeid of the moduletype instance
    //check whether the agent is registered or not
    UA_Variant value;
    UA_Variant_init(&value);
    UA_Server_readValue(server, registered_variableid, &value);
    if(*(UA_Boolean*)value.data == true){
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,
                    "The agent is already registered");
        UA_Variant_clear(&value);
        UA_NodeId_clear(&registered_variableid);
        return UA_STATUSCODE_GOOD;
    }

    //get the name of the service from method context
    UA_Server_getNodeContext(server, *methodId, &methodContext);
    UA_register_method_context *context = (UA_register_method_context*) methodContext;

    UA_Register_function_Input_data *inp = (UA_Register_function_Input_data*) UA_calloc(1, sizeof(UA_Register_function_Input_data));
    UA_String_init(&inp->Device_registry_url);
    UA_String_init(&inp->service_name);
    UA_String_init(&inp->moduleType);
    UA_String_init(&inp->address);
    UA_String_init(&inp->port);
    UA_String_copy(&context->service_name, &inp->service_name);
    UA_String_copy(&context->module_type, &inp->moduleType);
    extract_dr_and_server_url(&inp->Device_registry_url, &inp->address, &inp->port, input);
    pthread_create(&inp->thread, NULL, register_agent, inp);

    //set registered variable to true
    UA_Variant temp;
    UA_Boolean registered = true;
    UA_Variant_setScalarCopy(&temp, &registered, &UA_TYPES[UA_TYPES_BOOLEAN]);
    UA_Server_writeValue(server, registered_variableid, temp);
    UA_Variant_clear(&temp);
    UA_NodeId_clear(&registered_variableid);
    return UA_STATUSCODE_GOOD;
}

void * unregister_agent(void *input) {

    UA_Register_function_Input_data *inp = (UA_Register_function_Input_data *)input;
    char *url = (char*) UA_calloc(inp->Device_registry_url.length+1, sizeof(char));
    memcpy(url, inp->Device_registry_url.data, inp->Device_registry_url.length);
    url[inp->Device_registry_url.length] = '\0';

    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    UA_StatusCode retval = UA_Client_connect(client, url);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,
                                                "The client failed to connect to the device registry (URL: %s) %s", url,
                                                UA_StatusCode_name(retval));

        UA_String_clear(&inp->Device_registry_url);
        UA_String_clear(&inp->address);
        UA_String_clear(&inp->port);
        UA_String_clear(&inp->service_name);
        free(inp);
        free(url);
    }
    else{
        UA_NodeId service_methodid, service_parent_object_id;
        UA_NodeId_init(&service_methodid);
        UA_NodeId_init(&service_parent_object_id);
        client_get_single_node(client, UA_QUALIFIEDNAME(0, "Remove_Agent_Server"), &service_methodid);
        client_get_single_node(client, UA_QUALIFIEDNAME(0, "PFDLServiceAgents"), &service_parent_object_id);

        UA_Variant *var = (UA_Variant *)UA_Array_new(3, &UA_TYPES[UA_TYPES_VARIANT]);
        UA_Variant_setScalarCopy(&var[2], &inp->service_name, &UA_TYPES[UA_TYPES_STRING]);
        UA_Variant_setScalarCopy(&var[0], &inp->address, &UA_TYPES[UA_TYPES_STRING]);
        UA_Variant_setScalarCopy(&var[1], &inp->port, &UA_TYPES[UA_TYPES_STRING]);
        retval = UA_Client_call(client, service_parent_object_id,
                                service_methodid, 3, var, NULL, NULL);
        if(retval != UA_STATUSCODE_GOOD) {
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,
                           "The client failed to call the remove agent method %s",
                           UA_StatusCode_name(retval));
        }
        UA_Client_delete(client);
        UA_String_clear(&inp->Device_registry_url);
        UA_String_clear(&inp->address);
        UA_String_clear(&inp->port);
        UA_String_clear(&inp->service_name);
        UA_NodeId_clear(&service_methodid);
        UA_NodeId_clear(&service_parent_object_id);
        free(inp);
        free(url);
    }
};

//unregister method callback
UA_StatusCode unregister_method_callback(UA_Server *server,
                                         const UA_NodeId *sessionId, void *sessionHandle,
                                         const UA_NodeId *methodId, void *methodContext,
                                         const UA_NodeId *objectId, void *objectContext,
                                         size_t inputSize, const UA_Variant *input,
                                         size_t outputSize, UA_Variant *output){

    UA_NodeId registered_variableid;
    UA_NodeId_init(&registered_variableid);
    get_single_node(server, UA_QUALIFIEDNAME(0, "registered"), &registered_variableid);
    //check whether the agent is registered or not
    UA_Variant value;
    UA_Server_readValue(server, registered_variableid, &value);
    if(*(UA_Boolean*)value.data == false){
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,
                    "The agent is not registered yet");
        return UA_STATUSCODE_GOOD;
    }
    //get the name of the service from method context
    UA_Server_getNodeContext(server, *methodId, &methodContext);
    UA_register_method_context *context = (UA_register_method_context*) methodContext;

    //for the unregister function, the module type name is not required (not needed by the Remove Agent function in the Device Registry)
    UA_Register_function_Input_data *inp = (UA_Register_function_Input_data*) UA_calloc(1, sizeof(UA_Register_function_Input_data));
    UA_String_init(&inp->Device_registry_url);
    UA_String_init(&inp->service_name);
    UA_String_init(&inp->moduleType);
    UA_String_init(&inp->address);
    UA_String_init(&inp->port);
    extract_dr_and_server_url(&inp->Device_registry_url, &inp->address, &inp->port, input);
    UA_String_copy(&context->service_name, &inp->service_name);
    UA_String_copy(&context->module_type, &inp->moduleType);
    pthread_create(&inp->thread, NULL, unregister_agent, inp);

    //set registered variable to false
    UA_Variant temp;
    UA_Variant_init(&temp);
    UA_Boolean registered = false;
    UA_Variant_setScalarCopy(&temp, &registered, &UA_TYPES[UA_TYPES_BOOLEAN]);
    UA_Server_writeValue(server, registered_variableid, temp);
    UA_Variant_clear(&temp);
    UA_NodeId_clear(&registered_variableid);
    return UA_STATUSCODE_GOOD;
}
