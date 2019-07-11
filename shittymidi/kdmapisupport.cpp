#include "kdmapisupport.h"
#include <iostream>

MMRESULT(*KShortMsg)(DWORD) = nullptr;

bool InitKDMApi() {
    HMIDIOUT hmo;
    MMRESULT mmres = midiOutOpen(&hmo, 1, NULL, 0, NULL);
    if (mmres != MMSYSERR_NOERROR) {
        // Device 1 doesn't exist or failed to initialize, let's initialize Microsoft GS instead
        mmres = midiOutOpen(&hmo, 0, NULL, 0, NULL);
    }
    if (mmres != MMSYSERR_NOERROR) {
        // Microsoft GS also failed to initialize, close the app
        return false;
    }

    // This block of code is from WinMMWRP
    wchar_t omnimidi_dir[MAX_PATH];
    // Clear memory
    memset(omnimidi_dir, 0, sizeof(omnimidi_dir));
    // Get system directory
    GetSystemDirectoryW(omnimidi_dir, MAX_PATH);
    // Append system directory to OMDir and NTDLLDir, with their respective targets
    wcscat(omnimidi_dir, L"\\OmniMIDI\\OmniMIDI.dll");
    // Load the default DLL from the app's directory
    HMODULE omnimidi_handle = LoadLibraryW(L"OmniMIDI.dll");
    if (!omnimidi_dir) {
        omnimidi_handle = LoadLibraryW(omnimidi_dir);
        if (!omnimidi_handle)
            return false;
    }

    KShortMsg = (MMRESULT(*)(DWORD))GetProcAddress(omnimidi_handle, "SendDirectData");
    std::cout << GetLastError() << std::endl;
    if (KShortMsg == nullptr)
        return false;
    return true;
}