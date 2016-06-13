#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "Header.h"

struct Legaturi_Orase {
	int x_in, y_in;   //coordonatele nodului sursa
	int x_out, y_out; //coordonatele nodului destinatie
	int sursa, destinatia;
};
Legaturi_Orase muchii[10];
						   
HINSTANCE _hInstance; 
int _nCmdShow;
HWND _hwnd, _hwndPATH; //handle catre fereastra principala si fereastra path
MSG msg; 
HWND _sursa, _destinatia; //handle pentru datele introduse sursa si destinatie
HWND sursa1, sursa2; //textul afisat pe ecran pentru drumul minim aflat cu dijsktra si costul total
bool check_pathmenu = false; //verificam daca fereastra pathmenu este deschisa/inchisa

//pentru desenare
HDC hDC, hDC2, MemDCExercising, MemDCExercising2;
PAINTSTRUCT Ps;
HFONT font;
LOGFONT LogFont;

//pentru Dijktra
int calea[11], k = 0, k2 = 0;
int graf_orase[9][9];
void getPath(int *parent, int j);
void dijkstra(int graph[9][9], int src, int dest, int nodes);
void Initializare_Cautare(int src, int dest);

//fereastra principala si functiile acesteia
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void Paint_MAINWND();
void Draw_Buttons(HWND hwnd);
int convert_date(char *text);
char* nume_oras(int id_oras);
void Initializare_Noduri();

