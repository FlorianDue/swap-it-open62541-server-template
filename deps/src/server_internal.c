/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 * Copyright 2024 (c) Fraunhofer IWU (Author: Thorben Schnellhardt)
 */

#include <stdio.h>
#include "server_internal.h"
#include "node_finder.h"
#include "register_callbacks.h"

UA_StatusCode write_state_variable(UA_Server *server){
    UA_NodeId asset_state_nodeId;
    UA_StatusCode retval = get_single_node(server, UA_QUALIFIEDNAME(0, "AssetState"), &asset_state_nodeId);
    if(retval != UA_STATUSCODE_GOOD){
        printf("failed to find the AssetState Variable\n");
        return retval;
    }
    UA_Int32 service_state = UA_ASSETSTATETYPE_ASSET_STATE_IDLE;
    UA_Variant state;
    UA_Variant_init(&state);
    UA_Variant_setScalar(&state, &service_state, &UA_TYPES[UA_TYPES_INT32]);
    return UA_Server_writeValue(server, asset_state_nodeId, state);
}

UA_StatusCode write_registered_variable(UA_Server *server, UA_Boolean value){
    UA_NodeId registered_variable_nodeId;
    UA_StatusCode retval = get_single_node(server, UA_QUALIFIEDNAME(0, "registered"), &registered_variable_nodeId);
    if(retval != UA_STATUSCODE_GOOD){
        printf("failed to find the registered Variable\n");
        return retval;
    }
    //set te registered varible to false
    UA_Variant temp;
    UA_Variant_init(&temp);
    UA_Variant_setScalarCopy(&temp, &value, &UA_TYPES[UA_TYPES_BOOLEAN]);
    retval = UA_Server_writeValue(server, registered_variable_nodeId, temp);
    UA_Variant_clear(&temp);
    return retval;
}

UA_StatusCode add_method_callback(UA_Server *server, UA_service_server_interpreter *service_server, UA_MethodCallback callback){
    UA_NodeId service_method_nodeId;
    UA_StatusCode retval = get_single_node(server, UA_QUALIFIEDNAME(0, service_server->service_name), &service_method_nodeId);
    if(retval != UA_STATUSCODE_GOOD){
        printf("Failed to find the service Method Node\n");
        return retval;
    }
    return UA_Server_setMethodNodeCallback(server, service_method_nodeId,(UA_MethodCallback) callback);
}

UA_StatusCode find_method(UA_Server *server, char *methodname, UA_NodeId *service_method_nodeId){
    return get_single_node(server, UA_QUALIFIEDNAME(0, methodname), service_method_nodeId);
}

UA_StatusCode add_register_methods(UA_Server *server, UA_NodeId *register_function_nodeId, UA_NodeId *unregister_function_nodeId){
    /*UA_NodeId register_function_nodeId;
    UA_NodeId unregister_function_nodeId;
    UA_NodeId_init(&register_function_nodeId);
    UA_NodeId_init(&unregister_function_nodeId);*/
    UA_StatusCode retval = find_method(server, "register", register_function_nodeId);
    if(retval != UA_STATUSCODE_GOOD){
        printf("failed to find the register function\n");
        return retval;
    }
    retval = find_method(server, "unregister", unregister_function_nodeId);
    if(retval != UA_STATUSCODE_GOOD){
        printf("failed to find the unregister function\n");
        return retval;
    }
    retval = UA_Server_setMethodNodeCallback(server, *register_function_nodeId, (UA_MethodCallback) register_method_callback);
    if(retval != UA_STATUSCODE_GOOD){
        printf("failed to link register function\n");
        return retval;
    }
    retval = UA_Server_setMethodNodeCallback(server, *unregister_function_nodeId, (UA_MethodCallback) unregister_method_callback);
    if(retval != UA_STATUSCODE_GOOD){
        printf("failed to link the unregister function\n");
        return retval;
    }
    return retval;
}

static UA_StatusCode add_object(UA_Server *server, char *name, UA_NodeId ObjectTypeNodeId, UA_NodeId *ModuleTypeInstancenodeId){
    UA_ObjectAttributes Attr = UA_ObjectAttributes_default;
    Attr.displayName = UA_LOCALIZEDTEXT("en-US", name);
    UA_StatusCode retval = UA_Server_addObjectNode(server,UA_NODEID_NULL, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(1, name),
                            ObjectTypeNodeId ,Attr, NULL, ModuleTypeInstancenodeId);
    return retval;
}

