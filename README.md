# SWAP Server Template
The swap server template provides a utility function UA_server_swap_it() which can be used to make any open62541 based OPC UA 
server compatible with the SWAP-IT architecture.


        /*
        * Method Arguments:
        * UA_Server *server:                           the server instance
        * UA_ByteString json                           json configuration of the server
        * UA_MethodCallback callback:                  callback of for the service method
        * UA_Boolean default_behavior:                 boolean that determines whether a default information
        *                                              model will be instantiated based on the json configuration
        * UA_Boolean *running:                         running variable from the server's main function
        * UA_Boolean register_agent_in_registry:       Boolean that determines whether the Asset should register itself in a device registry
        * UA_service_server_interpreter *swap_server   Empty structure that stores the interpreted json configuration. The stucture is filled
        *                                              when interpreting the json config within the function and can then be used for the function
        *                                              clear_swap_server
        * UA_Queue_Data *queue_data                     structure for the queue interaction
        */
        
        UA_StatusCode UA_server_swap_it(UA_Server *server,
                                        UA_ByteString json,
                                        UA_MethodCallback callback,
                                        UA_Boolean default_behavior,
                                        UA_Boolean *running,
                                        UA_Boolean register_agent_in_registry,
                                        UA_service_server_interpreter *swap_server,
                                        UA_Queue_Data *queue_data);


An extensive documentation of the swap-it-open62541-server-template can be found here: https://fraunhoferiosb.github.io/swap-it-open62541-server-template

or can be built directly from the repository. However, sphinx and the sphinx rtd themes are required. Both can be installed with:

    pip install sphinx 
    pip install sphinx-rtd-theme

Then build the documentation with:

    cd swap-it-open62541-server-template
    sphinx-build -M html documentation/source/ documentation/build/


## Requirements
Installed version of the open62541 OPC UA SDK version 1.3.10 (https://github.com/open62541/open62541). Since custom information models
are integrated into the servers, the build flag
    
    -DUA_NAMESPACE_ZERO=FULL -DUA_ENABLE_JSON_ENCODING=ON -DBUILD_SHARED_LIBS=ON

must be set. Depending on the server configuration, the flag

    -DUA_MULTITHREADING

must be set to, e.g.

    -DUA_MULTITHREADING > 200

to enable the server to open the corresponding number of channels.

**Important Note:**
The template is also tested with the open62541 version v.1.4.6.


## Installation
    
    /*install dependencies check and open62541)*/
    apt-get -y update
    apt-get -y install git build-essential gcc pkg-config cmake python3 check
    git clone https://github.com/open62541/open62541
    cd open62541
    
    /*switch to open62541 version 1.3.10*/
    git fetch --all --tags
    git checkout tags/v1.3.10 -b v1.3.10-branch

    /* init submodules, build and install open62541*/
    git submodule update --init --recursive
    mkdir build && cd build
    cmake -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo -DUA_NAMESPACE_ZERO=FULL -DUA_ENABLE_JSON_ENCODING=ON -DUA_MULTITHREADING=200 ..
    make install

    /*install the server template*/
    git clone https://github.com/FraunhoferIOSB/swap-it-open62541-server-template
    cd swap-it-open62541-server-template    
    mkdir build && cd build
    cmake ..
    make install

## Related Projects
Since the SWAP-IT open62541 server template is part of the SWAP-IT Architecture, its application is linked to other SWAP-IT projects. Here are some other relevant repositories:

- SWAP-IT Demo Scenario: https://github.com/swap-it/demo-scenario
- SWAP-IT Common Information Model: https://github.com/FraunhoferIOSB/swap-it-common-information-model
- SWAP-IT Execution Engine: https://github.com/FraunhoferIOSB/swap-it-execution-engine
- SWAP-IT Registry Module: https://github.com/FraunhoferIOSB/swap-it-registry-module
- PFDL Scheduler: https://github.com/iml130/pfdl
- SWAP-IT Dashboard: https://github.com/iml130/swap-it-dashboard
