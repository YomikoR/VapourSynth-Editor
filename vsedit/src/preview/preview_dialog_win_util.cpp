#pragma comment (lib, "gdi32.lib")
#pragma comment (lib, "user32.lib")

#include "preview_dialog.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

QString PreviewDialog::getICM() const
{
	HWND hwnd = (HWND)winId();
	HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFOEX mi;
	mi.cbSize = sizeof(mi);
	GetMonitorInfo(monitor, &mi);
	HDC ic = CreateIC(mi.szDevice, NULL, NULL, NULL);
	if(!ic) return QString();
	DWORD max_path_length = MAX_PATH;
	wchar_t icm_path[MAX_PATH + 1];
	GetICMProfile(ic, &max_path_length, icm_path);
	return QString::fromWCharArray(icm_path);
}
