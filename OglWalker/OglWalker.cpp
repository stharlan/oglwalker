// OglWalker.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "OglWalker.h"

#pragma comment(lib, "C:\\Library\\glew-2.1.0\\vs2017\\glew\\x64\\Release\\glew.lib")
#pragma comment(lib, "CORE_RL_Magick++_.lib")
#pragma comment(lib, "CORE_RL_MagickCore_.lib")

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

typedef struct {
	GLuint base;
	int widths[256];
	int height;
} GLFONT;

RENDER_THREAD_CONTEXT g_ctx;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
HWND                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void FontPuts(GLFONT* font, const char* s);
void FontDestroy(GLFONT* font);
GLFONT* FontCreate(HDC hdc, const wchar_t* typeface, int height, int weight, DWORD italic);
void FontPrintf(GLFONT     *font,   /* I - Font to use */
	int        align,   /* I - Alignment to use */
	const char *format, /* I - printf() style format string */
	...);                /* I - Other arguments as necessary */
BOOL SetupDirectInput(HINSTANCE hInst, HWND hWnd);
void CleanupDirectInput();

HANDLE hRenderingThread = NULL;
DWORD RenderThreadId = 0;
//GLFONT* pFont;
FILE* g_log;

//void ProcessFloor(glm::vec3& p) {
//	if (p.x >= 30.0f && p.z <= -30.0f) {
//		p.y = 5.0f;
//	}
//	else if (p.x <= -30.0f && p.z >= 30.0f) {
//		p.y = -5.0f;
//	}
//}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

	fopen_s(&g_log, "c:\\temp\\ow.log", "w");

	//objl::Loader loader;
	//std::string objfile = "";
	//bool bRet = loader.LoadFile(objfile);

	//Point origin(0, 0, 0);
	//Point ray(0, -1, 0);
	//Triangle t(Point(0, -2, 1), Point(1, -2, -1), Point(-1, -2, -1));
	//Point result;
	//bool b = RayIntersectsTriangle(origin, ray, t, result);
	//fprintf(g_log, "%i %.1f %.1f %.1f\n", b, result.x, result.y, result.z);

	//Point a(5, 1, 0);
	//Point b(-1, 0, 0);
	//float adotb = a.dotProduct(b);
	//float bdotb = b.dotProduct(b);
	//Point rp = b * (adotb / bdotb);
	//fprintf(g_log, "proj %.1f, %.1f, %.1f\n", rp.x, rp.y, rp.z);

	//Point p1(0, 0, 0);
	//Point p2(1, 0, 0);
	//Point p3(1, 1, 0);
	//Triangle t(p1, p2, p3);
	//Point nv = t.SurfaceNormal();
	//fprintf(g_log, "nv %.1f, %.1f, %.1f\n", nv.x, nv.y, nv.z);

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_OGLWALKER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
	HWND hWnd = InitInstance(hInstance, nCmdShow);
	if(hWnd == NULL)
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OGLWALKER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	fclose(g_log);

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OGLWALKER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName = NULL; // MAKEINTRESOURCEW(IDC_OGLWALKER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hDesktopWnd = GetDesktopWindow();
   HDC hDesktopDc = GetDC(hDesktopWnd);

   int sx = GetDeviceCaps(hDesktopDc, HORZRES);
   int sy = GetDeviceCaps(hDesktopDc, VERTRES);

   ReleaseDC(hDesktopWnd, hDesktopDc);

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, 
	   //WS_OVERLAPPEDWINDOW,
	   WS_POPUP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
	   0,0,sx,sy,
		//CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 
	   nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return NULL;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return hWnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL useDI = FALSE;
    switch (message)
    {
	case WM_CREATE:
		{
			// create the rendering thread
			g_ctx.hQuitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			g_ctx.hWnd = hWnd;
			if (FALSE == SetupDirectInput(hInst, hWnd)) {
				useDI = FALSE;
			}
			else {
				useDI = TRUE;
				ShowCursor(FALSE);
			}
			g_ctx.useDI = useDI;
			g_ctx.InputFilename = "c:\\temp\\objects.json";
			hRenderingThread = (HANDLE)CreateThread(NULL, 0, RenderingThreadTwoEntryPoint, (LPVOID)&g_ctx, 0, &RenderThreadId);
		}
		break;
	case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
			ValidateRect(hWnd, NULL);
        }
        break;
	case CUSTOM_QUIT:
    case WM_DESTROY:
		{
			SetEvent(g_ctx.hQuitEvent);
			WaitForSingleObject(hRenderingThread, INFINITE);
			if (TRUE == useDI) {
				ShowCursor(TRUE);
				CleanupDirectInput();
			}
			PostQuitMessage(0);
		}
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}



/*
* Limits...
*/

#define MAX_STRING	1024


/*
* 'FontCreate()' - Load Windows font bitmaps into OpenGL display lists.
*/

