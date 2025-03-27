/*

Licensed under the MIT License.
For details on the licensing terms, see the LICENSE file.
SPDX-License-Identifier: MIT

Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
*/
#pragma GCC diagnostic ignored "-Wcast-function-type"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <check.h>
#include <math.h>
#include "swap_it.h"
#include "node_finder.h"
#include "test_include/service_callbacks.h"
#include "pthread.h"

UA_Boolean server_running = false;
static void stopHandler(int sign) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "received ctrl-c");
    server_running = false;
}

//static UA_Server *server = NULL;
static void setup(void) {
    printf("start_test\n");
    /*server = UA_Server_new();
    ck_assert(server != NULL);*/
}

static void teardown(void) {
    printf("test completed \n");
    //UA_Server_delete(server);
}

char *config_empty= "";
char *config_min = "{\n"
                   "  application_name: \"warehouse_dr1\",\n"
                   "  resource_ip: \"localhost\",\n"
                   "  port: \"5000\",\n"
                   "  module_type: \"WarehouseModuleType\",\n"
                   "  module_name: \"WarehouseModule\",\n"
                   "  service_name: \"GetPartsFromWarehouse\"\n"
                   "}";
char *correct_config = "{\n"
                       "  application_name: \"warehouse_dr1\",\n"
                       "  resource_ip: \"localhost\",\n"
                       "  port: \"5000\",\n"
                       "  module_type: \"WarehouseModuleType\",\n"
                       "  module_name: \"WarehouseModule\",\n"
                       "  service_name: \"GetPartsFromWarehouse\",\n"
                       "  device_registry:\"opc.tcp://localhost:4841\",\n"
                       "  registry_subscriptions: [\n"
                       "    {\n"
                       "      object:\"State\"\n"
                       "    },\n"
                       "    {\n"
                       "      object:\"Capabilities\"\n"
                       "    }\n"
                       "  ],\n"
                       "  Capabilities: [\n"
                       "    {\n"
                       "      variable_name: \"test numeric\",\n"
                       "      variable_type: \"numeric\",\n"
                       "      variable_value: \"120\",\n"
                       "      relational_operator: \"Smaller\"\n"
                       "    },\n"
                       "    {\n"
                       "      variable_name: \"test string\",\n"
                       "      variable_type: \"string\",\n"
                       "      variable_value: \"test string\",\n"
                       "      relational_operator: \"EqualString\"\n"
                       "    },\n"
                       "    {\n"
                       "      variable_name: \"test boolean\",\n"
                       "      variable_type: \"bool\",\n"
                       "      variable_value: \"false\",\n"
                       "      relational_operator: \"IsFalse\"\n"
                       "    }\n"
                       "  ],\n"
                       "  channels:\"100\",\n"
                       "  sessions:\"100\"\n"
                       "}";


START_TEST(empty_config){
    printf("\n");
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Start Test empty_config.");
    printf("\n");
    UA_ByteString json = UA_String_fromChars(config_empty);
    /*instantiate the warehousemoduletype*/
    UA_Server *server = UA_Server_new();
    UA_service_server_interpreter *swap_server = (UA_service_server_interpreter*) UA_calloc(1, sizeof(UA_service_server_interpreter));
    UA_Queue_Data queue_data;
    memset(&queue_data, 0, sizeof(UA_Queue_Data));
    UA_StatusCode retval = UA_server_swap_it(server, json, warehousemethodCallback, UA_FALSE, &server_running, UA_FALSE, swap_server, &queue_data);
    ck_assert(retval != UA_STATUSCODE_GOOD);
    UA_Server_run_shutdown(server);
    UA_Server_delete(server);
    UA_ByteString_clear(&json);
    free(swap_server);
}

void *server_terminator(void *data){
    unsigned int time = *(unsigned int*) data;
    sleep(time);
    server_running = false;
}

void check_object_instance(UA_Server* server, UA_QualifiedName qname){
    UA_NodeId temp = UA_NODEID_NULL;
    UA_NodeId null = UA_NODEID_NULL;
    UA_Boolean equal_id = UA_NodeId_equal(&temp, &null);
    ck_assert(equal_id == true);

    get_single_node(server, qname, &temp);
    equal_id = UA_NodeId_equal(&temp, &null);
    ck_assert(equal_id == false);
    UA_NodeId_clear(&temp);
}

void clear_and_init_variant_and_nodeId(UA_Variant *variant, UA_NodeId *nodeId){
    UA_Variant_clear(variant);
    UA_Variant_init(variant);
    UA_NodeId_clear(nodeId);
    UA_NodeId_init(nodeId);
}

