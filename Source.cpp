#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "Header.h"

HINSTANCE _hInstance;
int _nCmdShow;
HWND _hwnd;
MSG msg;

//Pentru desenare
HDC hDC, MemDCExercising;
PAINTSTRUCT Ps;
HFONT font;
LOGFONT LogFont;

void Paint_MAINWND() {
	hDC = BeginPaint(_hwnd, &Ps); //incepem sa pictam in fereastra principala
	HPEN hPenOld;
	HPEN hLinePen;
	COLORREF qLineColor;
	HBITMAP bmpExercising;

	bmpExercising = LoadBitmap(_hInstance, MAKEINTRESOURCE(IDB_BITMAP1)); //incarcam imaginea
	MemDCExercising = CreateCompatibleDC(hDC);
	SelectObject(MemDCExercising, bmpExercising);
	BitBlt(hDC, 0, 0, 818, 650, MemDCExercising, 0, 0, SRCCOPY); //afisam imaginea

	DeleteDC(MemDCExercising);
	DeleteObject(bmpExercising);

	qLineColor = RGB(196, 77, 31);  //maro pt linii
	hLinePen = CreatePen(PS_SOLID, 8, qLineColor); //linie continua, de grosime 8 pixeli
	hPenOld = (HPEN)SelectObject(hDC, hLinePen); //incarcam in hDC noile definitii
	MoveToEx(hDC, 818, 0, NULL); //desenam linia mare din punctul specificat
	LineTo(hDC, 818, 700); //capatul liniei

	//setam caracteristicile fontului
	LogFont.lfStrikeOut = 0;
	LogFont.lfUnderline = 1;
	LogFont.lfWeight = FW_BOLD;
	LogFont.lfHeight = 35;
	LogFont.lfEscapement = 0;
	LogFont.lfItalic = TRUE;

	qLineColor = RGB(165, 42, 42);
	font = CreateFontIndirect(&LogFont);
	SelectObject(hDC, font);
	SetTextColor(hDC, qLineColor);
	TextOut(hDC, 825, 0, "Dijkstra", 8); //scriem dijkstra sus in dreptul liniei maro
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {

	case WM_PAINT:
		Paint_MAINWND();
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//salvam global instantele programului
	_hInstance = hInstance;
	_nCmdShow = nCmdShow;

	//declaram clasa
	WNDCLASSW wc;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.lpszClassName = L"MainWindow";
	wc.hInstance = hInstance;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpszMenuName = NULL;
	wc.lpfnWndProc = WndProc;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

	//inregistram clasa
	RegisterClassW(&wc);

	_hwnd = CreateWindowW(L"MainWindow", L"Dijkstra",  //cream efectiv fereastra cu dimensiune, nume, stil
		WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE,
		0, 0, 1220, 700, NULL, NULL, hInstance, NULL);

	ShowWindow(_hwnd, nCmdShow); //afiasm fereastra
	UpdateWindow(_hwnd);

	while (GetMessage(&msg, NULL, 0, 0)) {   //loop-ul infinit pentru comenzile de intrare
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
