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
     * \param argc passed from main
     * \param argv passed from main
     * \param svcConfig Service configuration
     */
    explicit SvcCli(int argc, char *argv[], const SvcWrapperConfig& svcConfig);
    ~SvcCli();

    /*!
     * \brief Run CLI
     * \details Executes the CLI
     * \return CLI exit code
     */
    int run();

private:
    enum ScmAccess : DWORD {
        ScmAccessModify = SC_MANAGER_ALL_ACCESS
    };
    enum SvcAccess : DWORD {
        SvcAccessQuery = SERVICE_QUERY_STATUS,
        SvcAccessModify = SERVICE_ALL_ACCESS
    };

private:
    // CLI commands
    int help() const;
    int install();
    int uninstall();

    // Helpers
    int initSCM(ScmAccess accessLevel);
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
