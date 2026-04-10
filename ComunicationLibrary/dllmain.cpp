#include "pch.h"
#include <winioctl.h> // Add this include to define FILE_DEVICE_UNKNOWN
#include <windows.h>  // Add this include for SCM functions
#include <iostream>

struct TCPData {
    UINT32 localAddress;
    UINT16 localPort;
    UINT32 remoteAddress;
    UINT16 remotePort;
    // Add more fields as needed
};

#define IOCTL_GET_TCP_DATA CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

EXTERN_C __declspec(dllexport) BOOL GetTCPData(PVOID data, ULONG* dataSize);
EXTERN_C __declspec(dllexport) BOOL CreateTcpInterceptionService();
EXTERN_C __declspec(dllexport) BOOL DeleteTcpInterceptionService();

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
		DeleteTcpInterceptionService();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}

BOOL CreateTcpInterceptionService() {
    SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
    if (hSCManager == nullptr) {
		std::cout << "OpenSCManager failed" << std::endl;
        std::cout << GetLastError() << std::endl;
        return FALSE;
    }

    SC_HANDLE hService = CreateServiceA(
        hSCManager,
        "TcpInterception", // Service name
        "Tcp Interception Service", // Display name
        SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER,
        SERVICE_AUTO_START,  // Change to SERVICE_AUTO_START for automatic start
        SERVICE_ERROR_NORMAL,
        "C:\\Users\\saidhan\\Desktop\\deployedUI\\TcpInterception.sys",
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    );


	

    if (hService == nullptr) {
        CloseServiceHandle(hSCManager);
		std::cout << "CreateService failed" << std::endl;
        std::cout << GetLastError() << std::endl;
        return FALSE;
    }
    std::cout << "Service created\n" << std::endl;

    // Start the service immediately
    if (!StartService(hService, 0, nullptr)) {
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
		std::cout << "StartService failed" << std::endl;
		std::cout << GetLastError() << std::endl;
        return FALSE;
    }
	std::cout << "Service started\n" << std::endl;

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    return TRUE;
}



BOOL DeleteTcpInterceptionService() {
    SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (hSCManager == nullptr) {
		std::cout << "CloseSCManager failed" << std::endl;
        std::cout << GetLastError() << std::endl;
        return FALSE;
    }

    SC_HANDLE hService = OpenService(hSCManager, L"TcpInterception", SERVICE_STOP | DELETE);
    if (hService == nullptr) {
        CloseServiceHandle(hSCManager);
		std::cout << "CloseService failed" << std::endl;
        std::cout << GetLastError() << std::endl;
        return FALSE;
    }

    SERVICE_STATUS status;
    if (ControlService(hService, SERVICE_CONTROL_STOP, &status)) {
        // Wait for the service to stop
        Sleep(1000);
        while (QueryServiceStatus(hService, &status) && status.dwCurrentState != SERVICE_STOPPED) {
            Sleep(1000);
        }
    }

    BOOL result = DeleteService(hService);

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    return result;
}

BOOL GetTCPData(PVOID data, ULONG* dataSize) {
    HANDLE hDevice = CreateFile(L"\\\\.\\TcpInterception", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hDevice == INVALID_HANDLE_VALUE) {
		std::cout << "CreateFile failed" << std::endl;
        std::cout << GetLastError() << std::endl;
        return FALSE;
    }

    DWORD bytesReturned;
    BOOL result = DeviceIoControl(hDevice, IOCTL_GET_TCP_DATA, nullptr, 0, data, *dataSize, &bytesReturned, nullptr);
    if (bytesReturned) {
		std::cout << "Bytes returned: " << bytesReturned << std::endl;
    }
    if (result) {
        *dataSize = bytesReturned / sizeof(TCPData);
    }

    CloseHandle(hDevice);
    return result;
}