START_TEST(check_object_instances){
    printf("\n");
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "check_object_instances.");
    printf("\n");
    UA_Server *server = UA_Server_new();
    UA_StatusCode retval = namespace_common_generated(server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = namespace_pfdl_parameter_generated(server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = namespace_warehouse_generated(server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    UA_ByteString json = UA_String_fromChars(correct_config);
    /*instantiate the warehousemoduletype*/
    UA_service_server_interpreter *swap_server = (UA_service_server_interpreter*) UA_calloc(1, sizeof(UA_service_server_interpreter));
    /*browse the intatiated objects */
    UA_Queue_Data queue_data;
    retval = UA_server_swap_it(server, json, warehousemethodCallback, UA_FALSE, &server_running, UA_FALSE, swap_server, &queue_data);
    ck_assert(retval == UA_STATUSCODE_GOOD);

    UA_Variant value;
    UA_Variant_init(&value);
    UA_NodeId temp = UA_NODEID_NULL;
    UA_NodeId null = UA_NODEID_NULL;
    UA_Boolean equal_id = UA_NodeId_equal(&temp, &null);
    ck_assert(equal_id == true);

    UA_QualifiedName qnames[12] = {
            UA_QUALIFIEDNAME(4, "WarehouseModule"),
            UA_QUALIFIEDNAME(4, "Capabilities"),
            UA_QUALIFIEDNAME(4, "Properties"),
            UA_QUALIFIEDNAME(4, "Queue"),
            UA_QUALIFIEDNAME(4, "Registry_Subscriptions"),
            UA_QUALIFIEDNAME(4, "Services"),
            UA_QUALIFIEDNAME(4, "GetPartsFromWarehouse"),
            UA_QUALIFIEDNAME(4, "State"),
            UA_QUALIFIEDNAME(4, "test boolean"),
            UA_QUALIFIEDNAME(4, "test numeric"),
            UA_QUALIFIEDNAME(4, "test string"),
            UA_QUALIFIEDNAME(4, "Subscription_Objects")
    };
    for(size_t i=0; i< 12 ;i++)
        check_object_instance(server, qnames[i]);

    /*check capability variables*/
    get_single_node(server, UA_QUALIFIEDNAME(2, "test numeric"), &temp);
    UA_Server_readValue(server, temp, &value);
    UA_Capability_Struct_Number number = *(UA_Capability_Struct_Number*) value.data;
    ck_assert(number.value == 120);
    ck_assert(number.relational_Operator == 2);
    clear_and_init_variant_and_nodeId(&value, &temp);

    get_single_node(server, UA_QUALIFIEDNAME(2, "test boolean"), &temp);
    UA_Server_readValue(server, temp, &value);
    UA_Capability_Struct_Boolean boolean = *(UA_Capability_Struct_Boolean*) value.data;
    ck_assert(boolean.value == false);
    ck_assert(boolean.relational_Operator == 6);
    clear_and_init_variant_and_nodeId(&value, &temp);

    get_single_node(server, UA_QUALIFIEDNAME(2, "test string"), &temp);
    UA_Server_readValue(server, temp, &value);
    UA_Capability_Struct_String string = *(UA_Capability_Struct_String*) value.data;
    UA_String str_val = UA_String_fromChars("test string");
    UA_Boolean str_result = UA_String_equal(&str_val, &string.value);
    ck_assert(str_result == true);
    ck_assert(string.relational_Operator == 7);
    clear_and_init_variant_and_nodeId(&value, &temp);
    UA_String_clear(&str_val);

    /*check registry subscriptions variable*/
    get_single_node(server, UA_QUALIFIEDNAME(2, "Subscription_Objects"), &temp);
    UA_Server_readValue(server, temp, &value);
    UA_QualifiedName *read_value = (UA_QualifiedName*) value.data;
    UA_QualifiedName *actual_value = UA_calloc(2, sizeof(UA_QualifiedName));
    actual_value[0] = UA_QUALIFIEDNAME(2, "State");
    actual_value[1] = UA_QUALIFIEDNAME(2, "Capabilities");
    for(size_t i=0; i<2;i++){
        UA_Boolean qual_result = UA_QualifiedName_equal(&actual_value[i], &read_value[i]);
        ck_assert(qual_result == true);
    }
    free(actual_value);
    UA_NodeId_clear(&temp);
    UA_Variant_clear(&value);
    pthread_t threadId_test2;
    unsigned int time = 10;
    pthread_create(&threadId_test2, NULL, server_terminator, &time);
    while(server_running)
        UA_Server_run_iterate(server, NULL);
    clear_swap_server(swap_server, UA_FALSE, server, &queue_data);
    UA_Server_run_shutdown(server);
    UA_Server_delete(server);
    free(swap_server);
    UA_ByteString_clear(&json);

}

UA_StatusCode addAgentCallbacks(UA_Server *reg_server,
                                      const UA_NodeId *sessionId, void *sessionHandle,
                                      const UA_NodeId *methodId, void *methodContext,
                                      const UA_NodeId *objectId, void *objectContext,
                                      size_t inputSize, const UA_Variant *input,
                                      size_t outputSize, UA_Variant *output){

    UA_String act_agent_service = UA_String_fromChars("GetPartsFromWarehouse");
    UA_Boolean str_result = UA_String_equal(&act_agent_service, (UA_String*) input[0].data);
    UA_String_clear(&act_agent_service);
    ck_assert(str_result == true);

    UA_String act_agent_url = UA_String_fromChars("localhost");
    str_result = UA_String_equal(&act_agent_url, (UA_String*) input[1].data);
    UA_String_clear(&act_agent_url);
    ck_assert(str_result == true);

    UA_String act_agent_port = UA_String_fromChars("5000");
    str_result = UA_String_equal(&act_agent_port, (UA_String*) input[2].data);
    UA_String_clear(&act_agent_port);
    ck_assert(str_result == true);

    UA_String act_agent_module = UA_String_fromChars("WarehouseModule");
    str_result = UA_String_equal(&act_agent_module, (UA_String*) input[3].data);
    UA_String_clear(&act_agent_module);
    ck_assert(str_result == true);


    return UA_STATUSCODE_GOOD;
}

UA_StatusCode removeAgentCallbacks(UA_Server *reg_server,
                                const UA_NodeId *sessionId, void *sessionHandle,
                                const UA_NodeId *methodId, void *methodContext,
                                const UA_NodeId *objectId, void *objectContext,
                                size_t inputSize, const UA_Variant *input,
                                size_t outputSize, UA_Variant *output){

    UA_String act_agent_url = UA_String_fromChars("localhost");
    UA_Boolean str_result = UA_String_equal(&act_agent_url, (UA_String*) input[0].data);
    UA_String_clear(&act_agent_url);
    ck_assert(str_result == true);

    UA_String act_agent_port = UA_String_fromChars("5000");
    str_result = UA_String_equal(&act_agent_port, (UA_String*) input[1].data);
    UA_String_clear(&act_agent_port);
    ck_assert(str_result == true);

    UA_String act_agent_service = UA_String_fromChars("GetPartsFromWarehouse");
    str_result = UA_String_equal(&act_agent_service, (UA_String*) input[2].data);
    UA_String_clear(&act_agent_service);
    ck_assert(str_result == true);

    return UA_STATUSCODE_GOOD;
}

static void
addUnregisterMethod(UA_Server *reg_server, UA_NodeId parent) {
    UA_Argument inputArgument[3];
    UA_Argument_init(&inputArgument[0]);
    inputArgument[0].name = UA_STRING("agent_url");
    inputArgument[0].dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    inputArgument[0].valueRank = UA_VALUERANK_SCALAR;

    UA_Argument_init(&inputArgument[1]);
    inputArgument[1].name = UA_STRING("agent_port");
    inputArgument[1].dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    inputArgument[1].valueRank = UA_VALUERANK_SCALAR;

    UA_Argument_init(&inputArgument[2]);
    inputArgument[2].name = UA_STRING("agent_service");
    inputArgument[2].dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    inputArgument[2].valueRank = UA_VALUERANK_SCALAR;

    UA_MethodAttributes helloAttr = UA_MethodAttributes_default;
    helloAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Remove Agent");
    helloAttr.executable = true;
    helloAttr.userExecutable = true;
    UA_Server_addMethodNode(reg_server, UA_NODEID_STRING(1, "Remove_Agent_Server"),
                            parent,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME(1, "Remove_Agent_Server"),
                            helloAttr, removeAgentCallbacks,
                            3, inputArgument, 0, NULL, NULL, NULL);
}

static void
addRegisterMethod(UA_Server *reg_server, UA_NodeId parent) {
    UA_Argument inputArgument[4];
    UA_Argument_init(&inputArgument[0]);
    inputArgument[0].name = UA_STRING("service_name");
    inputArgument[0].dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    inputArgument[0].valueRank = UA_VALUERANK_SCALAR;

    UA_Argument_init(&inputArgument[1]);
    inputArgument[1].name = UA_STRING("address");
    inputArgument[1].dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    inputArgument[1].valueRank = UA_VALUERANK_SCALAR;

    UA_Argument_init(&inputArgument[2]);
    inputArgument[2].name = UA_STRING("port");
    inputArgument[2].dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    inputArgument[2].valueRank = UA_VALUERANK_SCALAR;

    UA_Argument_init(&inputArgument[3]);
    inputArgument[3].name = UA_STRING("moduleType");
    inputArgument[3].dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    inputArgument[3].valueRank = UA_VALUERANK_SCALAR;

    UA_MethodAttributes helloAttr = UA_MethodAttributes_default;
    helloAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Add Agent");
    helloAttr.executable = true;
    helloAttr.userExecutable = true;
    UA_Server_addMethodNode(reg_server, UA_NODEID_NULL,
                            parent,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME(1, "Add_Agent_Server"),
                            helloAttr, addAgentCallbacks,
                            4, inputArgument, 0, NULL, NULL, NULL);
}

void *client_thread_register(void *data){
    sleep(5);
    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://localhost:5000");
    ck_assert(retval == UA_STATUSCODE_GOOD);

    UA_NodeId registerId = UA_NODEID_NULL;
    UA_NodeId unregisterId = UA_NODEID_NULL;
    UA_NodeId servicesId = UA_NODEID_NULL;
    UA_NodeId null = UA_NODEID_NULL;
    client_get_single_node(client, UA_QUALIFIEDNAME(2, "Services"), &servicesId);
    UA_Boolean equal_id = UA_NodeId_equal(&servicesId, &null);
    ck_assert(equal_id == false);
    client_get_single_node(client, UA_QUALIFIEDNAME(2, "register"), &registerId);
    equal_id = UA_NodeId_equal(&registerId, &null);
    ck_assert(equal_id == false);
    client_get_single_node(client, UA_QUALIFIEDNAME(2, "unregister"), &unregisterId);
    equal_id = UA_NodeId_equal(&unregisterId, &null);
    ck_assert(equal_id == false);

    UA_Variant *out_val= UA_Variant_new();
    UA_Variant *in = (UA_Variant*) UA_calloc(2, sizeof(UA_Variant));
    size_t *output_size;
    UA_String *reg_inp = (UA_String*) UA_calloc(2, sizeof(UA_String));
    reg_inp[0] = UA_String_fromChars("localhost:4841");
    reg_inp[1] = UA_String_fromChars("localhost:5000");
    UA_Variant_setScalarCopy(&in[0], &reg_inp[0], &UA_TYPES[UA_TYPES_STRING]);
    UA_Variant_setScalarCopy(&in[1], &reg_inp[1], &UA_TYPES[UA_TYPES_STRING]);

    retval = UA_Client_call(client, servicesId, registerId, 2, in, output_size, &out_val);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = UA_Client_call(client, servicesId, unregisterId, 2, in, output_size, &out_val);
    ck_assert(retval == UA_STATUSCODE_GOOD);

    UA_Variant_clear(&in[0]);
    UA_Variant_clear(&in[1]);
    free(in);
    UA_String_clear(&reg_inp[0]);
    UA_String_clear(&reg_inp[1]);
    free(reg_inp);
    UA_Variant_delete(out_val);
    UA_Client_disconnect(client);
    UA_Client_delete(client);
}

void *server_mock_registry(void *data){
    UA_Server *reg_server = UA_Server_new();
    UA_ServerConfig *conf = UA_Server_getConfig(reg_server);
    UA_StatusCode retval = UA_ServerConfig_setMinimal(conf, 4841, NULL);
    ck_assert(retval == UA_STATUSCODE_GOOD);

    UA_NodeId agent_listId = UA_NODEID_NULL;
    UA_ObjectAttributes agent_list = UA_ObjectAttributes_default;
    agent_list.displayName = UA_LOCALIZEDTEXT("en-us", "AgentList");
    UA_Server_addObjectNode(reg_server, UA_NODEID_NULL,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME(0, "AgentList"), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),agent_list, NULL, &agent_listId);

    UA_NodeId pfdl_service_agentId = UA_NODEID_NULL;
    UA_ObjectAttributes PFDL_agent = UA_ObjectAttributes_default;
    PFDL_agent.displayName = UA_LOCALIZEDTEXT("en-us", "PFDLServiceAgents");
    UA_Server_addObjectNode(reg_server, UA_NODEID_NULL,
                            agent_listId,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME(0, "PFDLServiceAgents"), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),PFDL_agent, NULL, &pfdl_service_agentId);

    addRegisterMethod(reg_server, pfdl_service_agentId);
    addUnregisterMethod(reg_server, pfdl_service_agentId);
    while(server_running)
        UA_Server_run_iterate(reg_server, NULL);
    UA_Server_run_shutdown(reg_server);
    UA_Server_delete(reg_server);
}

