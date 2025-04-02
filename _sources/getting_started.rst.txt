..
    Licensed under the MIT License.
    For details on the licensing terms, see the LICENSE file.
    SPDX-License-Identifier: MIT

   Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)

.. toctree::
   :maxdepth: 2

===============
Getting Started
===============

With the open62541-server-template, a swap server can be configured with only a few steps. The example_server directory in the repository contains an example
implementation of a SWAP-IT server. In addition, the `Demonstration Scenario <https://github.com/swap-it/demo-scenario>`_ provides a set of `Tutorials <https://github.com/swap-it/demo-scenario/tree/main/Tutorials>`_,
which utilize the open62541-server-template, so that these tutorials can be worked through for a better understanding of the SWAP-IT architecture. However, the following only highlights how the server example
of this tutorial can be executed.

Install Dependencies
====================

open62541 installation
------------------------------
The open62541-server-template requires a local install of the `open62541 OPC UA SDK <https://github.com/open62541/open62541>`_ version 1.3.10:

.. code-block:: c

    /*install dependencies*/
    apt-get -y update
    apt-get -y install git build-essential gcc pkg-config cmake check python3

    /*clone open62541 library and init submodules*/
    git clone https://github.com/open62541/open62541
    cd open62541
    git fetch --all --tags
    git checkout tags/v1.3.10 -b v1.3.10-branch
    git submodule update --init --recursive

    /*install open62541*/
    mkdir build && cd build
    cmake -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo -DUA_NAMESPACE_ZERO=FULL -DUA_ENABLE_JSON_ENCODING=ON -DUA_MULTITHREADING=200 ..
    make install

open62541-server-template installation
----------------------------------------

Afterwards, the open62541-server-template can be installed:

.. code-block:: c

    /*clone the swap-it-server-template*/
    git clone https://github.com/FraunhoferIOSB/swap-it-open62541-server-template
    cd swap-it-open62541-server-template
    mkdir build && cd build
    cmake ..
    make install

Start the a simple Server
==============================
Next, the server provided in this repository can be started:

.. code-block:: c

    cd swap-it-open62541-server-template/example_server/custom_information_model
    mkdir build && cd build
    cmake ..
    make
    ./example_server

Connect with UA-Expert
======================

To explore the server, connect with it with the UA-Expert Client. The server runs on "opc.tcp://localhost:4840"

