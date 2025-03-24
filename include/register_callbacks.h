/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 * Copyright 2024 (c) Fraunhofer IWU (Author: Thorben Schnellhardt)
 */

#ifndef REGISTER_CALLBACKS_H
#define REGISTER_CALLBACKS_H


#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>

typedef struct{
    UA_String service_name;
    UA_String address;
    UA_String port;
    UA_String moduleType;
    UA_String Device_registry_url;
    pthread_t thread;
}UA_Register_function_Input_data;

typedef struct{
    UA_String service_name;
    UA_String module_type;
}UA_register_method_context;

void * register_agent(void *input);

void * unregister_agent(void *input);

UA_StatusCode register_method_callback(UA_Server *server,
                                              const UA_NodeId *sessionId, void *sessionHandle,
                                              const UA_NodeId *methodId, void *methodContext,
                                              const UA_NodeId *objectId, void *objectContext,
                                              size_t inputSize, const UA_Variant *input,
                                              size_t outputSize, UA_Variant *output);

UA_StatusCode unregister_method_callback(UA_Server *server,
                                         const UA_NodeId *sessionId, void *sessionHandle,
                                         const UA_NodeId *methodId, void *methodContext,
                                         const UA_NodeId *objectId, void *objectContext,
                                         size_t inputSize, const UA_Variant *input,
                                         size_t outputSize, UA_Variant *output);

#endif //REGISTER_CALLBACKS_H
