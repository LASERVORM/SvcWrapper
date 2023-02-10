// Command line interface of SvcWrapper library.
// Copyright (c) LASERVORM GmbH 2023
#ifndef SVCCLI_H
#define SVCCLI_H

#include <string>
#include <vector>
#include <windows.h>

#include "SvcWrapper/svcwrapper.h"

/*!
 * \brief SvcWrapper CLI
 * \details Implements the SvcWrapper command line interface which handles
 * installation and removal of the service.
 */
class SvcCli
{
public:
    /*!
     * \brief Construct CLI handler instance
     * \details This only initializes the member variables.
     * \param argc passed from main
     * \param argv passed from main
     * \param svcConfig Service configuration
     */
    explicit SvcCli(int argc, char *argv[], const SvcWrapperConfig& svcConfig);
    ~SvcCli();

    /*!
     * \brief Run CLI handler
     * \details Handles the command line interface by parsing the requested
     * CLI command and executing one of the private functions below.
     * \return CLI exit code
     */
    int run();

private:
    /*!
     * \brief Service control manager access level
     * \details The ScmAccess enum defines permission levels used to access
     * the windows service control manager (SCM).
     * \note This may be extended in future with e.g. read only access for
     * querying service status.
     */
    enum ScmAccess : DWORD {
        //! \brief Modify service configuration
        ScmAccessModify = SC_MANAGER_ALL_ACCESS
    };

    /*!
     * \brief Service configuration access level
     * \details The SvcAccess enum defines permission levels used to access
     * the configuration of a Windows service.
     */
    enum SvcAccess : DWORD {
        //! \brief Read service configuration and status
        SvcAccessQuery = SERVICE_QUERY_STATUS,
        //! \brief Modify service configuration
        SvcAccessModify = SERVICE_ALL_ACCESS
    };

private:
    // === CLI commands ========================================================

    /*!
     * \brief Print help
     * \details Prints command line interface help to stdout and exits.
     * \return Exit code 0
     */
    int help() const;

    /*!
     * \brief Install service
     * \details Register the service with the Windows Service Control Manager
     * (SCM). This function will print addition information to stdout and return
     * a different exit code in following cases:
     * - command syntax error (will also print help)
     * - service is already installed
     * - user has no permissions to access SCM
     * - service registration at SCM failed (prints SCM error code)
     * \return Exit code (0 on success)
     */
    int install();

    /*!
     * \brief Uninstall service
     * \details Removes the service from Windows Service Control Manager (SCM).
     * This function will print additional information to stdout and return a
     * different exit code in following cases:
     * - user has no permissions to access SCM
     * - service is not installed
     * - service is still running
     * - uninstallation at SCM failed (prints SCM error code)
     * \return Exit code (0 on success)
     */
    int uninstall();

    // === Helpers =============================================================
    /*!
     * \brief Init SCM access
     * \details Initializes access to the Service Control Manager (SCM) with
     * desired access level.
     * \param accessLevel desired access level
     * \return Result code (0 in success)
     */
    int initSCM(ScmAccess accessLevel);

    /*!
     * \brief Convert service start type
     * \details Translates configured service start type to the Windows API
     * specific DWORD value.
     * \param startType service start type
     * \return Windows service start type value
     */
    constexpr DWORD convertStartType(SvcWrapperConfig::StartType startType) const;

private:
    // Initial params
    const int    m_argc;
    std::vector<std::string> m_argv;
    const SvcWrapperConfig& m_svcCfg;

    // Executable information
    std::string m_binaryName;
    std::string m_binaryPath;
    std::string m_binaryPathQuoted;

    // Service information
    std::string m_svcName;

    // SCM handles
    SC_HANDLE   m_hSCM {NULL};
};

#endif // SVCCLI_H
