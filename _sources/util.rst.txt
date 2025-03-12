..
    Licensed under the MIT License.
    For details on the licensing terms, see the LICENSE file.
    SPDX-License-Identifier: MIT

   Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)

=================
Utility Functions
=================

The open62541 server template provides the following set of utility functions. To enable a simple integration, only the UA_Server_swap_it function needs
to be invoked, either at the server's start or during runtime, to make the server SWAP-IT compatible. The additional utility functions are used within the
UA_Server_swap_it function, however they can also be used separately for your individual applications.

.. toctree::
   :maxdepth: 2

   ua_swap_it
   register
   module
   queue
   browse_module
   interpret_conf