/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 * Copyright 2024 (c) Fraunhofer IWU (Author: Thorben Schnellhardt)
 */

#include "config_interpreter.h"
#include "cj5.h"
#include <stdio.h>
#include "assert.h"

UA_StatusCode get_server_dict(UA_service_server_interpreter *swap_server, UA_ByteString json){
    //allocate memory for pointer in UA_Service_server_interpreter
    swap_server->nbr_registry_subscriptions=0;
    swap_server->registry_subscriptions = (char **) UA_calloc(0, sizeof(char*));
    swap_server->nbr_capabilities = 0;
    swap_server->capabilities = (UA_capabilities *) UA_calloc(0, sizeof(UA_capabilities));
    swap_server->service_name = (char*) UA_calloc(0, sizeof(char));
    swap_server->resource_ip = (char*) UA_calloc(0, sizeof(char));
    swap_server->module_name = (char*) UA_calloc(0, sizeof(char));
    swap_server->module_type = (char*) UA_calloc(0, sizeof(char));
    swap_server->server_name = (char*) UA_calloc(0, sizeof(char));
    swap_server->device_registry_url = (char*) UA_calloc(0, sizeof(char));


    cj5_token tokens[256];
    cj5_result r = cj5_parse((const char *)json.data, (unsigned int)json.length, tokens, 256, NULL);
    const char *g_token_types[] = {"CJ5_TOKEN_OBJECT", "CJ5_TOKEN_ARRAY", "CJ5_TOKEN_NUMBER",
                                   "CJ5_TOKEN_STRING", "CJ5_TOKEN_BOOL", "CJ5_TOKEN_NULL"};

    // print complete parsed JSON for test and debug purpose
    if(!r.error) {
        for(unsigned int i = 0; i < r.num_tokens; i++) {
            char content[8*1024];
            unsigned int num = tokens[i].end - tokens[i].start + 1;
            memcpy(content, (const char *)&json.data[tokens[i].start], num);
            content[num] = '\0';
            assert(tokens[i].type <= CJ5_TOKEN_NULL);
            /*printf("%d: { type = %s, size = %d, parent = %d, content = '%s'\n", i,
                   g_token_types[tokens[i].type], tokens[i].size, tokens[i].parent_id, content);*/
        }
    } else {
        printf("ERROR: %d - line:%d (%d)\n", r.error, r.error_line, r.error_col);
        if(r.error == CJ5_ERROR_OVERFLOW) {
            printf("COUNT: %d\n", r.num_tokens);    // actual number of tokens needed
        }
        return UA_STATUSCODE_BADCONFIGURATIONERROR;
    }

    unsigned int idx = 0;
    cj5_error_code err;
    unsigned int local_object_index = idx;

    cj5_find(&r, &idx, "application_name");
    char buf_name[256];
    err = cj5_get_str(&r, idx, buf_name, NULL);
    if (err != CJ5_ERROR_NONE){
        return UA_STATUSCODE_BADCONFIGURATIONERROR;
    }
    swap_server->server_name = (char *) UA_realloc(swap_server->server_name, (strlen(buf_name)+1)*sizeof(char));
    memset(swap_server->server_name, (int)(strlen(buf_name)+1), sizeof(char));
    strcpy(swap_server->server_name, buf_name);
    idx = local_object_index;

    cj5_find(&r, &idx, "resource_ip");
    char buf_ip[256];
    err = cj5_get_str(&r, idx, buf_ip, NULL);
    if (err != CJ5_ERROR_NONE){
        return UA_STATUSCODE_BADCONFIGURATIONERROR;
    }
    swap_server->resource_ip = (char *) UA_realloc(swap_server->resource_ip, (strlen(buf_ip)+1)*sizeof(char));
    memset(swap_server->resource_ip, (int)(strlen(buf_ip)+1), sizeof(char));
    strcpy(swap_server->resource_ip, buf_ip);
    idx = local_object_index;

    char buf_port[64];
    cj5_find(&r, &idx, "port");
    err = cj5_get_str(&r, idx, buf_port, NULL);
    if (err != CJ5_ERROR_NONE){
        return UA_STATUSCODE_BADCONFIGURATIONERROR;
    }
    swap_server->port = (UA_UInt16) atoi(buf_port);
    idx = local_object_index;
    /*
     * cj5_find(&r, &idx, "sessions");
    err = cj5_get_str(&r, idx, buf_sessions, NULL);
    if (err == CJ5_ERROR_INVALID || err == CJ5_ERROR_INCOMPLETE || err == CJ5_ERROR_OVERFLOW || err == CJ5_ERROR_NOTFOUND ) {
        swap_server->sessions = 0;
    }
    else{
        swap_server->sessions = (UA_UInt16) atoi(buf_sessions);
    }
     * */

    cj5_find(&r, &idx, "device_registry");
    char dr_ip[256];
    err = cj5_get_str(&r, idx, dr_ip, NULL);
    if (err == CJ5_ERROR_INVALID || err == CJ5_ERROR_INCOMPLETE || err == CJ5_ERROR_OVERFLOW || err == CJ5_ERROR_NOTFOUND ) {
        char *temp = "device_registry";
        strcpy(dr_ip, temp);
        dr_ip[strlen(temp)+1] = '\0';
    }
    swap_server->device_registry_url = (char *) UA_realloc(swap_server->device_registry_url, (strlen(dr_ip)+1)*sizeof(char));
    memset(swap_server->device_registry_url, (int)(strlen(dr_ip)+1), sizeof(char));
    strcpy(swap_server->device_registry_url, dr_ip);
    idx = local_object_index;

    char buf_channels[64];
    cj5_find(&r, &idx, "channels");
    err = cj5_get_str(&r, idx, buf_channels, NULL);
    if (err == CJ5_ERROR_INVALID || err == CJ5_ERROR_INCOMPLETE || err == CJ5_ERROR_OVERFLOW || err == CJ5_ERROR_NOTFOUND ) {
        swap_server->channels = 0;
    }
    else{
        swap_server->channels = (UA_UInt16) atoi(buf_channels);
    }

    idx = local_object_index;

    char buf_sessions[64];
    cj5_find(&r, &idx, "sessions");
    err = cj5_get_str(&r, idx, buf_sessions, NULL);
    if (err == CJ5_ERROR_INVALID || err == CJ5_ERROR_INCOMPLETE || err == CJ5_ERROR_OVERFLOW || err == CJ5_ERROR_NOTFOUND ) {
        swap_server->sessions = 0;
    }
    else{
        swap_server->sessions = (UA_UInt16) atoi(buf_sessions);
    }

    idx = local_object_index;
    cj5_find(&r, &idx, "service_name");
    char buf_service_name[256];
    err = cj5_get_str(&r, idx, buf_service_name, NULL);
    if (err != CJ5_ERROR_NONE){
        return UA_STATUSCODE_BADCONFIGURATIONERROR;
    }
    swap_server->service_name = (char *) UA_realloc(swap_server->service_name, (strlen(buf_service_name)+1)*sizeof(char));
    memset(swap_server->service_name, (int)(strlen(buf_service_name)+1), sizeof(char));
    strcpy(swap_server->service_name, buf_service_name);
    idx = local_object_index;

    cj5_find(&r, &idx, "module_type");
    char buf_module_type_name[256];
    err = cj5_get_str(&r, idx, buf_module_type_name, NULL);
    if (err != CJ5_ERROR_NONE){
        return UA_STATUSCODE_BADCONFIGURATIONERROR;
    }
    swap_server->module_type = (char *) UA_realloc(swap_server->module_type ,(strlen(buf_module_type_name)+1)*sizeof(char));
    memset(swap_server->module_type, (int)(strlen(buf_module_type_name)+1), sizeof(char));
    strcpy(swap_server->module_type, buf_module_type_name);

    idx = local_object_index;
    cj5_find(&r, &idx, "module_name");
    char buf_module_name[256];
    err = cj5_get_str(&r, idx, buf_module_name, NULL);
    if (err != CJ5_ERROR_NONE){
        return UA_STATUSCODE_BADCONFIGURATIONERROR;
    }
    swap_server->module_name = (char *) UA_realloc(swap_server->module_name, (strlen(buf_module_name)+1)*sizeof(char));
    memset(swap_server->module_name, (int)(strlen(buf_module_name)+1), sizeof(char));
    strcpy(swap_server->module_name, buf_module_name);

    idx = local_object_index;
    cj5_find(&r, &idx, "registry_subscriptions");
    unsigned int inner_local_mapping_index = idx;
    if(tokens[idx].type != CJ5_TOKEN_ARRAY){
        printf("Registry Subscriptions not configured!\n");
    }
    else{
        for (unsigned int j = 0; j < tokens[inner_local_mapping_index].size; ++j) {
            swap_server->nbr_registry_subscriptions++;
            swap_server->registry_subscriptions = (char **) UA_realloc(swap_server->registry_subscriptions, swap_server->nbr_registry_subscriptions*sizeof(char*));
            idx += 1;
            cj5_find(&r, &idx, "object");
            char buf_subscription_object[512];
            err = cj5_get_str(&r, idx, buf_subscription_object, NULL);
            if (err != CJ5_ERROR_NONE){
                printf("No Nodes are specified to be subscribed \n");
                return UA_STATUSCODE_BADCONFIGURATIONERROR;
            }
            swap_server->registry_subscriptions[swap_server->nbr_registry_subscriptions-1] = (char*) UA_calloc(strlen(buf_subscription_object)+1, sizeof(char));
            strcpy(swap_server->registry_subscriptions[swap_server->nbr_registry_subscriptions-1], buf_subscription_object);
        }
    }

    idx = local_object_index;
    cj5_find(&r, &idx, "Capabilities");
    inner_local_mapping_index = idx;
    if(tokens[idx].type != CJ5_TOKEN_ARRAY){
        printf("Capabilitites not configured!\n");
    }
    else{
        for (unsigned int j = 0; j < tokens[inner_local_mapping_index].size; ++j) {
            swap_server->nbr_capabilities++;
            swap_server->capabilities = (UA_capabilities *) UA_realloc(swap_server->capabilities,
                                                                               swap_server->nbr_capabilities *
                                                                               sizeof(UA_capabilities));
            idx += 1;
            unsigned int inner_local_object_index = idx;
            cj5_find(&r, &idx, "variable_name");
            char buf_variable_name[512];
            err = cj5_get_str(&r, idx, buf_variable_name, NULL);
            if (err != CJ5_ERROR_NONE) {
                return UA_STATUSCODE_BADCONFIGURATIONERROR;
            }
            swap_server->capabilities[swap_server->nbr_capabilities - 1].variable_name = (char *) UA_calloc(
                    strlen(buf_variable_name) + 1, sizeof(char));
            strcpy(swap_server->capabilities[swap_server->nbr_capabilities - 1].variable_name, buf_variable_name);
            idx = inner_local_object_index;
            cj5_find(&r, &idx, "variable_type");
            char buf_variable_type[512];
            err = cj5_get_str(&r, idx, buf_variable_type, NULL);
            if (err != CJ5_ERROR_NONE) {
                return UA_STATUSCODE_BADCONFIGURATIONERROR;
            }
            swap_server->capabilities[swap_server->nbr_capabilities - 1].data_type = (char *) UA_calloc(
                            strlen(buf_variable_type) + 1, sizeof(char));
            strcpy(swap_server->capabilities[swap_server->nbr_capabilities - 1].data_type, buf_variable_type);
            idx = inner_local_object_index;
            cj5_find(&r, &idx, "variable_value");
            char buf_variable_value[512];
            err = cj5_get_str(&r, idx, buf_variable_value, NULL);
            if (err != CJ5_ERROR_NONE) {
                return UA_STATUSCODE_BADCONFIGURATIONERROR;
            }
            char *num = "numeric", *str = "string", *bo = "bool";
            if (strcmp(swap_server->capabilities[swap_server->nbr_capabilities - 1].data_type, num) == 0) {
                swap_server->capabilities[swap_server->nbr_capabilities - 1].identifier = numeric;
                swap_server->capabilities[swap_server->nbr_capabilities - 1].value.numeric = (UA_Double) atof(buf_variable_value);
            }
            else if (strcmp(swap_server->capabilities[swap_server->nbr_capabilities - 1].data_type, str) == 0) {
                swap_server->capabilities[swap_server->nbr_capabilities - 1].identifier = string;
                swap_server->capabilities[swap_server->nbr_capabilities - 1].value.string = UA_String_fromChars(buf_variable_value);
            }
            else if (strcmp(swap_server->capabilities[swap_server->nbr_capabilities - 1].data_type, bo) == 0) {
                swap_server->capabilities[swap_server->nbr_capabilities - 1].identifier = boolean;
                char *false_1 = "False", *false_2 = "false", *false_3 = "1", *true_1 = "True", *true_2 = "true", *true_3 = "0";
                if (strcmp(buf_variable_value, false_1) == 0 || strcmp(buf_variable_value, false_2) == 0 ||
                        strcmp(buf_variable_value, false_3) == 0) {
                    swap_server->capabilities[swap_server->nbr_capabilities - 1].identifier = boolean;
                    swap_server->capabilities[swap_server->nbr_capabilities - 1].value.boolean = UA_FALSE;
                }
                else if (strcmp(buf_variable_value, true_1) == 0 || strcmp(buf_variable_value, true_2) == 0 ||
                        strcmp(buf_variable_value, true_3) == 0) {
                    swap_server->capabilities[swap_server->nbr_capabilities - 1].identifier = boolean;
                    swap_server->capabilities[swap_server->nbr_capabilities - 1].value.boolean = UA_TRUE;
                }
                else {
                    printf("wrong value for boolean\n");
                }
            }
            else {
                printf("unknown data type for capability used\n");
            }
            idx = inner_local_object_index;
            cj5_find(&r, &idx, "relational_operator");
            char buf_rel_operator[512];
            err = cj5_get_str(&r, idx, buf_rel_operator, NULL);
            if (err != CJ5_ERROR_NONE) {
                return UA_STATUSCODE_BADCONFIGURATIONERROR;
            }
            char *equal = "Equal", *greater = "Greater", *smaller = "Smaller", *goe = "GreaterOrEqual", *soe = "SmallerOrEqual";
            char *IsTrue = "IsTrue", *IsFalse = "IsFalse", *eString = "EqualString";
            if (strcmp(buf_rel_operator, equal) == 0) {
                swap_server->capabilities[swap_server->nbr_capabilities - 1].relational_operator = 0;
            } else if (strcmp(buf_rel_operator, greater) == 0) {
                swap_server->capabilities[swap_server->nbr_capabilities - 1].relational_operator = 1;
            } else if (strcmp(buf_rel_operator, smaller) == 0) {
                swap_server->capabilities[swap_server->nbr_capabilities - 1].relational_operator = 2;
            } else if (strcmp(buf_rel_operator, goe) == 0) {
                swap_server->capabilities[swap_server->nbr_capabilities - 1].relational_operator = 3;
            } else if (strcmp(buf_rel_operator, soe) == 0) {
                swap_server->capabilities[swap_server->nbr_capabilities - 1].relational_operator = 4;
            } else if (strcmp(buf_rel_operator, IsTrue) == 0) {
                swap_server->capabilities[swap_server->nbr_capabilities - 1].relational_operator = 5;
            } else if (strcmp(buf_rel_operator, IsFalse) == 0) {
                swap_server->capabilities[swap_server->nbr_capabilities - 1].relational_operator = 6;
            } else if (strcmp(buf_rel_operator, eString) == 0) {
                swap_server->capabilities[swap_server->nbr_capabilities - 1].relational_operator = 7;
            } else {
                printf("unknown relational operator\n");
            }
        }
    }
    return UA_STATUSCODE_GOOD;
}