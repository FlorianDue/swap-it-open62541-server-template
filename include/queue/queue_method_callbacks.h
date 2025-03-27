/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 */

#ifndef QUEUE_METHOD_CALLBACKS_H
#define QUEUE_METHOD_CALLBACKS_H

#include "queue_handling_utils.h"

UA_StatusCode move_queue_element_method_callback(UA_Server *server,
                                                 const UA_NodeId *sessionId, void *sessionHandle,
                                                 const UA_NodeId *methodId, void *methodContext,
                                                 const UA_NodeId *objectId, void *objectContext,
                                                 size_t inputSize, const UA_Variant *input,
                                                 size_t outputSize, UA_Variant *output);

UA_StatusCode set_queue_element_state_method_callback(UA_Server *server,
                             const UA_NodeId *sessionId, void *sessionHandle,
                             const UA_NodeId *methodId, void *methodContext,
                             const UA_NodeId *objectId, void *objectContext,
                             size_t inputSize, const UA_Variant *input,
                             size_t outputSize, UA_Variant *output);

UA_StatusCode remove_from_queue_method_callback(UA_Server *server,
                             const UA_NodeId *sessionId, void *sessionHandle,
                             const UA_NodeId *methodId, void *methodContext,
                             const UA_NodeId *objectId, void *objectContext,
                             size_t inputSize, const UA_Variant *input,
                             size_t outputSize, UA_Variant *output);

UA_StatusCode add_to_queue_method_callback(UA_Server *server,
                            const UA_NodeId *sessionId, void *sessionHandle,
                            const UA_NodeId *methodId, void *methodContext,
                            const UA_NodeId *objectId, void *objectContext,
                            size_t inputSize, const UA_Variant *input,
                            size_t outputSize, UA_Variant *output);

UA_StatusCode sort_queue_elements_method_callback(UA_Server *server,
                             const UA_NodeId *sessionId, void *sessionHandle,
                             const UA_NodeId *methodId, void *methodContext,
                             const UA_NodeId *objectId, void *objectContext,
                             size_t inputSize, const UA_Variant *input,
                             size_t outputSize, UA_Variant *output);

#endif //QUEUE_METHOD_CALLBACKS_H
