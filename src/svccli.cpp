// Command line interface of SvcWrapper library.
// Copyright (c) LASERVORM GmbH 2023
#include "svccli.h"
#include "SvcWrapper/svcwrapper.h"

#include <windows.h>
#include <cassert>
#include <string>
#include <iostream>

#define ECODE_OK SVCWRAPPER_EXITCODE_OK
#define ECODE_SYNTAX SVCWRAPPER_EXITCODE_CLI_SYNTAX_ERROR
#define ECODE_SCM SVCWRAPPER_EXITCODE_CLI_SCM_ERROR

using std::cout, std::cerr, std::endl;

SvcCli::SvcCli(int argc, char *argv[], const SvcWrapperConfig& svcConfig)
    : m_argc(argc), m_svcCfg(svcConfig)
{
    // Convert char*'s to strings
    m_argv.resize(m_argc);
    for (int i = 0; i < m_argc; ++i) {
        m_argv[i] = std::string(argv[i]);
    }

    // Initialize own module name and path
    char binPath[MAX_PATH], binPathQuoted[MAX_PATH+2];
    assert(GetModuleFileName(NULL, binPath, MAX_PATH));
    sprintf(binPathQuoted, "\"%s\"", binPath);
    m_binaryPath = binPath;
    m_binaryPathQuoted = binPathQuoted;
    std::string::size_type const p(m_binaryPath.find_last_of("\\")+1);
    m_binaryName = m_binaryPath.substr(p, m_binaryPath.length()-p);

    // Populate service info
    m_svcName = std::string(m_svcCfg.svcName);
}

SvcCli::~SvcCli()
{
    // Clean up handles
    if (m_hSCM)
        CloseServiceHandle(m_hSCM);
}

int SvcCli::run()
{
    assert(m_argc > 1);

    // Evaluate command to execute
    if (m_argv[1] == "help") {
        return help();
    } else if (m_argv[1] == "install") {
        return install();
    } else if (m_argv[1] == "uninstall") {
        return uninstall();
    }

    cerr << "Unknown command!" << endl;
    cout << "Add argument \"help\" for help." << endl;
    return ECODE_SYNTAX;
}

int SvcCli::help() const
{
    cout << "Usage: " << m_binaryName << " [command]\n\n"
         << "Where [command] is one of:\n\n"
         << "  help         Displays this message.\n"
         << "  install      Installs the " << m_svcName << " service. (Needs admin privileges!)\n"
         << "  uninstall    Uninstalls the " << m_svcName << " service. (Needs admin privileges!)\n"
         << endl;
    return ECODE_OK;
}

int SvcCli::install()
{
    int code = ECODE_OK;
    SC_HANDLE hSvc;
    std::string binPath = m_binaryPathQuoted;
    std::string svcUser, svcPass;
    const char * svcUserPtr {NULL};
    const char * svcPassPtr {NULL};

    // Determine start user and password
    switch (m_svcCfg.svcUserType) {
    case SvcWrapperConfig::UserTypeCustom: {
        // Parse auth options
        bool hasError = (m_argc < 4 || m_argv[2] != "-u");
        if (!hasError) {
            svcUser = m_argv[3];
            svcUserPtr = svcUser.c_str();
            switch (m_argc) {
            case 4: // No password
                break;
            case 5: // -p
                // Ask for password
                hasError = (m_argv[4] != "-p");
                cout << "Enter password for user " << svcUser
                     << "(empty = no password): ";
                std::getline(std::cin, svcPass);
                break;
            case 6: // -p pass
                // Store password
                hasError = (m_argv[4] != "-p");
                svcPass = m_argv[5];
                break;
            default:
                hasError = true;
            }
        }
        if (hasError) {
            cout << "Usage: " << m_binaryName << " install -u username [-p [password]]\n" << endl;
            cout << "  -u user    Set username for the service. This option is mandatory!\n"
                 << "  -p [pass]  Set password for the service. This is optional.\n"
                    "             If the option is not specified, the service\n"
                    "             will have no (=empty) password. If specified\n"
                    "             without value, the password will be asked for.\n\n"
                 << endl;
            return ECODE_SYNTAX;
        }
        break;
    }
    case SvcWrapperConfig::UserTypeLocalService:
        svcUser = R"(NT AUTHORITY\LocalService)";
        svcUserPtr = svcUser.c_str();
        break;
    case SvcWrapperConfig::UserTypeLocalNetwork:
        svcUser = R"(NT AUTHORITY\NetworkService)";
        svcUserPtr = svcUser.c_str();
    case SvcWrapperConfig::UserTypeSystem: [[fallthrough]];
    default:
        break;
    }

    // Verify syntax for other user types
    if (m_svcCfg.svcUserType != SvcWrapperConfig::UserTypeCustom && m_argc > 2) {
        cout << "Usage: " << m_binaryName << " install" << endl;
        return ECODE_SYNTAX;
    }

    cout << "Installing " << m_svcName << " service..." << endl;

    // Check CLI params
    if (m_svcCfg.svcUserType == SvcWrapperConfig::UserTypeCustom) {
        // Parse custom user args
    } else {

    }

    // Open SCM
    code = initSCM(ScmAccessModify);

    // Verify service isn't installed already
    if (code == ECODE_OK) {
        hSvc = OpenService(m_hSCM, m_svcName.c_str(), SvcAccessQuery);
        if (hSvc != NULL) {
            CloseServiceHandle(hSvc);
            cerr << "Service " << m_svcName << " is already installed!\n"
                 << "Execute the uninstall command first, if you want to "
                    "reinstall it!" << endl;
            code = ECODE_SCM;
        }
    }

    // Any args need to be added to binary path?
    if (m_svcCfg.svcArgs != nullptr && strlen(m_svcCfg.svcArgs)) {
        binPath.append(" ").append(m_svcCfg.svcArgs);
    }

    // Install service
    if (code == ECODE_OK) {
        hSvc = CreateService(
                m_hSCM, // ServiceManager database
                m_svcCfg.svcName, // Service internal name
                m_svcCfg.svcDisplayName, // Service display name
                GENERIC_WRITE, // Service access rights [1]
                SERVICE_WIN32_OWN_PROCESS, // Service runs in own process
                convertStartType(m_svcCfg.svcStartType), // Service start type
                SERVICE_ERROR_NORMAL, // Service error handling
                binPath.c_str(), // Service binary path
                NULL, // Service does not belong to a group
                NULL, // No tar var pointer, as we don't belong to a group
                NULL, // No dependencies
                svcUserPtr, // Service username
                svcPassPtr); // Service password
        /*
         * [1] Service access rights should be at least GENERIC_READ, otherwise
         *     further modification calls - like setting the service description
         *     may fail.
         */
        if (hSvc == NULL) {
            cerr << "CreateService call failed: " << GetLastError() << endl;
            code = ECODE_SCM;
        }
    }

    // Set service description
    if (code == ECODE_OK && hSvc != NULL && m_svcCfg.svcDescription != nullptr) {
        SERVICE_DESCRIPTION sd;
        sd.lpDescription = const_cast<char*>(m_svcCfg.svcDescription);
        if (ChangeServiceConfig2(hSvc, SERVICE_CONFIG_DESCRIPTION, &sd) == FALSE) {
            cout << "WARNING: Failed to set service description text:"
                 << GetLastError() << endl;
        }
    }

    // Clean up
    if (hSvc != NULL) {
        CloseServiceHandle(hSvc);
    }

    cout << (code == ECODE_OK ?
                 "Service installation succeeded!" :
                 "Service installation failed!")
         << endl;
    return code;
}