//a doua fereastra in care vom afisa drumul
LRESULT CALLBACK WndProc_Path(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void Register_Path();
void Paint_PATHWND(HWND hwnd);

//fereastra de instructiuni
LRESULT CALLBACK WndProc_Instruction(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	_hInstance = hInstance;
	_nCmdShow = nCmdShow;
	Initializare_Noduri();
	WNDCLASSW wc; //declarare fereastra

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

	RegisterClassW(&wc); //inregistram clasa

	_hwnd = CreateWindowW(L"MainWindow", L"Dijkstra",   //cream efectiv fereastra cu dimensiune, nume, stil
		WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE, //stil
		0, 0, 1220, 700, NULL, NULL, hInstance, NULL);

	ShowWindow(_hwnd, nCmdShow); //afiasm fereastra si updatam
	UpdateWindow(_hwnd);

	while (GetMessage(&msg, NULL, 0, 0)) {   //loopul infinit pentru comenzile de intrare
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int surs;
	int dest;

	switch (msg) {

	case WM_PAINT:
		Paint_MAINWND();
		break;

	case WM_CREATE:
		Draw_Buttons(hwnd);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_CALC:
		{
			bool okk = false;
			char buf1[30], buf2[30];
			SendMessage(_sursa, WM_GETTEXT, sizeof(buf1) / sizeof(char), reinterpret_cast<LPARAM>(buf1));
			SendMessage(_destinatia, WM_GETTEXT, sizeof(buf2) / sizeof(char), reinterpret_cast<LPARAM>(buf2));

			//resetam casutele in care introducem localitatile si le reinitializam
			DestroyWindow(_sursa);
			DestroyWindow(_destinatia);
			_sursa = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE, 1000, 270, 150, 25, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
			_destinatia = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE, 1000, 315, 150, 25, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);

			if (convert_date(buf1) != -1 && convert_date(buf2) != -1)
			{
				surs = convert_date(buf1);
				dest = convert_date(buf2);
				okk = true;
			}
			else
			{
				MessageBoxW(hwnd, L"Orase invalide", L"Error", MB_ICONERROR);
			}
			if (okk)
			{
				Initializare_Cautare(surs, dest);
			    //calculam drumul
				int sum = 0;
				char sum_buff[101];
				for (int i = 0; i < 99; i++)
					sum_buff[i] = '\0';
				for (int i = 0; i < k - 1; i++)
					sum += graf_orase[calea[i]][calea[i + 1]];

				sprintf(sum_buff, "Costul total este de: %d", sum);
				//traseul Parcurs
				char buff[3000], bufff[60];
				for (int i = 0; i < 2999; i++)
					buff[i] = '\0';
				for (int i = 0; i < 60; i++)
					bufff[i] = '\0';
				strcpy(bufff, nume_oras(calea[0]));
				sprintf(buff, "%s", bufff);
				for (int i = 1; i < k; i++)
				{
					strcpy(bufff, nume_oras(calea[i]));
					sprintf(buff, "%s  ->  %s", buff, bufff);
				}
				sursa1 = CreateWindow("static", buff, WS_CHILD | WS_VISIBLE, 850, 400, 350, 100, hwnd, (HMENU)1, NULL, NULL);
				sursa2 = CreateWindow("static", sum_buff, WS_CHILD | WS_VISIBLE, 850, 500, 350, 100, hwnd, (HMENU)1, NULL, NULL);
				k = 0;
			}
		}
		break;
		case IDC_RESET: //redam aspectul initial al ferestrei la apasarea butonului de reset
			DestroyWindow(_sursa);
			DestroyWindow(_destinatia);
			_sursa = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE, 1000, 270, 150, 25, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
			_destinatia = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE, 1000, 315, 150, 25, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
			DestroyWindow(sursa1);
			DestroyWindow(sursa2);
			break;
		case IDC_PATHWND:
		{
			if (!check_pathmenu)
			{
				check_pathmenu = true;
				Register_Path();
				_hwndPATH = CreateWindowW(L"PathWnd", L"Path", WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE, 0, 0, 818, 700, hwnd, NULL, _hInstance, NULL);
				ShowWindow(_hwndPATH, _nCmdShow);
				UpdateWindow(_hwndPATH);
			}
			else
			{
				check_pathmenu = false;
				DestroyWindow(_hwndPATH);
			}
		}
		break;
		case IDC_INST:
		{
			WNDCLASSW wc; //declarare fereastra

			wc.style = CS_HREDRAW | CS_VREDRAW;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.lpszClassName = L"Instructiuni";
			wc.hInstance = _hInstance;
			wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
			wc.lpszMenuName = NULL;
			wc.lpfnWndProc = WndProc_Instruction;
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

			RegisterClassW(&wc);

			HWND _inst = CreateWindowW(L"Instructiuni", L"Instructiuni",
				WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE,
				0, 0, 750, 550, hwnd, NULL, _hInstance, NULL);

			ShowWindow(_inst, _nCmdShow);
			UpdateWindow(_inst);
		}
		break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void Paint_MAINWND() {
	hDC = BeginPaint(_hwnd, &Ps);
	HPEN hPenOld;
	HPEN hLinePen;
	COLORREF qLineColor;
	HBITMAP bmpExercising;

	bmpExercising = LoadBitmap(_hInstance, MAKEINTRESOURCE(IDB_BITMAP1)); //incarcam imaginea si o afisam
	MemDCExercising = CreateCompatibleDC(hDC);
	SelectObject(MemDCExercising, bmpExercising);
	BitBlt(hDC, 0, 0, 818, 650, MemDCExercising, 0, 0, SRCCOPY); //afisam imaginea cu parametri

	DeleteDC(MemDCExercising);
	DeleteObject(bmpExercising);

	qLineColor = RGB(196, 77, 31);  //maro pt linii
	hLinePen = CreatePen(PS_SOLID, 8, qLineColor); //linie continua, de grosime 8 pixeli
	hPenOld = (HPEN)SelectObject(hDC, hLinePen); //incarcam in hDC noile definitii
	MoveToEx(hDC, 818, 0, NULL); //desenez linia mare
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
	TextOut(hDC, 825, 0, "Dijkstra", 8);

	LogFont.lfUnderline = 0;
	LogFont.lfHeight = 27;
	qLineColor = RGB(77, 109, 204);
	font = CreateFontIndirect(&LogFont);
	SelectObject(hDC, font);
	SetTextColor(hDC, qLineColor);
	TextOut(hDC, 825, 200, "Alege 2 municipii:", 18); //18-lungime text

	DeleteObject(font);

	HBRUSH NewBrush = CreateSolidBrush(RGB(45, 102, 194));
	SelectObject(hDC, NewBrush);
	Rectangle(hDC, 850, 260, 1170, 350);
}

void Draw_Buttons(HWND hwnd)
{
	CreateWindowW(L"static", L"Oras Sursa:", WS_CHILD | WS_VISIBLE, 880, 270, 100, 25, hwnd, (HMENU)1, NULL, NULL);
	_sursa = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE, 1000, 270, 150, 25, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
	CreateWindowW(L"static", L"Oras Dest.:", WS_CHILD | WS_VISIBLE, 880, 315, 100, 25, hwnd, (HMENU)1, NULL, NULL);
	_destinatia = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE, 1000, 315, 150, 25, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
	CreateWindow("BUTTON", "Calculeaza", WS_BORDER | WS_CHILD | WS_VISIBLE, 905, 60, 220, 30, hwnd, (HMENU)IDC_CALC, _hInstance, NULL);
	CreateWindow("BUTTON", "Resetare Desen", WS_BORDER | WS_CHILD | WS_VISIBLE, 905, 100, 220, 30, hwnd, (HMENU)IDC_RESET, _hInstance, NULL);
	CreateWindow("BUTTON", "Instructiuni", WS_BORDER | WS_CHILD | WS_VISIBLE, 905, 140, 220, 30, hwnd, (HMENU)IDC_INST, _hInstance, NULL);
	CreateWindow("BUTTON", "Arata Drum", WS_BORDER | WS_CHILD | WS_VISIBLE, 1050, 620, 150, 30, hwnd, (HMENU)IDC_PATHWND, _hInstance, NULL);
}

int convert_date(char *text) {
	//scoatem spatiile
	int i, j;
	char *output = text;
	for (i = 0, j = 0; i<strlen(text); i++, j++)
	{
		if (text[i] != ' ')
			output[j] = text[i];
		else
			j--;
	}
	output[j] = 0;
	//facem toate literele mici
	for (int i = 0; output[i]; i++) {
		output[i] = tolower(output[i]);
	}
	//verificam nodurile si returnam cate o cifra pentru fiecare oras daca a fost gasit
	if (strcmp(output, "targujiu") == 0)
	{
		return 8;
	}
	else if (strcmp(output, "braila") == 0)
	{
		return 7;
	}
	else if (strcmp(output, "brasov") == 0)
	{
		return 6;
	}
	else if (strcmp(output, "albaiulia") == 0)
	{
		return 5;
	}
	else if (strcmp(output, "cluj") == 0)
	{
		return 4;
	}
	else if (strcmp(output, "iasi") == 0)
	{
		return 3;
	}
	else if (strcmp(output, "suceava") == 0)
	{
		return 2;
	}
	else if (strcmp(output, "bistrita") == 0)
	{
		return 1;
	}
	else if (strcmp(output, "satumare") == 0)
	{
		return 0;
	}
	return -1;
}

char* nume_oras(int id_oras) {
	if (id_oras == 0)
	{
		return "Satu Mare";
	}
	else if (id_oras == 1)
	{
		return "Bistrita";
	}
	else if (id_oras == 2)
	{
		return "Suceava";
	}
	else if (id_oras == 3)
	{
		return "Iasi";
	}
	else if (id_oras == 4)
	{
		return "Cluj";
	}
	else if (id_oras == 5)
	{
		return "Alba Iulia";
	}
	else if (id_oras == 6)
	{
		return "Brasov";
	}
	else if (id_oras == 7)
	{
		return "Braila";
	}
	else if (id_oras == 8)
	{
		return "Targu Jiu";
	}
}

void Initializare_Noduri() {
	//pozitiile pe ecran pentru fiecare muchie
	muchii[0].destinatia = 0;
	muchii[0].sursa = 1;
	muchii[0].x_in = 222 - 9;
	muchii[0].y_in = 150 - 38;
	muchii[0].x_out = 318 - 9;
	muchii[0].y_out = 181 - 38;

	muchii[1].destinatia = 1;
	muchii[1].sursa = 2;
	muchii[1].x_in = 424 - 9;
	muchii[1].y_in = 180 - 38;
	muchii[1].x_out = 519 - 9;
	muchii[1].y_out = 250 - 38;

	muchii[2].destinatia = 2;
	muchii[2].sursa = 3;
	muchii[2].x_in = 613 - 9;
	muchii[2].y_in = 258 - 38;
	muchii[2].x_out = 677 - 9;
	muchii[2].y_out = 313 - 38;

	muchii[3].destinatia = 0;
	muchii[3].sursa = 4;
	muchii[3].x_in = 151 - 9;
	muchii[3].y_in = 182 - 38;
	muchii[3].x_out = 127 - 9;
	muchii[3].y_out = 261 - 38;

	muchii[4].destinatia = 1;
	muchii[4].sursa = 5;
	muchii[4].x_in = 359 - 9;
	muchii[4].y_in = 240 - 38;
	muchii[4].x_out = 307 - 9;
	muchii[4].y_out = 381 - 38;

	muchii[5].destinatia = 2;
	muchii[5].sursa = 6;
	muchii[5].x_in = 541 - 9;
	muchii[5].y_in = 309 - 38;
	muchii[5].x_out = 527 - 9;
	muchii[5].y_out = 437 - 38;

	muchii[6].destinatia = 3;
	muchii[6].sursa = 7;
	muchii[6].x_in = 725 - 9;
	muchii[6].y_in = 357 - 38;
	muchii[6].x_out = 699 - 9;
	muchii[6].y_out = 506 - 38;

	muchii[7].destinatia = 4;
	muchii[7].sursa = 5;
	muchii[7].x_in = 115 - 9;
	muchii[7].y_in = 317 - 38;
	muchii[7].x_out = 208 - 9;
	muchii[7].y_out = 386 - 38;

	muchii[8].destinatia = 6;
	muchii[8].sursa = 7;
	muchii[8].x_in = 515 - 9;
	muchii[8].y_in = 485 - 38;
	muchii[8].x_out = 590 - 9;
	muchii[8].y_out = 526 - 38;

	muchii[9].destinatia = 5;
	muchii[9].sursa = 8;
	muchii[9].x_in = 288 - 9;
	muchii[9].y_in = 435 - 38;
	muchii[9].x_out = 335 - 9;
	muchii[9].y_out = 562 - 38;
}

LRESULT CALLBACK WndProc_Path(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {

	case WM_PAINT:
		Paint_PATHWND(hwnd);
		break;
	case WM_DESTROY:
		check_pathmenu = false;
		DestroyWindow(hwnd);
		break;
	}
	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void Register_Path()
{
	WNDCLASSW wc;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.lpszClassName = L"PathWnd";
	wc.hInstance = _hInstance;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpszMenuName = NULL;
	wc.lpfnWndProc = WndProc_Path;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassW(&wc);
}

void Paint_PATHWND(HWND hwnd) {
	hDC2 = BeginPaint(hwnd, &Ps);
	HPEN hPenOld;
	HPEN hLinePen;
	COLORREF qLineColor;
	HBITMAP bmpExercising2;

	bmpExercising2 = LoadBitmap(_hInstance, MAKEINTRESOURCE(IDB_BITMAP1));
	MemDCExercising2 = CreateCompatibleDC(hDC2);
	SelectObject(MemDCExercising2, bmpExercising2);
	BitBlt(hDC2, 0, 0, 818, 650, MemDCExercising2, 0, 0, SRCCOPY);

	DeleteDC(MemDCExercising2);
	DeleteObject(bmpExercising2);

	qLineColor = RGB(55, 38, 230);
	hLinePen = CreatePen(PS_SOLID, 8, qLineColor);
	hPenOld = (HPEN)SelectObject(hDC2, hLinePen);

	for (int i = 0; i < k2 - 1; i++)
	{
		for (int j = 0; j <= 9; j++)
		{
			if ((calea[i] == muchii[j].sursa && calea[i + 1] == muchii[j].destinatia) || (calea[i + 1] == muchii[j].sursa && calea[i] == muchii[j].destinatia))
			{
				MoveToEx(hDC2, muchii[j].x_in, muchii[j].y_in, NULL);
				LineTo(hDC2, muchii[j].x_out, muchii[j].y_out);
			}
		}
	}
	k2 = 0;
}

LRESULT CALLBACK WndProc_Instruction(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CREATE:
	{
		FILE *fd = fopen("Text.txt", "rt");
		char buff[1000];
		int i = 0;
		while (!feof(fd))
		{
			i++;
			fgets(buff, 1000, fd);
			CreateWindow("static", buff, WS_CHILD | WS_VISIBLE, 0, 30 * i, 700, 30, hwnd, (HMENU)1, NULL, NULL);
		}
		fclose(fd);
	}
	break;

	case WM_DESTROY:
		DestroyWindow(hwnd);
		break;
	}
	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void getPath(int *parent, int j)
{
	if (parent[j] == -1)
		return;
	else if (parent[j] == INT_MIN)
	{
		MessageBox(_hwnd, "Nu exita nici un calea intre cele 2 puncte!", "Error", MB_ICONERROR);
		return;
	}

	getPath(parent, parent[j]);
	calea[k++] = j;
	k2++;
}

void dijkstra(int graph[9][9], int src, int dest, int nodes)
{
	int *dist = (int*)malloc(sizeof(int)*nodes);
	bool *sptSet = (bool*)malloc(sizeof(bool)*nodes);
	int *parent = (int*)malloc(sizeof(int)*nodes);
	for (int i = 0; i < nodes; i++)
	{
		parent[i] = INT_MIN;
		dist[i] = INT_MAX;
		sptSet[i] = false;
	}
	parent[src] = -1;
	dist[src] = 0;

	for (int count = 0; count < nodes - 1; count++)
	{
		int u;
		int min = INT_MAX, min_index;
		for (int v = 0; v < nodes; v++)
			if (sptSet[v] == false && dist[v] <= min)
				min = dist[v], min_index = v;
		u = min_index;
		sptSet[u] = true;

		for (int v = 0; v < nodes; v++)
			if (!sptSet[v] && graph[u][v] && dist[u] + graph[u][v] < dist[v])
			{
				parent[v] = u;
				dist[v] = dist[u] + graph[u][v];
			}
	}
	if (dist[dest] != INT_MAX)
	{
		if (dest != src)
		{
			calea[k++] = src;
			k2++;
			getPath(parent, dest);
		}
	}
	else
	{
		MessageBox(_hwnd, "Nu exista calea!", "Error", MB_ICONERROR);
	}
}

void Initializare_Cautare(int src, int dest)
{
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			graf_orase[i][j] = 0;

	graf_orase[0][1] = graf_orase[1][0] = 147;
	graf_orase[0][4] = graf_orase[4][0] = 133;
	graf_orase[1][2] = graf_orase[2][1] = 105;
	graf_orase[1][5] = graf_orase[5][1] = 213;
	graf_orase[2][3] = graf_orase[3][2] = 117;
	graf_orase[2][6] = graf_orase[6][2] = 223;
	graf_orase[3][7] = graf_orase[7][3] = 198;
	graf_orase[4][5] = graf_orase[5][4] = 97;
	graf_orase[5][8] = graf_orase[8][5] = 267;
	graf_orase[6][7] = graf_orase[7][6] = 119;

	dijkstra(graf_orase, src, dest, 9);
}
