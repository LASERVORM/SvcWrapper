# SvcWrapper

SvcWrapper is a library that allows to wrap any C/C++ application into a fully
functional Windows service.

## How does it work?

SvcWrapper adds a configurable wrapper around your applications `main`
function. This additionaly layer adds command line options to the executable,
allowing it to install and uninstall a Windows service. When Windows starts
your newly defined serivce, the wrapper code does all the Windows API work for
you, like interacting with the Service Control Manager (SCM). Once Windows
tells your service to start up, your original `main` function will be
executed in a separate thread. When the service is being shut down, a
callback function instructs your application to shutdown and wait for it to do
so, before continuing to clean up everything.

## How to use it

This very simplified example shows how to use SvcWrapper for your project:

```cpp
#include <SvcWrapper/svcwrapper.h>

// Your app's main function
int myapp_main(int argc, char* argv[])
{
    // Init your frameworks event loop or do whatever, but keep running...
    return 0;
}

// Your app's shutdown controller
void myapp_shutdown()
{
    // Tell your app to shutdown here, but return immediately!
    // (Don't wait for the shutdown to finish.)
    // Note: This function must be thread safe, as it'll be called from
    //       SvcWrapper's control thread!
}

// SvcWrapper main
int main(int argc, char* argv[])
{
    // Create a configuration for our service with the minimum settings:
    SvcWrapperConfig cfg;
    cfg.svcName = "MyAppService";
    cfg.svcDisplayName = "MyApp service";
    cfg.svcCallbackMain = myapp_main;
    cfg.svcCallbackStop = myapp_shutdown;

    // Hand over to SvcWrapper
    return SvcWrapper(argc, argv, cfg);
}
```

Thats all you need to run your application as customized Windows service.
The `example/` directory constains a fully functional example, implementing a
Windows service based on Qt framework.

Copyright (c) LASERVORM GmbH 2023