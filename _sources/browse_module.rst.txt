..
    Licensed under the MIT License.
    For details on the licensing terms, see the LICENSE file.
    SPDX-License-Identifier: MIT

   Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)


==========================
Browse ModuleType Instance
==========================
The following functions can be used to identify single nodes within the instance of the ModuleType. However, as the Instance only features Organizes, HasComponent and HasProperty References,
Nodes that are added with different references will not be found. Since the Instance features an undetermined Namespace Index, only the Name of the QualifiedName is used for the node matching. Consequently,
BrowseNames of Nodes within the Instance must have unique BrowseNames

.. code-block:: c

    /*
     * Method with which a server finds a node based on its browsename
     * Arguments:
     * UA_Server *server: server instance
     * UA_QualifiedName node_name: Browsename of the target node, matching is only based on its Name, not on the NamespaceIndex
     * UA_NodeId *node_nodeId: Pointer to a NodeId which returns the NodeId of the requested node
     */
    UA_StatusCode get_single_node(UA_Server *server,
                                  UA_QualifiedName node_name,
                                  UA_NodeId *node_nodeId);

    /*
     * Method with which a client finds a node based on its browsename
     * Arguments:
     * UA_Client *client: client instance
     * UA_QualifiedName node_name: Browsename of the target node, matching is only based on its Name, not on the NamespaceIndex
     * UA_NodeId *node_nodeId: Pointer to a NodeId which returns the NodeId of the requested node
     */
    UA_StatusCode client_get_single_node(UA_Client *client,
                                         UA_QualifiedName node_name,
                                         UA_NodeId *node_nodeId);
