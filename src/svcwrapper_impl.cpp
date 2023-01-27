#include "svcwrapper_impl.h"
#include <cstring>
#include <cassert>
#include <iostream>

using namespace std;

int SvcWrapperVerifyConfig(const SvcWrapperConfig &svcCfg)
{
    // Name and DisplayName may not be empty
    if (!svcCfg.svcName || strlen(svcCfg.svcName) > 255 ||
        !svcCfg.svcDisplayName || strlen(svcCfg.svcDisplayName) > 255)
        return SVCWRAPPER_EXITCODE_INVALID_CONFIG;

    // Check for required callbacks
    if (!svcCfg.svcCallbackMain || !svcCfg.svcCallbackStop)
        return SVCWRAPPER_EXITCODE_INVALID_CONFIG;

    // Config ok
    return SVCWRAPPER_EXITCODE_OK;
}

int SvcWrapper(int argc, char* argv[], const SvcWrapperConfig &svcConfig)
{
    // Verify supplied SvcWrapper configuration
    int exitCode = SvcWrapperVerifyConfig(svcConfig);
    if (exitCode != 0) {
        cerr << "Invalid service configuration!" << endl;
        return exitCode;
    }

    // Store config pointer and startup args
    hSvc.cfg = &svcConfig;
    hSvc.argc = argc;
    hSvc.argv = argv;

    // Parse CLI args
    if (argc > 1) {
        return SvcParseCLI();
    }

    // Startup service
    return SvcInit(svcConfig);
}

int SvcInit(const SvcWrapperConfig &svcCfg)
{
    //! \todo check if argc, argv can be removed

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
             << "Add --help argument for supported CLI commands." << endl;
        return SVCWRAPPER_EXITCODE_SVC_CTRL_DISPATCHER_FAILED;
    }

    return 0;
}

void SvcMain()
{
    assert(hSvc.cfg != nullptr);

    // Register service control handler
    hSvc.statusHandle = RegisterServiceCtrlHandler(hSvc.cfg->svcName, SvcCtrlHandler);
    if (hSvc.statusHandle == NULL) {
        //! \todo log failure?
        return;
    }

    // Inform SCM we are starting...
    ZeroMemory(&hSvc.status, sizeof(hSvc.status));
    hSvc.status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    hSvc.status.dwControlsAccepted = 0; // none
    hSvc.status.dwCurrentState = SERVICE_START_PENDING;
    hSvc.status.dwWin32ExitCode = NO_ERROR;
    hSvc.status.dwServiceSpecificExitCode = NO_ERROR;
    hSvc.status.dwCheckPoint = 0;
    if (!SetServiceStatus(hSvc.statusHandle, &hSvc.status)) {
        //! \todo log this problem somehow
    }

    // Create stop event to wait on later
    hSvc.stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hSvc.stopEvent == NULL) {
        SvcStopWithLastError();
        return;
    }

    // Inform SCM we are started.
    hSvc.status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    hSvc.status.dwCurrentState = SERVICE_RUNNING;
    hSvc.status.dwWin32ExitCode = NO_ERROR;
    hSvc.status.dwCheckPoint = 0;
    if (SetServiceStatus(hSvc.statusHandle, &hSvc.status) == FALSE) {
        //! \todo log this problem
    }

    // Start a thread for running our encapsulated application
    HANDLE hWorkerThread = CreateThread(NULL, 0, SvcWorkerThread, NULL, 0, NULL);

    // Wait for stop event to be set
    WaitForSingleObject(hSvc.stopEvent, INFINITE);
    CloseHandle(hSvc.stopEvent);

    // Wait for worker thread to finish
    WaitForSingleObject(hWorkerThread, hSvc.cfg->shutdownTimeout ?
                            hSvc.cfg->shutdownTimeout : INFINITE);
    CloseHandle(hWorkerThread);

    // Tell SCM we stopped
    hSvc.status.dwControlsAccepted = 0; // none
    hSvc.status.dwCurrentState = SERVICE_STOPPED;
    if (hSvc.exitCode) {
        hSvc.status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
        hSvc.status.dwServiceSpecificExitCode = hSvc.exitCode;
    } else {
        hSvc.status.dwWin32ExitCode = NO_ERROR;
    }
    hSvc.status.dwCheckPoint = 3;
    if (SetServiceStatus(hSvc.statusHandle, &hSvc.status) == FALSE) {
        //! \todo log failure
    }
}

