#ifndef HELPERS_GUI_H_INCLUDED
#define HELPERS_GUI_H_INCLUDED

#include <QColor>

#pragma comment (lib, "gdi32.lib")
#pragma comment (lib, "user32.lib")

namespace vsedit
{

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

template <typename T>
QString getICM(const T *a_pWidget)
{
	HWND hwnd = (HWND)a_pWidget->winId();
	DWORD max_path_length = MAX_PATH;
	wchar_t icm_path[MAX_PATH + 1];
	GetICMProfile(GetDC(hwnd), &max_path_length, icm_path);
	return QString::fromWCharArray(icm_path);
}

QColor highlight(const QColor & a_color, int a_strength = 20);

}

#endif // HELPERS_GUI_H_INCLUDED
