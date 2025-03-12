..
    Licensed under the MIT License.
    For details on the licensing terms, see the LICENSE file.
    SPDX-License-Identifier: MIT

   Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)

==================
Information Models
==================

To configure a server for the SWAP-IT architecture, an instance of a SubType of the abstract ModuleType is required, so that a custom information model that extends the `SWAP-IT Common Information Model <https://github.com/FraunhoferIOSB/swap-it-common-information-model>`_
must be provided. Within this custom information model, the Services Object
of the Common Information Model's ModuleType has to be extended with a custom asset-specific OPC UA Method. To invoke this method from SWAP-IT software modules, its BrowseName must correspond
to service definition in PFDL (Production Flow Description Language)
descriptions. Besides, the custom information model must provide a corresponding EventType as SubType of the ServiceFinishedEventType and just like with the method, the BrowseName must
be equal to the name of the PFDL service.

.. figure:: /images/moduleType.PNG
   :alt: alternate text