void SvcStopWithLastError()
{
    hSvc.status.dwControlsAccepted = 0; // none
    hSvc.status.dwCurrentState = SERVICE_STOPPED;
    hSvc.status.dwWin32ExitCode = GetLastError();
    hSvc.status.dwCheckPoint = 1;
    if (SetServiceStatus(hSvc.statusHandle, &hSvc.status) == FALSE) {
        //! \todo log service status failed
    }
}

void SvcCtrlHandler(DWORD CtrlCode)
{
    switch (CtrlCode) {
    case SERVICE_CONTROL_STOP:
        if (hSvc.status.dwCurrentState != SERVICE_RUNNING) {
            //! \todo log
            break;
        }

        // Execute serice stop callback
        hSvc.cfg->svcCallbackStop();

        // Tell SCM we're stopping
        hSvc.status.dwControlsAccepted = 0; // none
        hSvc.status.dwCurrentState = SERVICE_STOP_PENDING;
        hSvc.status.dwWin32ExitCode = NO_ERROR;
        hSvc.status.dwCheckPoint = 4;
        if (SetServiceStatus(hSvc.statusHandle, &hSvc.status) == FALSE) {
            //! \todo log error
        }

        // Set stop event to let SvcMain resume
        SetEvent(hSvc.stopEvent);
        break;
    default:
        break;
    }
}

DWORD SvcWorkerThread(LPVOID)
{
    // Run service main procedure and store it's exit code
    hSvc.exitCode = hSvc.cfg->svcCallbackMain(hSvc.argc, hSvc.argv);
    return ERROR_SUCCESS;
}

int SvcParseCLI()
{
    assert(hSvc.argc > 1);

    // Install command
    if (strcmp(hSvc.argv[1], "install") == 0) {
        return SvcInstall();
    } else if (strcmp(hSvc.argv[1], "uninstall") == 0) {
        return SvcUninstall();
    }
}

int SvcInstall()
{
    if (hSvc.argc > 2) {
        cout << "Usage: " << hSvc.argv[0] << " install" << endl;
        return SVCWRAPPER_EXITCODE_CLI_ERROR;
    }

    cout << "Installing service " << hSvc.cfg->svcName << "..." << endl;

    // Get executable path
    char binPathUnquoted[MAX_PATH];
    if (!GetModuleFileName(NULL, binPathUnquoted, MAX_PATH)) {
        cerr << "GetModuleFileName call failed: " << GetLastError() << endl;
        cout << "Service install failed!";
        return SVCWRAPPER_EXITCODE_CLI_ERROR;
    }

    // Path must be quoted, in case it contains spaces...
    char binPath[MAX_PATH+2];
    sprintf(binPath, "\"%s\"", binPathUnquoted);

    // Get handle to SCM database
    SC_HANDLE svcMgr = OpenSCManager(
                NULL, // Local computer
                NULL, // ServicesActive database
                SC_MANAGER_ALL_ACCESS); // Full access rights
    if (svcMgr == NULL) {
        cerr << "OpenSCManager call failed. Do you have Admin rights?" << endl;
        cerr << "Error info: " << GetLastError() << endl;
        cout << "Service install failed!";
        return SVCWRAPPER_EXITCODE_CLI_ERROR;
    }

    // Check services already exists
    SC_HANDLE svcDef = OpenService(svcMgr, hSvc.cfg->svcName, SERVICE_QUERY_STATUS);
    if (svcDef != NULL) {
        cerr << "Service is already installed!" << endl;
        cout << "Service install failed!";
        CloseServiceHandle(svcDef);
        CloseServiceHandle(svcMgr);
        return SVCWRAPPER_EXITCODE_CLI_ERROR;
    }

    // Create the service
    svcDef = CreateService(
                svcMgr, // ServiceManager database
                hSvc.cfg->svcName, // Service internal name
                hSvc.cfg->svcDisplayName, // Service display name
                SERVICE_ALL_ACCESS, //!< \todo configurable
                SERVICE_WIN32_OWN_PROCESS, // Service runs in own process
                SERVICE_DEMAND_START, //! \todo configurable
                SERVICE_ERROR_NORMAL, //!< \todo configurable
                binPath, // Service binary path
                NULL, //! \todo check loadordergroup
                NULL, //! < \todo check tag identifier
                NULL, //!< \todo check dependencies
                NULL, //! < \todo check run account
                NULL); //! < \todo check run password
    if (svcDef == NULL) {
        cerr << "CreateService call failed: " << GetLastError() << endl;
        cout << "Service install failed!";
        CloseServiceHandle(svcMgr);
        return SVCWRAPPER_EXITCODE_CLI_ERROR;
    }

    // Service installed successfully
    CloseServiceHandle(svcDef);
    CloseServiceHandle(svcMgr);
    cout << "Success!" << endl;
    return SVCWRAPPER_EXITCODE_OK;
}

