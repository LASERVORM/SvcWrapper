/* Public header of the SvcWrapper library.
 *
 * SvcWrapper is a library that allows to wrap any C/C++ application into a
 * fully functional Windows service.
 * Source code and readme can be found at:
 * https://github.com/LASERVORM/SvcWrapper
 *
 * Copyright (c) LASERVORM GmbH 2023
 */
#ifndef SVCWRAPPER_H
#define SVCWRAPPER_H

#include <functional>

// === SvcWrapper exitcodes ====================================================
// The following exit codes may be returned by SvcWrapper executables:

// Everything ok
#define SVCWRAPPER_EXITCODE_OK 0

// The supplied SvcWrapperConfig object was invalid.
#define SVCWRAPPER_EXITCODE_INVALID_CONFIG 1

// The CLI encountered syntax error(s) in argv
#define SVCWRAPPER_EXITCODE_CLI_SYNTAX_ERROR 2

// There was a error during communication with Windows Service Control Manager
// druing a CLI operation. Details may be found in console output.
#define SVCWRAPPER_EXITCODE_CLI_SCM_ERROR 3

// The service process couldn't be attached to the Windows Service controller.
// This happens, when a service executable is started manually.
#define SVCWRAPPER_EXITCODE_SVC_CTRL_DISPATCHER_FAILED 1000

// The process failed to register it's control handler to the Windows Service
// Control Manager.
#define SVCWRAPPER_EXITCODE_SVC_REG_CTRL_HANDLER_FAILED 1001

/*!
 * \brief SvcWrapper configuration
 * \details The SvcWrapperConfig struct contains the configuration for a
 * SvcWrapper service. It should be created within `main` and passed to
 * the SvcWrapper function.
 */
struct SvcWrapperConfig {
    /*!
     * \brief Service start types
     * \details The StartType enum defines possible start types for the service.
     */
    enum StartType {
        StartTypeAuto,      //!< Start when the OS boots.
        StartTypeDemand,    //!< Start on demand (manually)
        StartTypeDisabled   //!< Don't start at all
    };

    /*!
     * \brief Possible service user types
     * \details The UserType enum defines types of users the service can run as.
     */
    enum UserType {
        UserTypeSystem,         //!< Run as `SYSTEM` user (dangerous)
        UserTypeLocalService,   //!< Run as `NT AUTHORITY\LocalService`
        UserTypeLocalNetwork,   //!< Run as `NT AUTHORITY\NetworkService`
        UserTypeCustom          //!< Run as custom user
    };

    /*!
     * \brief Service internal name
     * \details Defines the internal name of the service, this should not be
     * localized and shouldn't contain whitespaces. The maximum length is
     * 255 characters. This value is always mandatory!
     */
    const char* svcName {nullptr};

    /*!
     * \brief Service display name
     * \details Defines the display name of the service. This can contain any
     * pritable character and is allowed to be localized. This filed is always
     * mandatory and should not exceed 255 characters.
     */
    const char* svcDisplayName {nullptr};

    /*!
     * \brief Service description
     * \details Defines a optional description string to be displayed in the
     * Windows service manager. This string may be localized and should not
     * exceed 255 characters. (In theory longer strings would be possible,
     * but to be honest: no one ever reads this.)
     */
    const char* svcDescription {nullptr};

    /*!
     * \brief Startup args
     * \details Optional string for startup args that will be passed to your
     * `main` function when the service starts up. Just think of this as the
     * string containing all whitespace separated arguments as you would type
     * it into command prompt.
     */
    const char* svcArgs {nullptr};

    /*!
     * \brief Service start type
     * \details Defines the default start type of the service. The default
     * value is StartType::StartTypeDemand.
     */
    StartType svcStartType {StartType::StartTypeDemand};

    /*!
     * \brief Service user type
     * \details Defines the type of user the service should logon as.
     * See UserType enum for possible values. The default value is
     * UserType::UserTypeLocalService.
     *
     * \note If UserType::UserTypeCustom is set, this adds CLI options to the
     * `install` command of the service executable for specifying the user
     * accout name and password.
     */
    UserType svcUserType {UserType::UserTypeLocalService};

    /*!
     * \brief Service shutdown timeout [ms]
     * \details Specifies the timeout in milliseconds for the wrapped
     * application to shutdown safely after the shutdown callback was invoked,
     * before it's thread will be terminated. If this value is 0, the wrapper
     * will wait until the applications main function returns.
     * The default value is 30000 (30s).
     */
    unsigned int shutdownTimeout {30000};

    /*!
     * \brief Application main callback
     * \details Callback function to your applications `main` function, this
     * is mandatory. This function will be invoked in a new thread when the
     * service starts, it will be passed argc and argv from the service
     * controller and is expected to return an exit code.
     * \note This function needs to block as long as the service is running, so
     * this is the right place to execute your frameworks/own event loop.
     */
    std::function<int(int, char**)> svcCallbackMain {nullptr};

    /*!
     * \brief Application shutdown callback
     * \details Callback to your applications shutdown routine, this is
     * mandatory. This function will be called, when the Windows Service
     * Control Manager wants the service to stop. It should instruct your
     * application to initiate shutdown but return immediately.
     * \note This function will be called from SvcWrappers thread, so it
     * must be thread safe!
     */
    std::function<void()> svcCallbackStop {nullptr}; //!< must be thread safe!
};

/*!
 * \brief SvcWrapper main function
 * \details This is all the magic it takes to wrap your application into a
 * windows service. Just call it in `main` while returning it's return value
 * as exit code. It'll handle the rest for your!
 * \param argc pass from your main
 * \param argv pass from your main
 * \param svcConfig Configuration for your service
 * \return application exit code
 * \sa SvcWrapperConfig
 */
int SvcWrapper(int argc, char* argv[], const SvcWrapperConfig &svcConfig);

#endif // SVCWRAPPER_H
