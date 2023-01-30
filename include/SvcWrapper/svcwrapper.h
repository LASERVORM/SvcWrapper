#ifndef SVCWRAPPER_H
#define SVCWRAPPER_H

#include <functional>

// SvcWrapper exitcodes
#define SVCWRAPPER_EXITCODE_OK 0
#define SVCWRAPPER_EXITCODE_INVALID_CONFIG 1
#define SVCWRAPPER_EXITCODE_CLI_SYNTAX_ERROR 2
#define SVCWRAPPER_EXITCODE_CLI_SCM_ERROR 3
#define SVCWRAPPER_EXITCODE_SVC_CTRL_DISPATCHER_FAILED 1000
#define SVCWRAPPER_EXITCODE_SVC_REG_CTRL_HANDLER_FAILED 1001

// SvcWrapperConfig format
struct SvcWrapperConfig {
    enum StartType {
        StartTypeAuto,
        StartTypeDemand,
        StartTypeDisabled
    };

    enum UserType {
        UserTypeSystem,
        UserTypeLocalService,
        UserTypeLocalNetwork,
        UserTypeCustom
    };

    const char* svcName {nullptr};
    const char* svcDisplayName {nullptr};
    const char* svcDescription {nullptr};
    const char* svcArgs {nullptr};
    StartType svcStartType {StartType::StartTypeDemand};
    UserType svcUserType {UserType::UserTypeLocalService};
    unsigned int checkStopInterval {3000};
    unsigned int shutdownTimeout {30000};

    std::function<int(int, char**)> svcCallbackMain {nullptr};
    std::function<void()> svcCallbackStop {nullptr}; //!< must be thread safe!
};

int SvcWrapper(int argc, char* argv[], const SvcWrapperConfig &svcConfig);

#endif // SVCWRAPPER_H
