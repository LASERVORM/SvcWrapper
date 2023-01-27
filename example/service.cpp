#include <SvcWrapper/svcwrapper.h>
#include <EchoServer/echoserver_main.h>

int main(int argc, char* argv[])
{
    SvcWrapperConfig cfg;
    cfg.svcName = "EchoServer";
    cfg.svcDisplayName = "EchoServer example";
    cfg.svcCallbackMain = echoserver_main;
    cfg.svcCallbackStop = shutdown;

    return SvcWrapper(argc, argv, cfg);
}
