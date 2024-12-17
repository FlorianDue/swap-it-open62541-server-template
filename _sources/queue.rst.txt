..
    Licensed under the MIT License.
    For details on the licensing terms, see the LICENSE file.
    SPDX-License-Identifier: MIT

   Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)

.. _Queue Handler:

=============
Queue Handler
=============

The open62541 server template includes a Queue Object and provides a corresponding functionality which adds or removes queue entries to the queue_variable based on tge
method calls add_queue_element and remove_queue_element. Since the queue handler operates in a separate thread, the boolean running variable
of the server's main loop has to be passed to this thread, so that it terminates when the server shuts down.

.. figure:: /images/queue.PNG
   :alt: alternate textgit

.. code-block:: c

    /* Arguments:
     * UA_Server *server: server instance
     * UA_NodeId module_object_nodeId: NodeId of the instance of the Instance of the ModuleType
     * UA_Boolean *running: Pointer to the server's running variable to shut down the thread
     */
    UA_StatusCode start_queue_handler(UA_Server *server,
                                      UA_NodeId module_object_nodeId,
                                      UA_Boolean *running);