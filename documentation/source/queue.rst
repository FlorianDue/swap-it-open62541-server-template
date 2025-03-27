..
    Licensed under the MIT License.
    For details on the licensing terms, see the LICENSE file.
    SPDX-License-Identifier: MIT

   Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)

.. _Queue Handler:

=============
Queue Handler
=============

The open62541 server template includes a Queue Object that defines a **read-only** *queue_variable*, as well as a set of methods to interact with it. The *add_queue_element* and the
*remove_queue_element* methods can be used to add or remove entries to or from the queue list respectively. The *set_queue_element_state* changes the state of a single queue element.
To change the position of a single queue element, e.g., move it upwards or downwards within the queue list, the *move_queue_element*. With the *sort_queue_elements* methods, the complete queue can be adjusted,
by providing a prioritization list, that includes an *orderId* and a *prioritization value*. The lower the prioritization value for an order, the higher the position of the element in the sorted queue. Elements that do
not provide prioritization value are added to the end of the queue, after the elements, which provide one.


.. figure:: /images/queue.PNG
   :alt: alternate text
   :width: 50%

.. code-block:: c

    /* Arguments:
     * UA_Server *server: server instance
     * UA_Queue_Data *queue_data: queue data structure defined in
     */
    UA_StatusCode init_queue_data(UA_Server *server,
                                  UA_Queue_Data *queue_data);