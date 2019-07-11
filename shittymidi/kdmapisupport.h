#pragma once
#include <Windows.h>
#include <mmsystem.h>

#ifndef KDMAPISUPPORT_H
#define KDMAPISUPPORT_H
extern MMRESULT(*KShortMsg)(DWORD);

bool InitKDMApi();
#endif