int SvcUninstall()
{
    if (hSvc.argc > 2) {
        cout << "Usage: " << hSvc.argv[0] << " uninstall" << endl;
        return SVCWRAPPER_EXITCODE_CLI_ERROR;
    }

    cout << "Uninstalling service " << hSvc.cfg->svcName << "..." << endl;

    // Get handle to SCM database
    SC_HANDLE svcMgr = OpenSCManager(
                NULL, // Local computer
                NULL, // ServicesActive database
                SC_MANAGER_ALL_ACCESS); // Full access rights
    if (svcMgr == NULL) {
        cerr << "OpenSCManager call failed. Do you have Admin rights?" << endl;
        cerr << "Error info: " << GetLastError() << endl;
        cout << "Service uninstall failed!";
        return SVCWRAPPER_EXITCODE_CLI_ERROR;
    }

    // Check service exists
    SC_HANDLE svcDef = OpenService(svcMgr, hSvc.cfg->svcName, SERVICE_ALL_ACCESS);
    if (svcDef == NULL) {
        cerr << "Service " << hSvc.cfg->svcName << " is not installed!" << endl;
        cout << "Service uninstall failed!";
        CloseServiceHandle(svcDef);
        CloseServiceHandle(svcMgr);
        return SVCWRAPPER_EXITCODE_CLI_ERROR;
    }

    // Check service is stopped
    SERVICE_STATUS_PROCESS svcState;
    DWORD dwBytesNeeded;
    int result = QueryServiceStatusEx(svcDef, SC_STATUS_PROCESS_INFO,
        reinterpret_cast<LPBYTE>(&svcState), sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded);
    if (!result) {
        cerr << "Failed to query status of service " << hSvc.cfg->svcName << "!" << endl;
        cout << "Service uninstall failed!";
        CloseServiceHandle(svcDef);
        CloseServiceHandle(svcMgr);
        return SVCWRAPPER_EXITCODE_CLI_ERROR;
    }
    if (svcState.dwCurrentState != SERVICE_STOPPED) {
        cerr << "Service " << hSvc.cfg->svcName << " is not stopped!" << endl;
        cout << "Service uninstall failed!";
        CloseServiceHandle(svcDef);
        CloseServiceHandle(svcMgr);
        return SVCWRAPPER_EXITCODE_CLI_ERROR;
    }

    // Delete service
    if (DeleteService(svcDef) == FALSE) {
        cerr << "Failed to delete service " << hSvc.cfg->svcName << ": "
             << GetLastError() << endl;
        cout << "Service uninstall failed!";
        CloseServiceHandle(svcDef);
        CloseServiceHandle(svcMgr);
        return SVCWRAPPER_EXITCODE_CLI_ERROR;
    }

    // Service delete success
    cout << "Success!" << endl;
    CloseServiceHandle(svcDef);
    CloseServiceHandle(svcMgr);
    return SVCWRAPPER_EXITCODE_OK;
}