START_TEST(check_register_callbacks_from_client){
    UA_Server *new_server = UA_Server_new();
    printf("\n");
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "check_register_callbacks_from_client.");
    printf("\n");
    server_running = true;
    UA_StatusCode retval = namespace_common_generated(new_server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = namespace_pfdl_parameter_generated(new_server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = namespace_warehouse_generated(new_server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    UA_ByteString json = UA_String_fromChars(correct_config);
    /*instantiate the warehousemoduletype*/
    UA_service_server_interpreter *swap_server = (UA_service_server_interpreter*) UA_calloc(1, sizeof(UA_service_server_interpreter));
    UA_Queue_Data queue_data;
    retval = UA_server_swap_it(new_server, json, warehousemethodCallback, UA_FALSE, &server_running, UA_FALSE, swap_server, &queue_data);
    ck_assert(retval == UA_STATUSCODE_GOOD);

    pthread_t threadId_2;
    pthread_create(&threadId_2, NULL, server_mock_registry, NULL);
    sleep(10);
    pthread_t threadId_test1;
    pthread_create(&threadId_test1, NULL, client_thread_register, NULL);
    pthread_t threadId_test2;
    unsigned int time = 20;
    pthread_create(&threadId_test2, NULL, server_terminator, &time);
    while(server_running)
        UA_Server_run_iterate(new_server, NULL);

    clear_swap_server(swap_server, UA_FALSE, new_server, &queue_data);
    UA_Server_run_shutdown(new_server);
    UA_Server_delete(new_server);
    free(swap_server);
    UA_ByteString_clear(&json);
}

START_TEST(check_register_callbacks_from_config){
    printf("\n");
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "check_register_callbacks_from_config.");
    printf("\n");
    server_running = true;
    pthread_t threadId_2;
    pthread_create(&threadId_2, NULL, server_mock_registry, NULL);
    sleep(5);
    UA_Server *new_server = UA_Server_new();
    UA_StatusCode retval = namespace_common_generated(new_server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = namespace_pfdl_parameter_generated(new_server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = namespace_warehouse_generated(new_server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    UA_ByteString json = UA_String_fromChars(correct_config);
    UA_service_server_interpreter *swap_server = (UA_service_server_interpreter*) UA_calloc(1, sizeof(UA_service_server_interpreter));
    UA_Queue_Data queue_data;
    retval = UA_server_swap_it(new_server, json, warehousemethodCallback, UA_FALSE, &server_running, UA_TRUE, swap_server, &queue_data);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    pthread_t threadId_test2;
    unsigned int time = 25;
    pthread_create(&threadId_test2, NULL, server_terminator, &time);
    while(server_running)
        UA_Server_run_iterate(new_server, NULL);
    clear_swap_server(swap_server, UA_TRUE, new_server, &queue_data);
    UA_Server_run_shutdown(new_server);
    UA_Server_delete(new_server);
    free(swap_server);
    UA_ByteString_clear(&json);
}

START_TEST(check_register_callbacks_from_config_without_dr){
    printf("\n");
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "check_register_callbacks_from_config_without_dr.");
    printf("\n");
    server_running = true;
    UA_Server *new_server = UA_Server_new();
    UA_StatusCode retval = namespace_common_generated(new_server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = namespace_pfdl_parameter_generated(new_server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = namespace_warehouse_generated(new_server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    UA_ByteString json = UA_String_fromChars(config_min);
    UA_service_server_interpreter *swap_server = (UA_service_server_interpreter*) UA_calloc(1, sizeof(UA_service_server_interpreter));
    UA_Queue_Data queue_data;
    retval = UA_server_swap_it(new_server, json, warehousemethodCallback, UA_FALSE, &server_running, UA_TRUE, swap_server, &queue_data);
    ck_assert(retval == UA_STATUSCODE_GOOD);

    pthread_t threadId_test2;
    unsigned int time = 25;
    pthread_create(&threadId_test2, NULL, server_terminator, &time);
    while(server_running)
        UA_Server_run_iterate(new_server, NULL);
    clear_swap_server(swap_server, UA_TRUE, new_server, &queue_data);
    UA_Server_run_shutdown(new_server);
    UA_Server_delete(new_server);
    free(swap_server);
    UA_ByteString_clear(&json);
}

void *client_thread_service_method(void *data){
    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://localhost:5000");
    ck_assert(retval == UA_STATUSCODE_GOOD);
    /*todo add event subscription*/

    UA_NodeId getPartsId = UA_NODEID_NULL;
    UA_NodeId servicesId = UA_NODEID_NULL;
    UA_NodeId null = UA_NODEID_NULL;
    client_get_single_node(client, UA_QUALIFIEDNAME(2, "GetPartsFromWarehouse"), &getPartsId);
    UA_Boolean equal_id = UA_NodeId_equal(&getPartsId, &null);
    ck_assert(equal_id == false);
    client_get_single_node(client, UA_QUALIFIEDNAME(4, "Services"), &servicesId);
    equal_id = UA_NodeId_equal(&servicesId, &null);
    ck_assert(equal_id == false);
    UA_Variant *out = UA_Variant_new();
    size_t *output_size;
    retval = UA_Client_call(client, servicesId, getPartsId, 0, NULL, output_size, &out);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    UA_Variant_delete(out);
    UA_Client_disconnect(client);
    UA_Client_delete(client);
}

START_TEST(check_service_method_call){
    printf("\n");
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "check_service_method_call.");
    printf("\n");
    UA_Server *server = UA_Server_new();
    server_running = true;
    UA_StatusCode retval = namespace_common_generated(server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = namespace_pfdl_parameter_generated(server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = namespace_warehouse_generated(server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    UA_ByteString json = UA_String_fromChars(correct_config);
    /*instantiate the warehousemoduletype*/
    UA_service_server_interpreter *swap_server = (UA_service_server_interpreter*) UA_calloc(1, sizeof(UA_service_server_interpreter));
    /*browse the intatiated objects */
    UA_Queue_Data queue_data;
    retval = UA_server_swap_it(server, json, warehousemethodCallback, UA_FALSE, &server_running, UA_FALSE, swap_server, &queue_data);
    ck_assert(retval == UA_STATUSCODE_GOOD);

    pthread_t threadId;
    pthread_create(&threadId, NULL, client_thread_service_method, NULL);
    pthread_t threadId_test2;
    unsigned int time = 45;
    pthread_create(&threadId_test2, NULL, server_terminator, &time);
    while(server_running)
        UA_Server_run_iterate(server, NULL);
    clear_swap_server(swap_server, UA_FALSE, server, &queue_data);
    UA_Server_run_shutdown(server);
    UA_Server_delete(server);
    free(swap_server);
    UA_ByteString_clear(&json);
}

START_TEST(load_swap_it_module){
    printf("\n");
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Start Test load_swap_it_module.");
    printf("\n");
    UA_Server *server = UA_Server_new();
    UA_StatusCode retval = namespace_common_generated(server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = namespace_pfdl_parameter_generated(server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = namespace_warehouse_generated(server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    server_running = UA_TRUE;
    UA_ByteString json = UA_String_fromChars(correct_config);
    /*instantiate the warehousemoduletype*/
    UA_service_server_interpreter *swap_server = (UA_service_server_interpreter*) UA_calloc(1, sizeof(UA_service_server_interpreter));
    UA_Queue_Data queue_data;
    retval = UA_server_swap_it(server, json, warehousemethodCallback, UA_FALSE, &server_running, UA_FALSE, swap_server, &queue_data);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    pthread_t threadId_test2;
    unsigned int time = 10;
    pthread_create(&threadId_test2, NULL, server_terminator, &time);
    while(server_running)
        UA_Server_run_iterate(server, NULL);
    clear_swap_server(swap_server, UA_FALSE, server, &queue_data);
    UA_Server_run_shutdown(server);
    UA_Server_delete(server);
    free(swap_server);
    UA_ByteString_clear(&json);
}


void *client_check_queue(void *data){
    const UA_DataTypeArray *types = (UA_DataTypeArray*) data;
    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    UA_ClientConfig *cc = UA_Client_getConfig(client);
    cc->customDataTypes = types;

    UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://localhost:5000");
    ck_assert(retval == UA_STATUSCODE_GOOD);



    UA_NodeId queue_object = UA_NODEID_NULL;
    UA_NodeId add_to_queue = UA_NODEID_NULL;
    UA_NodeId remove_from_queue = UA_NODEID_NULL;
    UA_NodeId queue_variable = UA_NODEID_NULL;
    UA_NodeId null = UA_NODEID_NULL;
    client_get_single_node(client, UA_QUALIFIEDNAME(2, "ServiceQueue"), &queue_object);
    UA_Boolean equal_id = UA_NodeId_equal(&queue_object, &null);
    ck_assert(equal_id == false);
    client_get_single_node(client, UA_QUALIFIEDNAME(2, "add_queue_element"), &add_to_queue);
    equal_id = UA_NodeId_equal(&add_to_queue, &null);
    ck_assert(equal_id == false);
    client_get_single_node(client, UA_QUALIFIEDNAME(2, "remove_queue_element"), &remove_from_queue);
    equal_id = UA_NodeId_equal(&remove_from_queue, &null);
    ck_assert(equal_id == false);
    client_get_single_node(client, UA_QUALIFIEDNAME(2, "queue_variable"), &queue_variable);
    equal_id = UA_NodeId_equal(&queue_variable, &null);
    ck_assert(equal_id == false);

    UA_Queue_Data_Type queue_element_1;
    UA_Queue_Data_Type_init(&queue_element_1);
    queue_element_1.client_Identifier = UA_String_fromChars("Test_Client");
    queue_element_1.orderId = UA_String_fromChars("Test_Product");
    queue_element_1.service_UUID = UA_String_fromChars("Test_Service");

    UA_Queue_Data_Type queue_element_2;
    UA_Queue_Data_Type_init(&queue_element_2);
    queue_element_2.client_Identifier = UA_String_fromChars("Test_Client_2");
    queue_element_2.orderId = UA_String_fromChars("Test_Product_2");
    queue_element_2.service_UUID = UA_String_fromChars("Test_Service_2");

    /*create a new queue element*/
    UA_Variant input;
    UA_Variant_init(&input);
    UA_Variant_setScalarCopy(&input, &queue_element_1, &UA_TYPES_COMMON[UA_TYPES_COMMON_QUEUE_DATA_TYPE]);
    /*add the queue element to the server*/
    UA_Variant *out_val = UA_Variant_new();
    size_t *output_size;
    retval = UA_Client_call(client, queue_object, add_to_queue, 1, &input, output_size, &out_val);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    UA_Variant_clear(out_val);
    UA_Variant_clear(&input);

    UA_Variant_setScalarCopy(&input, &queue_element_2, &UA_TYPES_COMMON[UA_TYPES_COMMON_QUEUE_DATA_TYPE]);
    retval = UA_Client_call(client, queue_object, add_to_queue, 1, &input, output_size, &out_val);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    UA_Variant_clear(out_val);
    UA_Variant_clear(&input);
    /*wait 2 sec so that the element is added to the queue (call is asynchronous)*/
    sleep(2);
    /*check if the element was added to the queue*/
    retval = UA_Client_readValueAttribute(client, queue_variable, out_val);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    /*check the number of queue elements*/
    ck_assert(out_val->arrayLength == 2);

    UA_Queue_Data_Type *read_value = (UA_Queue_Data_Type*) out_val->data;

    /*UA_String out = UA_STRING_NULL;
    UA_print(out_val, &UA_TYPES[UA_TYPES_VARIANT], &out);
    printf("read Variant after adding element : %.*s\n", (int)out.length, out.data);
    UA_String_clear(&out);


    UA_print(&read_value[0], &UA_TYPES_COMMON[UA_TYPES_COMMON_QUEUE_DATA_TYPE], &out);
    printf("Queue Variable after adding element : %.*s\n", (int)out.length, out.data);
    UA_String_clear(&out);

    UA_print(&read_value[1], &UA_TYPES_COMMON[UA_TYPES_COMMON_QUEUE_DATA_TYPE], &out);
    printf("Queue Variable after adding element : %.*s\n", (int)out.length, out.data);
    UA_String_clear(&out);*/

    /*check if the read queue elements are correct*/
    /*equal_id = UA_String_equal(&read_value[0].service_UUID, &queue_element_1.service_UUID);
    ck_assert(equal_id == false);
    equal_id = UA_String_equal(&read_value[0].client_Identifier, &queue_element_1.client_Identifier);
    ck_assert(equal_id == false);
    equal_id = UA_String_equal(&read_value[0].productId, &queue_element_1.productId);
    ck_assert(equal_id == false);
    UA_Int16 tmp = 1;
    equal_id = UA_Int16_equal(&read_value[0].entry_Number, &tmp);
    ck_assert(equal_id == false);
    UA_Queue_State_Variable_Type state = UA_QUEUE_STATE_VARIABLE_TYPE_READY_FOR_EXECUTION;
    equal_id = UA_Queue_State_Variable_Type_equal(&state, &read_value[0].queue_Element_State);
    ck_assert(equal_id == false);

    equal_id = UA_String_equal(&read_value[1].service_UUID, &queue_element_2.service_UUID);
    ck_assert(equal_id == false);
    equal_id = UA_String_equal(&read_value[1].client_Identifier, &queue_element_2.client_Identifier);
    ck_assert(equal_id == false);
    equal_id = UA_String_equal(&read_value[1].productId, &queue_element_2.productId);
    ck_assert(equal_id == false);
    tmp = 2;
    equal_id = UA_Int16_equal(&read_value[1].entry_Number, &tmp);
    ck_assert(equal_id == false);
    state = UA_QUEUE_STATE_VARIABLE_TYPE_WAITING_FOR_EXECUTION;
    equal_id = UA_Queue_State_Variable_Type_equal(&state, &read_value[1].queue_Element_State);
    ck_assert(equal_id == false);*/

    UA_Variant_setScalarCopy(&input, &queue_element_1, &UA_TYPES_COMMON[UA_TYPES_COMMON_QUEUE_DATA_TYPE]);
    retval = UA_Client_call(client, queue_object, remove_from_queue, 1, &input, output_size, &out_val);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    UA_Variant_clear(out_val);
    UA_Variant_clear(&input);
    /*wait 2 sec so that the element is added to the queue (call is asynchronous)*/
    sleep(2);
    /*check if the element was added to the queue*/
    retval = UA_Client_readValueAttribute(client, queue_variable, out_val);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    /*check the number of queue elements*/
    ck_assert(out_val->arrayLength == 1);

    /*equal_id = UA_String_equal(&read_value[1].service_UUID, &queue_element_2.service_UUID);
    ck_assert(equal_id == false);
    equal_id = UA_String_equal(&read_value[1].client_Identifier, &queue_element_2.client_Identifier);
    ck_assert(equal_id == false);
    equal_id = UA_String_equal(&read_value[1].productId, &queue_element_2.productId);
    ck_assert(equal_id == false);
    tmp = 1;
    equal_id = UA_Int16_equal(&read_value[1].entry_Number, &tmp);
    ck_assert(equal_id == false);
    state = UA_QUEUE_STATE_VARIABLE_TYPE_READY_FOR_EXECUTION;
    equal_id = UA_Queue_State_Variable_Type_equal(&state, &read_value[1].queue_Element_State);
    ck_assert(equal_id == false);*/

    UA_Variant_setScalarCopy(&input, &queue_element_2, &UA_TYPES_COMMON[UA_TYPES_COMMON_QUEUE_DATA_TYPE]);
    retval = UA_Client_call(client, queue_object, remove_from_queue, 1, &input, output_size, &out_val);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    UA_Variant_clear(out_val);
    UA_Variant_clear(&input);
    /*wait 2 sec so that the element is added to the queue (call is asynchronous)*/
    sleep(2);
    /*check if the element was added to the queue*/
    retval = UA_Client_readValueAttribute(client, queue_variable, out_val);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    /*check the number of queue elements*/
    ck_assert(out_val->arrayLength == 0);



    UA_Variant_delete(out_val);
    UA_Client_disconnect(client);
    UA_Client_delete(client);
}

START_TEST(check_queue_handler){
    printf("\n");
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Start Test check_queue_handler.");
    printf("\n");
    server_running = true;
    UA_Server *new_server = UA_Server_new();

    UA_StatusCode retval = namespace_common_generated(new_server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = namespace_pfdl_parameter_generated(new_server);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = namespace_warehouse_generated(new_server);
    ck_assert(retval == UA_STATUSCODE_GOOD);

    UA_ByteString json = UA_String_fromChars(correct_config);
    /*instantiate the warehousemoduletype*/
    UA_service_server_interpreter *swap_server = (UA_service_server_interpreter*) UA_calloc(1, sizeof(UA_service_server_interpreter));
    UA_Queue_Data queue_data;
    retval = UA_server_swap_it(new_server, json, warehousemethodCallback, UA_FALSE, &server_running, UA_FALSE, swap_server, &queue_data);
    ck_assert(retval == UA_STATUSCODE_GOOD);

    pthread_t threadId;
    const UA_DataTypeArray *types = UA_Server_getConfig(new_server)->customDataTypes;
    pthread_create(&threadId, NULL, client_check_queue, (void*) types);
    pthread_t threadId_test2;
    unsigned int time = 20;
    pthread_create(&threadId_test2, NULL, server_terminator, &time);
    while(server_running)
        UA_Server_run_iterate(new_server, NULL);
    clear_swap_server(swap_server, UA_FALSE, new_server, &queue_data);
    UA_Server_run_shutdown(new_server);
    UA_Server_delete(new_server);
    free(swap_server);
    UA_ByteString_clear(&json);
}

int main(void) {
    Suite *s = suite_create("open62541-server-template");
    TCase *tc_call = tcase_create("open62541-server-template");
    tcase_add_checked_fixture(tc_call, setup, teardown);
    tcase_add_test(tc_call, load_swap_it_module);
    tcase_add_test(tc_call, empty_config);
    tcase_add_test(tc_call, check_object_instances);
    tcase_add_test(tc_call, check_service_method_call);
    //tcase_add_test(tc_call, check_register_callbacks_from_client);
    tcase_add_test(tc_call, check_register_callbacks_from_config);
    tcase_add_test(tc_call, check_register_callbacks_from_config_without_dr);
    //tcase_add_test(tc_call, check_queue_handler);
    suite_add_tcase(s, tc_call);

    SRunner *sr = srunner_create(s);

    srunner_set_fork_status(sr, CK_NOFORK);
    srunner_run_all(sr, CK_NORMAL);
    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}