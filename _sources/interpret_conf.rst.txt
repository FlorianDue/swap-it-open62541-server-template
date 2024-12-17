..
    Licensed under the MIT License.
    For details on the licensing terms, see the LICENSE file.
    SPDX-License-Identifier: MIT

   Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)

.. _Interpret Server Configuration:

===============================
Interpret Server Configuration
===============================

.. code-block:: c

    /*Structure type that holds all information of a json-based swap-it configuration of a server*/
    typedef struct{
        char *server_name;
        UA_UInt16 port;
        UA_UInt16 channels;
        UA_UInt16 sessions;
        char *module_type;
        char *module_name;
        char *service_name;
        char **registry_subscriptions;
        size_t nbr_registry_subscriptions;
        UA_capabilities *capabilities;
        size_t nbr_capabilities;
    }UA_service_server_interpreter;

    /*
     * Method to interpret a swap-it server configuration
     * Arguments:
     * UA_service_server_interpreter *swap_server: Structure that returns all information from the json configuration
     * UA_ByteString json: UA_ByteString that holds a json payload with the swap-it configuration for a server
     */
    UA_StatusCode get_server_dict(UA_service_server_interpreter *swap_server,
                                  UA_ByteString json);