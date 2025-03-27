/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 */
#ifndef WRITE_QUEUE_H
#define WRITE_QUEUE_H

#include "queue_method_callbacks.h"

UA_StatusCode init_queue_data(UA_Server *server, UA_Queue_Data *queue_data);


#endif //WRITE_QUEUE_H