int SvcCli::uninstall()
{
    cout << "Uninstalling " << m_svcName << " service..." << endl;

    SC_HANDLE hSvc;

    // Open SCM
    int code = initSCM(ScmAccessModify);

    // Verify service is installed
    if (code == ECODE_OK) {
        hSvc = OpenService(m_hSCM, m_svcName.c_str(), SvcAccessModify);
        if (hSvc == NULL) {
            CloseServiceHandle(hSvc);
            cerr << "Service " << m_svcName << " is not installed!\n" << endl;
            code = ECODE_SCM;
        }
    }

    // Verify service is stopped
    if (code == ECODE_OK) {
        SERVICE_STATUS_PROCESS svcState;
        DWORD dwBytesNeeded;
        int result = QueryServiceStatusEx(
            hSvc, SC_STATUS_PROCESS_INFO,
            reinterpret_cast<LPBYTE>(&svcState),
            sizeof(SERVICE_STATUS_PROCESS),
            &dwBytesNeeded);
        if (!result) {
            cerr << "Failed to query status of service " << m_svcName << "!" << endl;
            CloseServiceHandle(hSvc);
            code = ECODE_SCM;
        } else if (svcState.dwCurrentState != SERVICE_STOPPED) {
            cerr << "Service " << m_svcName << " is not stopped!" << endl;
            CloseServiceHandle(hSvc);
            code = ECODE_SCM;
        }
    }

    // Uninstall service
    if (code == ECODE_OK) {
        if (DeleteService(hSvc) == FALSE) {
            cerr << "Failed to delete service " << m_svcName << ": "
                 << GetLastError() << endl;
            CloseServiceHandle(hSvc);
            code = ECODE_SCM;
        }
    }

    cout << (code == ECODE_OK ?
                 "Service uninstallation succeeded!" :
                 "Service uninstallation failed!")
         << endl;
    return code;
}

int SvcCli::initSCM(ScmAccess accessLevel)
{
    assert(m_hSCM == NULL);
    m_hSCM = OpenSCManager(
                NULL, // Local computer
                NULL, // ServicesActive database
                static_cast<DWORD>(accessLevel)); // Full access rights
    if (m_hSCM == NULL) {
        cerr << "Failed to access Service Control Manager!"
             << (accessLevel == ScmAccessModify ?
                     " Do you have Admin rights?" : "")
             << " (Error code: " << GetLastError() << ")" << endl;
        return ECODE_SCM;
    }
    return ECODE_OK;
}

constexpr DWORD SvcCli::convertStartType(SvcWrapperConfig::StartType startType) const
{
    switch (startType) {
    case SvcWrapperConfig::StartTypeAuto: return SERVICE_AUTO_START;
    case SvcWrapperConfig::StartTypeDemand: return SERVICE_DEMAND_START;
    case SvcWrapperConfig::StartTypeDisabled: [[fallthrough]];
    default: return SERVICE_DISABLED;
    }
}

