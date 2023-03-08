// Private implementation of the SvcWrapper library.
// Copyright (c) LASERVORM GmbH 2023
#include "svcwrapper_impl.h"
#include "svccli.h"

#include <cstring>
#include <cassert>
#include <iostream>

GlobalHandles *hSvc {nullptr};

using namespace std;

static void SvcLog(SvcLogLevel level, const char* msg)
{
    if (!hSvc)
        return;
    // Forward to log handler callback
    if (hSvc->cfg->svcLogCallback) {
        hSvc->cfg->svcLogCallback(level, msg);
        return;
    }
    // Fallback to stderr for critical messages
    if (level == Critical)
        cerr << msg << endl;
}

int SvcWrapperVerifyConfig(const SvcWrapperConfig &svcCfg)
{
    // Name and DisplayName may not be empty
    if (!svcCfg.svcName || strlen(svcCfg.svcName) > 255 ||
        !svcCfg.svcDisplayName || strlen(svcCfg.svcDisplayName) > 255)
        return SVCWRAPPER_EXITCODE_INVALID_CONFIG;

    // Description is optional, but should not exceed 255 chars
    if (svcCfg.svcDescription != nullptr && strlen(svcCfg.svcDescription) > 255)
        return SVCWRAPPER_EXITCODE_INVALID_CONFIG;

    // Check for required callbacks
    if (!svcCfg.svcCallbackMain || !svcCfg.svcCallbackStop)
        return SVCWRAPPER_EXITCODE_INVALID_CONFIG;

    // Config ok
    return SVCWRAPPER_EXITCODE_OK;
}

int SvcWrapper(int argc, char* argv[], const SvcWrapperConfig &svcConfig)
{
    assert(hSvc == nullptr);
    hSvc = new GlobalHandles;

    // Verify supplied SvcWrapper configuration
    int exitCode = SvcWrapperVerifyConfig(svcConfig);
    if (exitCode != 0) {
        SvcLog(Critical, "Invalid service configuration!");
        return exitCode;
    }

    // Store config pointer and startup args
    hSvc->cfg = &svcConfig;
    hSvc->argc = argc;
    hSvc->argv = argv;

    // Parse CLI args
    if (argc > 1) {
        SvcCli p(argc, argv, svcConfig);
        return p.run();
    }

    // Startup service
    exitCode =  SvcInit(svcConfig);
    delete hSvc;
    return exitCode;
}

int SvcInit(const SvcWrapperConfig &svcCfg)
{
    // Define ServiceTable entry for our service
    const SERVICE_TABLE_ENTRY ServiceTable[] = {
        // Entry for our service:
        {
            // Cast away const from service name for sucking Windows API
            const_cast<LPSTR>(svcCfg.svcName),
            // Specify service's main function
            (LPSERVICE_MAIN_FUNCTION) SvcMain},
        // End of service table definition
        {NULL, NULL}
    };

    // Pass ServiceTable to service control dispatcher
    if (!StartServiceCtrlDispatcher(ServiceTable)) {
        cout << "This application is a Windows Service executable!\n"
             << "Add help argument for supported CLI commands." << endl;
        return SVCWRAPPER_EXITCODE_SVC_CTRL_DISPATCHER_FAILED;
    }

    return 0;
}

