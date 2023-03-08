// Private implementation of the SvcWrapper library.
// Copyright (c) LASERVORM GmbH 2023
#ifndef SVCWRAPPER_IMPL_H
#define SVCWRAPPER_IMPL_H

#include "SvcWrapper/svcwrapper.h"
#include <windows.h>

// Global handles required for service operation
struct GlobalHandles {
    // Service configuration
    const SvcWrapperConfig* cfg {nullptr};

    // Current status of the service
    SERVICE_STATUS status;

    // Service manager status handle
    SERVICE_STATUS_HANDLE statusHandle {NULL};

    // Service stop event
    HANDLE stopEvent {INVALID_HANDLE_VALUE};

    // Original argc & argv
    int argc {0};
    char** argv {nullptr};

    // Wrapped application exit code
    int exitCode {SVCWRAPPER_EXITCODE_OK};
};

/*!
 * \brief Verify service configuration
 * \details Verifies the service configuration settings.
 * \param svcCfg Service configuration
 * \return Exit code 0 if configuration is okay, exit code != 0 otherwise.
 */
int SvcWrapperVerifyConfig(const SvcWrapperConfig& svcCfg);

/*!
 * \brief Init service
 * \details Initializes the service be registering with and dispatching control
 * to Windows Service Control Manager.
 * \param svcCfg Service configuration
 * \return Exit code
 */
int SvcInit(const SvcWrapperConfig& svcCfg);

/*!
 * \brief Service main function
 * \details Will be invoked through Windows Service Control Manager and controls
 * the lifecycle of the service.
 */
void WINAPI SvcMain();

/*!
 * \brief Servicce control handler
 * \details Processes control commands from Windows Service Control Manager.
 * \param CtrlCode Control code from SCM
 */
void WINAPI SvcCtrlHandler(DWORD CtrlCode);

/*!
 * \brief Service worker thread
 * \details Runs the application wrapped by SvcWrapper in it's own thread.
 * \return Always exit code 0. (Return value unused but required by Windows
 * thread API.)
 */
DWORD SvcWorkerThread(LPVOID);

#endif // SVCWRAPPER_IMPL_H
