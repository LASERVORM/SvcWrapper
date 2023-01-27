#ifndef SVCWRAPPER_IMPL_H
#define SVCWRAPPER_IMPL_H

#include "SvcWrapper/svcwrapper.h"
#include <windows.h>

//! \todo logging

// Global handles
struct {
    const SvcWrapperConfig* cfg {nullptr};
    SERVICE_STATUS status;
    SERVICE_STATUS_HANDLE statusHandle {NULL};
    HANDLE stopEvent {INVALID_HANDLE_VALUE};

    int argc {0};
    char** argv {nullptr};
    int exitCode {SVCWRAPPER_EXITCODE_OK};
} hSvc;

int SvcWrapperVerifyConfig(const SvcWrapperConfig& svcCfg);

int SvcInit(const SvcWrapperConfig& svcCfg);
void WINAPI SvcMain();
void WINAPI SvcCtrlHandler(DWORD CtrlCode);
DWORD SvcWorkerThread(LPVOID);
void SvcStopWithLastError();

// CLI
int SvcParseCLI();

int SvcInstall();
int SvcUninstall();

#endif // SVCWRAPPER_IMPL_H
