#include <SvcWrapper/svcwrapper.h>
#include <chrono>
#include <thread>
using namespace std;

static bool running {true};

int svc_main(int, char**)
{
    while (running) {
        this_thread::sleep_for(chrono::seconds(1));
    }
    return 0;
}

void svc_stop()
{
    running = false;
}

int main(int argc, char* argv[])
{
    SvcWrapperConfig cfg;
    cfg.svcName = "SvcWrapperTestService";
    cfg.svcDisplayName = "SvcWrapper conan test service";
    cfg.svcCallbackMain = svc_main;
    cfg.svcCallbackStop = svc_stop;
    return SvcWrapper(argc, argv, cfg);
}
