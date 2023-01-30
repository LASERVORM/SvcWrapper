// SvcWrapper demo service example.
// Copyright (c) LASERVORM GmbH 2023
#include <SvcWrapper/svcwrapper.h>
#include <EchoServer/echoserver_main.h>

// This version of the example demonstrates wrapping our little application
// into a fully functional Windows service using SvcWrapper.
int main(int argc, char* argv[])
{
    // Create a configuration for our service
    SvcWrapperConfig cfg;
    cfg.svcName = "EchoServer";
    cfg.svcDisplayName = "EchoServer example";
    cfg.svcDescription = "Just a demonstration of the SvcWrapper library!";
    cfg.svcCallbackMain = echoserver_main;
    cfg.svcCallbackStop = shutdown;

    // Execute the service wrapper
    return SvcWrapper(argc, argv, cfg);
}