UA_StatusCode instantiate_module_type(UA_Server *server, UA_service_server_interpreter *service_server, UA_NodeId *module_type_instance_nodeId){
    UA_String module_type = UA_String_fromChars("ModuleType");
    UA_NodeId base_module_type_nodeId;
    UA_NodeId_init(&base_module_type_nodeId);
    UA_StatusCode retval = get_module_type(server, &base_module_type_nodeId, module_type, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE));
    if(retval != UA_STATUSCODE_GOOD){
        printf("failed to find the ObjectType ModuleType. Please Check if the common namespace was successfully added \n");
        return retval;
    }
    UA_String_clear(&module_type);
    UA_String module_type_name = UA_String_fromChars(service_server->module_type);
    UA_NodeId module_type_id;
    UA_NodeId_init(&module_type_id);
    retval = get_module_type(server, &module_type_id, module_type_name, base_module_type_nodeId);
    if(retval != UA_STATUSCODE_GOOD){
        printf("failed to find the Subtype of the BaseModuleType.\n");
        return retval;
    }
    UA_String_clear(&module_type_name);
    retval = add_object(server, service_server->module_name, module_type_id, module_type_instance_nodeId);
    return retval;
}

UA_StatusCode add_subscription_objects(UA_Server *server, UA_service_server_interpreter *service_server, UA_NodeId moduletype_instance_nodeId){
    if(service_server->nbr_registry_subscriptions > 0){
        UA_QualifiedName *q_name_array = UA_Array_new(service_server->nbr_registry_subscriptions, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
        for(size_t i=0; i< service_server->nbr_registry_subscriptions; i++){
            UA_NodeId object_nodeId;
            UA_NodeId_init(&object_nodeId);
            get_object_node(server, moduletype_instance_nodeId, &object_nodeId, service_server->registry_subscriptions[i]);
            UA_QualifiedName_init(&q_name_array[i]);
            UA_Server_readBrowseName(server, object_nodeId, &q_name_array[i]);
            UA_NodeId_clear(&object_nodeId);
        }
        UA_NodeId subscription_objects_variable_nodeId;
        UA_NodeId_init(&subscription_objects_variable_nodeId);
        get_single_node(server, UA_QUALIFIEDNAME(0, "Subscription_Objects"), &subscription_objects_variable_nodeId);
        UA_Variant value;
        UA_Variant_init(&value);
        UA_Variant_setArrayCopy(&value, q_name_array, service_server->nbr_registry_subscriptions, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
        UA_StatusCode retval = UA_Server_writeValue(server, subscription_objects_variable_nodeId, value);
        if(retval != UA_STATUSCODE_GOOD){
            printf("failed to write the subscription object to the server\n");
            return retval;
        }
        UA_Variant_clear(&value);
        UA_NodeId_clear(&subscription_objects_variable_nodeId);
        UA_Array_delete(q_name_array, service_server->nbr_registry_subscriptions, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode add_variable(UA_Server *server, UA_NodeId data_type_nodeId, UA_NodeId parent_NodeId, char *variable_name, UA_Variant value){
    UA_VariableAttributes att = UA_VariableAttributes_default;
    att.dataType = data_type_nodeId;
    att.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    att.displayName = UA_LOCALIZEDTEXT("en-us", variable_name);
    UA_NodeId var_nodeId;
    UA_NodeId_init(&var_nodeId);
    UA_StatusCode retval = UA_Server_addVariableNode(server, UA_NODEID_NULL, parent_NodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), UA_QUALIFIEDNAME(0, variable_name),
                              UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), att, NULL, &var_nodeId);
    if(retval != UA_STATUSCODE_GOOD){
        printf("failed to add the variable\n");
    }
    if(value.data != NULL){
        UA_Server_writeValue(server, var_nodeId, value);
    }
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode add_capability_nodes(UA_Server *server, UA_service_server_interpreter *service_server, UA_NodeId moduletype_instance_nodeId){
    UA_NodeId data_type_nodeId, capability_node_NodeId;
    UA_NodeId_init(&data_type_nodeId);
    UA_NodeId_init(&capability_node_NodeId);
    UA_Variant value;
    UA_Variant_init(&value);
    UA_StatusCode retval = get_object_node(server, moduletype_instance_nodeId, &capability_node_NodeId, "Capabilities");
    if(retval != UA_STATUSCODE_GOOD){
        printf("failed to find the Capabilities Object\n");
        return retval;
    }
    //iterate over the list of capabilities
    for(size_t i=0; i< service_server->nbr_capabilities; i++){
        UA_String cs_struct_data_type = UA_String_fromChars("Capability_Struct_Data_Type");
        if(service_server->capabilities[i].identifier == numeric){
            UA_String cs_number = UA_String_fromChars("Capability_Struct_Number");
            retval = server_get_data_type_node(server, cs_number, &data_type_nodeId, cs_struct_data_type);
            if(retval != UA_STATUSCODE_GOOD){
                printf("failed to find the Capability_Struct_Number data type\n");
                return retval;
            }
            UA_Capability_Struct_Number val;
            UA_Capability_Struct_Number_init(&val);
            UA_Double_copy(&service_server->capabilities[i].value.numeric, &val.value);
            if(service_server->capabilities[i].relational_operator == (UA_UInt32) 0
            || service_server->capabilities[i].relational_operator == (UA_UInt32) 1
            || service_server->capabilities[i].relational_operator == (UA_UInt32) 2
            || service_server->capabilities[i].relational_operator == (UA_UInt32) 3
            || service_server->capabilities[i].relational_operator == (UA_UInt32) 4 ){
                val.relational_Operator = service_server->capabilities[i].relational_operator;
            }
            else{
                printf("wrong relational operator for boolean data type\n");
            }
            UA_Variant_setScalarCopy(&value, &val, &UA_TYPES_COMMON[UA_TYPES_COMMON_CAPABILITY_STRUCT_NUMBER]);
            UA_String_clear(&cs_number);
            UA_Double_clear(&service_server->capabilities[i].value.numeric);
        }
        else if(service_server->capabilities[i].identifier == string){
            UA_String cs_string = UA_String_fromChars("Capability_Struct_String");
            retval = server_get_data_type_node(server, cs_string, &data_type_nodeId, cs_struct_data_type);
            if(retval != UA_STATUSCODE_GOOD){
                printf("failed to find the Capability_Struct_String data type\n");
                return retval;
            }
            UA_Capability_Struct_String val;
            UA_Capability_Struct_String_init(&val);
            UA_String_copy(&service_server->capabilities[i].value.string, &val.value);
            if(service_server->capabilities[i].relational_operator == (UA_UInt32) 7){
                val.relational_Operator = service_server->capabilities[i].relational_operator;
            }
            else{
                printf("wrong relational operator for string data type\n");
            }
            UA_Variant_setScalarCopy(&value, &val, &UA_TYPES_COMMON[UA_TYPES_COMMON_CAPABILITY_STRUCT_STRING]);
            UA_String_clear(&val.value);
            UA_String_clear(&cs_string);
            UA_String_clear(&service_server->capabilities[i].value.string);
        }
        else if(service_server->capabilities[i].identifier == boolean){
            UA_String cs_boolean = UA_String_fromChars("Capability_Struct_Boolean");
            retval = server_get_data_type_node(server, cs_boolean, &data_type_nodeId, cs_struct_data_type);
            if(retval != UA_STATUSCODE_GOOD){
                printf("failed to find the Capability_Struct_Boolean data type\n");
                return retval;
            }
            UA_Capability_Struct_Boolean val;
            UA_Capability_Struct_Boolean_init(&val);
            UA_Boolean_copy(&service_server->capabilities[i].value.boolean, &val.value);
            if(service_server->capabilities[i].relational_operator == (UA_UInt32) 5 || service_server->capabilities[i].relational_operator == (UA_UInt32) 6){
                val.relational_Operator = service_server->capabilities[i].relational_operator;
            }
            else{
                printf("wrong relational operator for boolean data type\n");
            }
            UA_String_clear(&cs_boolean);
            UA_Boolean_clear(&service_server->capabilities[i].value.boolean);
            UA_Variant_setScalarCopy(&value, &val, &UA_TYPES_COMMON[UA_TYPES_COMMON_CAPABILITY_STRUCT_BOOLEAN]);
        }
        else{
            printf("unknown data type\n");
            return UA_STATUSCODE_BAD;
        }
        //clear capability info
        UA_String_clear(&cs_struct_data_type);
        add_variable(server, data_type_nodeId, capability_node_NodeId, service_server->capabilities[i].variable_name, value);
        free(service_server->capabilities[i].variable_name);
        free(service_server->capabilities[i].data_type);
        UA_Variant_clear(&value);
    }
    return UA_STATUSCODE_GOOD;
}