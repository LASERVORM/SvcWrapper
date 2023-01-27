#ifndef SVCWRAPPER_H
#define SVCWRAPPER_H

#include <functional>

// SvcWrapper exitcodes
#define SVCWRAPPER_EXITCODE_OK 0
#define SVCWRAPPER_EXITCODE_INVALID_CONFIG 1
#define SVCWRAPPER_EXITCODE_CLI_ERROR 2
#define SVCWRAPPER_EXITCODE_SVC_CTRL_DISPATCHER_FAILED 1000
#define SVCWRAPPER_EXITCODE_SVC_REG_CTRL_HANDLER_FAILED 1001

// SvcWrapperConfig format
struct SvcWrapperConfig {
    const char* svcName {nullptr};
    const char* svcDisplayName {nullptr};
    unsigned int checkStopInterval {3000};
    unsigned int shutdownTimeout {0};

    std::function<int(int, char**)> svcCallbackMain {nullptr};
    std::function<void()> svcCallbackStop {nullptr}; //!< must be thread safe!
};

int SvcWrapper(int argc, char* argv[], const SvcWrapperConfig &svcConfig);

#endif // SVCWRAPPER_H
