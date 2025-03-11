#include <windows.h>
#include <iostream>
#include <string>

bool IsAdmin() {
    BOOL isAdmin = FALSE;
    HANDLE token = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        TOKEN_ELEVATION elevation;
        DWORD size;
        if (GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size)) {
            isAdmin = elevation.TokenIsElevated;
        }
        CloseHandle(token);
    }
    return isAdmin;
}

void RunAsAdmin() {
    wchar_t executable[MAX_PATH];
    GetModuleFileNameW(NULL, executable, MAX_PATH);
    
    SHELLEXECUTEINFOW sei = { sizeof(sei) };
    sei.lpVerb = L"runas";
    sei.lpFile = executable;
    sei.nShow = SW_SHOWNORMAL;
    
    if (!ShellExecuteExW(&sei)) {
        std::cerr << "Failed to request administrator privileges." << std::endl;
    }
    exit(0);
}

void ToggleRegistryValue(const std::wstring& keyPath, const std::wstring& valueName) {
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, KEY_READ | KEY_WRITE, &hKey);
    
    if (result != ERROR_SUCCESS) {
        std::wcerr << L"Failed to open registry key: " << keyPath << std::endl;
        return;
    }
    
    DWORD value = 0;
    DWORD size = sizeof(DWORD);
    DWORD type = REG_DWORD;
    
    result = RegQueryValueExW(hKey, valueName.c_str(), 0, &type, reinterpret_cast<LPBYTE>(&value), &size);
    if (result != ERROR_SUCCESS) {
        std::wcerr << L"Failed to query registry value: " << valueName << std::endl;
        RegCloseKey(hKey);
        return;
    }
    
    DWORD newValue = (value == 1) ? 0 : 1;
    result = RegSetValueExW(hKey, valueName.c_str(), 0, REG_DWORD, reinterpret_cast<const BYTE*>(&newValue), sizeof(DWORD));
    if (result == ERROR_SUCCESS) {
        std::wcout << L"Toggled " << valueName << L" from " << value << L" to " << newValue << std::endl;
    } else {
        std::wcerr << L"Failed to set registry value: " << valueName << std::endl;
    }
    
    RegCloseKey(hKey);
}

int main() {
    if (!IsAdmin()) {
        std::cout << "Requesting Administrator privileges..." << std::endl;
        RunAsAdmin();
    }
    
    std::wstring registryPath = L"SYSTEM\\CurrentControlSet\\Control\\PriorityControl";
    ToggleRegistryValue(registryPath, L"SystemDockMode");
    ToggleRegistryValue(registryPath, L"ConvertibleSlateMode");
    
    return 0;
}