GLFONT *                         /* O - Font data */
FontCreate(HDC        hdc,       /* I - Device Context */
	const wchar_t *typeface, /* I - Font specification */
	int        height,    /* I - Font height/size in pixels */
	int        weight,    /* I - Weight of font (bold, etc) */
	DWORD      italic)    /* I - Text is italic */
{
	GLFONT *font;                /* Font data pointer */
	HFONT  fontid;               /* Windows font ID */
	int    charset;              /* Character set code */

								 /* Allocate memory */
	if ((font = (GLFONT*)calloc(1, sizeof(GLFONT))) == (GLFONT *)0)
		return ((GLFONT *)0);

	/* Allocate display lists */
	if ((font->base = glGenLists(256)) == 0)
	{
		free(font);
		return (0);
	}

	/* Select a character set */
	if (wcscmp(typeface, L"symbol") == 0)
		charset = SYMBOL_CHARSET;
	else
		charset = ANSI_CHARSET;

	/* Load the font */
	fontid = CreateFont(height, 0, 0, 0, weight, italic, FALSE, FALSE,
		charset, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
		DRAFT_QUALITY, DEFAULT_PITCH, typeface);

	SelectObject(hdc, fontid);

	/* Create bitmaps for each character */
	wglUseFontBitmaps(hdc, 0, 256, font->base);

	/* Get the width and height information for each character */
	GetCharWidth(hdc, 0, 255, font->widths);
	font->height = height;

	return (font);
}

/*
* 'FontDestroy()' - Delete the specified font.
*/

void
FontDestroy(GLFONT *font) /* I - Font to delete */
{
	if (font == (GLFONT *)0)
		return;

	glDeleteLists(font->base, 256);
	free(font);
}


/*
* 'FontPuts()' - Display a string using the specified font.
*/

void
FontPuts(GLFONT     *font, /* I - Font to use */
	const char *s)    /* I - String to display */
{
	if (font == (GLFONT *)0 || s == NULL)
		return;

	glPushAttrib(GL_LIST_BIT);
	glListBase(font->base);
	glCallLists((GLsizei)strlen(s), GL_UNSIGNED_BYTE, s);
	glPopAttrib();
}


/*
* 'FontPrintf()' - Display a formatted string using the specified font.
*/

void
FontPrintf(GLFONT     *font,   /* I - Font to use */
	int        align,   /* I - Alignment to use */
	const char *format, /* I - printf() style format string */
	...)                /* I - Other arguments as necessary */
{
	va_list       ap;          /* Argument pointer */
	unsigned char s[1024],     /* Output string */
		*ptr;        /* Pointer into string */
	int           width;       /* Width of string in pixels */

	if (font == (GLFONT *)0 || format == (char *)0)
		return;

	/* Format the string */
	va_start(ap, format);
	vsprintf_s((char *)s, 1024, format, ap);
	va_end(ap);

	/* Figure out the width of the string in pixels... */
	for (ptr = s, width = 0; *ptr; ptr++)
		width += font->widths[*ptr];

	/* Adjust the bitmap position as needed to justify the text */
	if (align < 0)
		glBitmap(0, 0, 0, 0, (float)-width, 0, NULL);
	else if (align == 0)
		glBitmap(0, 0, 0, 0, (float)-width / 2, 0, NULL);

	/* Draw the string */
	FontPuts(font, (const char*)s);
}

IDirectInput8 *pDirectInput8 = NULL;
extern IDirectInputDevice8 *pKeyboard;
extern IDirectInputDevice8 *pMouse;

void CleanupDirectInput()
{
	if (pMouse != NULL) {
		pMouse->Unacquire();
		pMouse->Release();
		pMouse = NULL;
	}
	if (pKeyboard != NULL) {
		pKeyboard->Unacquire();
		pKeyboard->Release();
		pKeyboard = NULL;
	}
	if (pDirectInput8 != NULL) {
		pDirectInput8->Release();
		pDirectInput8 = NULL;
	}
}

BOOL SetupDirectInput(HINSTANCE hInst, HWND hWnd)
{
	if (FAILED(DirectInput8Create(hInst, 0x0800, IID_IDirectInput8, (void**)&pDirectInput8, NULL)))
		goto ERR;

	if (FAILED(pDirectInput8->CreateDevice(GUID_SysKeyboard, &pKeyboard, NULL)))
		goto ERR;

	if (FAILED(pKeyboard->SetDataFormat(&c_dfDIKeyboard)))
		goto ERR;

	if (FAILED(pKeyboard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
		goto ERR;

	if (FAILED(pKeyboard->Acquire()))
		goto ERR;

	if (FAILED(pDirectInput8->CreateDevice(GUID_SysMouse, &pMouse, NULL)))
		goto ERR;

	if (FAILED(pMouse->SetDataFormat(&c_dfDIMouse)))
		goto ERR;

	if (FAILED(pMouse->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
		goto ERR;

	if (FAILED(pMouse->Acquire()))
		goto ERR;

	return TRUE;

ERR:
	CleanupDirectInput();
	return FALSE;
}
