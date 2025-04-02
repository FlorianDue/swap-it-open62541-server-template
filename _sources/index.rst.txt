..
    Licensed under the MIT License.
    For details on the licensing terms, see the LICENSE file.
    SPDX-License-Identifier: MIT

   Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)

.. toctree::
   :maxdepth: 2

==========================
Open62541 Server Template
==========================

This library is part of the SWAP-IT software modules and builds on the `open62541 OPC UA SDK <https://www.open62541.org/>`_. This software module provides a set of utility functions that can
transform any open62541 based OPC UA server application into a SWAP-IT application. Consequently, the server application will be extended with an additional set ob OPC UA Objects, Variables
and Methods to interact with other SWAP-IT software modules. The software requires a custom information model which extends the Common Information Model, as well as a JSON configuration to
customize the added information model.


.. toctree::
   :maxdepth: 2

   getting_started
   informationmodels
   json
   util
   swap-it-architecture
   related_projects
   glossary
   contact