void SvcMain()
{
    assert(hSvc->cfg != nullptr);

    // Register service control handler
    SvcLog(Debug, "Registering at service control manager...");
    hSvc->statusHandle = RegisterServiceCtrlHandler(hSvc->cfg->svcName, SvcCtrlHandler);
    if (hSvc->statusHandle == NULL) {
        SvcLog(Critical, "Failed to register service control handler!");
        return;
    }

    // Inform SCM we are starting...
    SvcLog(Info, "Starting service");
    ZeroMemory(&hSvc->status, sizeof(hSvc->status));
    hSvc->status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    hSvc->status.dwControlsAccepted = 0; // none
    hSvc->status.dwCurrentState = SERVICE_START_PENDING;
    hSvc->status.dwWin32ExitCode = NO_ERROR;
    hSvc->status.dwServiceSpecificExitCode = NO_ERROR;
    hSvc->status.dwCheckPoint = 0;
    if (!SetServiceStatus(hSvc->statusHandle, &hSvc->status)) {
        SvcLog(Warning, "Failed to set service status!");
    }

    // Create stop event to wait on later
    hSvc->stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hSvc->stopEvent == NULL) {
        hSvc->status.dwControlsAccepted = 0; // none
        hSvc->status.dwCurrentState = SERVICE_STOPPED;
        hSvc->status.dwWin32ExitCode = GetLastError();
        hSvc->status.dwCheckPoint = 1;
        if (SetServiceStatus(hSvc->statusHandle, &hSvc->status) == FALSE) {
            SvcLog(Warning, "Failed to set service status!");
        }
        return;
    }

    // Inform SCM we are started.
    hSvc->status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    hSvc->status.dwCurrentState = SERVICE_RUNNING;
    hSvc->status.dwWin32ExitCode = NO_ERROR;
    hSvc->status.dwCheckPoint = 0;
    if (SetServiceStatus(hSvc->statusHandle, &hSvc->status) == FALSE) {
        SvcLog(Warning, "Failed to set service status!");
    }

    // Start a thread for running our encapsulated application
    SvcLog(Debug, "Creating worker thread");
    HANDLE hWorkerThread = CreateThread(NULL, 0, SvcWorkerThread, NULL, 0, NULL);

    // Wait for stop event to be set
    SvcLog(Info, "Started worker thread");
    WaitForSingleObject(hSvc->stopEvent, INFINITE);
    CloseHandle(hSvc->stopEvent);

    // Wait for worker thread to finish
    WaitForSingleObject(hWorkerThread, hSvc->cfg->shutdownTimeout ?
                            hSvc->cfg->shutdownTimeout : INFINITE);
    CloseHandle(hWorkerThread);
    SvcLog(Info, "Service thread shutdown complete");

    // Tell SCM we stopped
    hSvc->status.dwControlsAccepted = 0; // none
    hSvc->status.dwCurrentState = SERVICE_STOPPED;
    if (hSvc->exitCode != 0) {
        hSvc->status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
        hSvc->status.dwServiceSpecificExitCode = hSvc->exitCode;
    } else {
        hSvc->status.dwWin32ExitCode = NO_ERROR;
    }
    hSvc->status.dwCheckPoint = 3;
    if (SetServiceStatus(hSvc->statusHandle, &hSvc->status) == FALSE) {
        SvcLog(Warning, "Failed to set service status!");
    }
}

void SvcCtrlHandler(DWORD CtrlCode)
{
    switch (CtrlCode) {
    case SERVICE_CONTROL_STOP:
        SvcLog(Debug, "Received service stop command");
        if (hSvc->status.dwCurrentState != SERVICE_RUNNING) {
            SvcLog(Warning, "Received stop command while service is inactive!");
            break;
        }

        // Execute serice stop callback
        SvcLog(Debug, "Executing service stop callback");
        hSvc->cfg->svcCallbackStop();

        // Tell SCM we're stopping
        hSvc->status.dwControlsAccepted = 0; // none
        hSvc->status.dwCurrentState = SERVICE_STOP_PENDING;
        hSvc->status.dwWin32ExitCode = NO_ERROR;
        hSvc->status.dwCheckPoint = 4;
        if (SetServiceStatus(hSvc->statusHandle, &hSvc->status) == FALSE) {
            SvcLog(Warning, "Failed to set service status!");
        }

        // Set stop event to let SvcMain resume
        SetEvent(hSvc->stopEvent);
        break;
    default:
        break;
    }
}

DWORD SvcWorkerThread(LPVOID)
{
    // Run service main procedure and store it's exit code
    hSvc->exitCode = hSvc->cfg->svcCallbackMain(hSvc->argc, hSvc->argv);
    char msg[60];
    sprintf(msg, "Worker thread has finished with exit code %d",
            hSvc->exitCode);
    SvcLog(Info, msg);
    return ERROR_SUCCESS;
}
