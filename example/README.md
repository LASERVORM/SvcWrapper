# SvcWrapper example

This directory contains an example application for demonstrating how to use
the SvcWrapper library. Enable CMake option `SVCWRAPPER_EXAMPLE` to build the
example.

**Note:** The example requires Qt framework v5.x or v6.x!

## Components

The example contains:

* **LibEchoServer** (echoserver.\*, echoserver_main.\*)
  
  Shared library containing the logic of our example application. The main
  function is named `echoserver_main`, as we need to preserve the real `main`
  function for SvcWrapper.

  The application starts a TCP server listening on 127.0.0.1:12345 that echoes
  all received data back to the client.

* **EchoServer** (main.cpp)
  
  Standalone application that just invokes the main function of LibEchoServer
  to demonstrate our application works when started from shell.

* **EchoServerService** (service.cpp)
  
  Executable that utilizes SvcWrapper to run LibEchoServer as a Windows Service
  and offer a command line interface for installing and uninstalling the
  service. The registered service is named "EchoServer example".

## Usage

In case you don't have the Qt libraries accessible in your $PATH, you may need
to copy QtXCore.dll and QtXNetwork.dll (and their dependencies) to the binary
directory. You should verify your example runs by executing **EchoServer**
first.

Install the echo server service by executing the following command in a shell
with administrative permissions:

```
EchoServerService.exe install
```

Now you can start the service and use your favourite TCP client (Windows
Telnet, PuTTY, ...) to connect to 127.0.0.1:12345. Type some messages and see
how the EchoServer responst.

To clean up everything, stop the service and invoke the EchoServerService
executable again with the "uninstall" argument.
