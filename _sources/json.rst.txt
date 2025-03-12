..
    Licensed under the MIT License.
    For details on the licensing terms, see the LICENSE file.
    SPDX-License-Identifier: MIT

   Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)

==================
JSON Configuration
==================

The following illustrates the required JSON configuration that must be provided as input for the UA_Server_swap_it method. The values for the corresponding
fields must be provided as string values.

.. code-block:: javascript

    {
        //mandatory elements
        application_name:"string",
        resource_ip:"string",
        port:"string",
        module_type:"string",
        module_name:"string",
        service_name:"string",
        //optional elements
        device_registry:"string"
        Capabilities:[
            {
                variable_name:"string",
                variable_type:"string",
                variable_value:"string",
                relational_operator:"string"
            },
            {
                variable_name:"string",
                variable_type:"string",
                variable_value:"string",
                relational_operator:"string"
            }
        ],
        channels:string,
        sessions:string
    }


Mandatory Fields
================

- application_name: Name of the server application
- resource_ip: IP address of the device
- port: Port of the server
- module_type: Name of the custom defined module type in the custom information model.
- module_name: Target BrowseName of the module_tpe instance.
- service_name: Name of the service that should be executed from SWAP-IT software modules.

Optional Fields
===============
The optional arguments enable a user to add custom capabilities to a server, or to change default settings of the server.

- device_registry: URL and port of an device registry OPC UA server
- channels: Number of secure channels provided by the server. If not set, the default number of open62541 server is used
- sessions: Number of parallel sessions the server allows. If not set, the default number of open62541 server is used.


Capabilities
------------
The capabilities specified within the JSON configuration are linked to the Capabilities Object. The `Common Information Model <https://github.com/FraunhoferIOSB/swap-it-common-information-model>`_
defines an abstract Capability_Data_Type, as well as three non abstract subtypes:

- Capability_Struct_Boolean: a boolean value will be added with the "IsTrue" and "IsFalse" as possible relational operators. The variable_type field has to be set to "bool".
- Capability_Struct_String: a string value will be added with "StringEqual" as relational operator. The variable_type field has to be set to "string".
- Capability-Struct_Number: a double value will be added with relational operators: "Equal", "Greater", "Smaller", "GreaterOrEqual", "SmallerOrEqual". The variable_type field has to be set to "numeric".

Each capability element is characterized by a:

- capability_name: BrowseName of the Capability
- capability_type: Distinguished whether the Capability_Struct_Boolean, Capability_Struct_String or the Capability_Struct_Number is used as DataType for added capability variable.
- capability_value: Value of the capability variable.
- relational_operator: Sets a relational operator in which basis the capability can be matched to product capabilities.
