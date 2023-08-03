#include <iostream>
#include <sstream>
#include <string>
#include <Windows.h>
#include <psapi.h>
#include <Shlwapi.h>
#include <atlbase.h>
#include <memory>
#include <vector>
#include <iomanip> // For std::setprecision

#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

using namespace std;

string get_processor_description() {
    string processorName = "Informação do processador não disponível.";

    HKEY key;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Hardware\\Description\\System\\CentralProcessor\\0", 0, KEY_READ, &key) == ERROR_SUCCESS) {
        char processorNameBuffer[MAX_PATH];
        DWORD dataSize = sizeof(processorNameBuffer);
        if (RegQueryValueExA(key, "ProcessorNameString", nullptr, nullptr, (BYTE*)processorNameBuffer, &dataSize) == ERROR_SUCCESS) {
            processorName = processorNameBuffer;
        }
        RegCloseKey(key);
    }

    return processorName;
}

string format_disk_space(ULARGE_INTEGER space) {
    double sizeGB = static_cast<double>(space.QuadPart) / (1024 * 1024 * 1024);
    stringstream ss;
    ss << fixed << setprecision(2) << sizeGB;
    return ss.str() + " GB";
}

// Get a list of available disk drives
vector<string> get_disk_drives() {
    vector<string> drives;
    char buffer[MAX_PATH] = { 0 };
    GetLogicalDriveStringsA(sizeof(buffer) - 1, buffer);

    char* drive = buffer;
    while (*drive) {
        drives.push_back(drive);
        drive += strlen(drive) + 1;
    }

    return drives;
}

string get_system_info() {
    // Get system information
    char hostName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(hostName);
    GetComputerNameA(hostName, &size);

    string processorName = get_processor_description();

    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(memInfo);
    GlobalMemoryStatusEx(&memInfo);

    // Build the information string
    stringstream ss;
    ss << "Nome do Host: " << hostName << "\n";
    ss << "Processador: " << processorName << "\n";
    ss << "Memória RAM Instalada: " << memInfo.ullTotalPhys / (1024 * 1024 * 1024) << " GB\n";



    // Get a list of available disk drives
    vector<string> drives = get_disk_drives();

    // Append information for each disk
    ss << "Discos Rígidos:\n";
    for (const auto& drive : drives) {
        string diskLetter = drive.substr(0, 2);
        ULARGE_INTEGER totalSpace;
        ULARGE_INTEGER freeSpace;
        GetDiskFreeSpaceExA(drive.c_str(), nullptr, &totalSpace, &freeSpace);
        ss << "  Disco " << diskLetter << ": " << format_disk_space(totalSpace) << "\n";
    }

    return ss.str();
}

// Entry point for the application
int main() {
    string infoStr = get_system_info();
    const char* title = "Informações do Sistema";

    // Show popup box with the information
    MessageBoxA(nullptr, infoStr.c_str(), title, MB_OK | MB_ICONINFORMATION);

    return 0;
}

// Set the entry point to main instead of WinMain
#pragma comment(linker, "/entry:mainCRTStartup")
