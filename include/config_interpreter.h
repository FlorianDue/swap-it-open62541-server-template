/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 */

#ifndef OPEN62541_CONFIG_INTERPRETER_H
#define OPEN62541_CONFIG_INTERPRETER_H

#include <pthread.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

typedef enum{
    numeric = 0,
    boolean,
    string
}UA_capability_identifier;

typedef struct{
    char *variable_name;
    char *data_type;
    UA_capability_identifier identifier;
    UA_UInt32 relational_operator;
    union{
        UA_Double numeric;
        UA_String string;
        UA_Boolean boolean;
    }value;
}UA_capabilities;

typedef struct{
    char *server_name;
    char *resource_ip;
    UA_UInt16 port;
    UA_UInt16 channels;
    UA_UInt16 sessions;
    char *module_type;
    char *module_name;
    char *service_name;
    char *device_registry_url;
    char **registry_subscriptions;
    size_t nbr_registry_subscriptions;
    UA_capabilities *capabilities;
    size_t nbr_capabilities;
}UA_service_server_interpreter;

UA_StatusCode get_server_dict(UA_service_server_interpreter *swap_server, UA_ByteString json);

#endif  // OPEN62541_CONFIG_INTERPRETER_H
