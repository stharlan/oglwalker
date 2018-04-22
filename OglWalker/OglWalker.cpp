// OglWalker.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "OglWalker.h"

#define MAX_LOADSTRING 100
#define PI 3.14159f
#define DEG2RAD(x) (x * PI / 180.0f)
#define CUSTOM_QUIT (WM_USER + 1)

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

typedef struct {
	GLuint base;
	int widths[256];
	int height;
} GLFONT;

typedef struct {
	HANDLE hQuitEvent;
	HWND hWnd;
	BOOL useDI;
} RENDER_THREAD_CONTEXT;

RENDER_THREAD_CONTEXT g_ctx;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
HWND                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void SetDCPixelFormat(HDC hdc);
void ChangeSize(GLsizei w, GLsizei h);
void FontPuts(GLFONT* font, const char* s);
void FontDestroy(GLFONT* font);
GLFONT* FontCreate(HDC hdc, const wchar_t* typeface, int height, int weight, DWORD italic);
void FontPrintf(GLFONT     *font,   /* I - Font to use */
	int        align,   /* I - Alignment to use */
	const char *format, /* I - printf() style format string */
	...);                /* I - Other arguments as necessary */
BOOL SetupDirectInput(HINSTANCE hInst, HWND hWnd);
void CleanupDirectInput();
bool ReadMouseState(DIMOUSESTATE *pMouseState);
bool ReadKeyboardState(unsigned char* keystate);

HANDLE hRenderingThread = NULL;
DWORD RenderThreadId = 0;
GLFONT* pFont;
FILE* g_log;

void SetupRC()
{
	pFont = FontCreate(wglGetCurrentDC(), L"Arial", 18, 0, 0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
}

void SetPerspective(int w, int h)
{
	GLfloat fAspect;
	if (h == 0) h = 1;
	fAspect = (GLfloat)w / (GLfloat)h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, fAspect, 1.0, 400.0);
	glMatrixMode(GL_MODELVIEW);
}

void SetOrtho(int w, int h)
{
	if (h == 0) h = 1;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, (GLfloat)w, 0.0, (GLfloat)h, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

void RenderScene(int w, int h, float fps, float azimuth, float elevation, float px, float pz, float ex, float ez)
//void RenderScene(int w, int h, float fps, float azimuth, float elevation, float px, float pz)
{
	glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SetPerspective(w, h);

	glPushMatrix();
	glLoadIdentity();
	glRotatef(azimuth, 0.0f, 1.0f, 0.0);

	float erad = DEG2RAD(azimuth);
	GLfloat eex = cosf(erad);
	GLfloat eez = sinf(erad);
	glRotatef(elevation, eex, 0.0f, eez);
	glTranslatef(px, -6.0f, pz);

	// do 3d stuff here
	glColor3f(1.0f, 1.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_TRIANGLES);
	glLineWidth(1.0f);
	for (float x = -50.0f; x < 50.0f; x += 5.0f) {
		for(float z = -50.0f; z<50.0f; z += 5.0f) {
			glVertex3f(x, 0.0f, z);
			glVertex3f(x + 5.0f, 0.0f, z);
			glVertex3f(x + 5.0f, 0.0f, z + 5.0f);
			glVertex3f(x, 0.0f, z);
			glVertex3f(x + 5.0f, 0.0f, z + 5.0f);
			glVertex3f(x, 0.0f, z + 5.0f);
		}
	}
	glEnd();

	glPopMatrix();

	SetOrtho(w, h); 

	glColor3f(0.0f, 1.0f, 0.0f);
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glVertex2i((w / 2) - 10, h / 2);
	glVertex2i((w / 2) + 10, h / 2);
	glVertex2i(w / 2, (h / 2) - 10);
	glVertex2i(w / 2, (h / 2) + 10);
	glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);
	glRasterPos2i(4, 18);
	FontPrintf(pFont, 1, "hello world %i", (int)fps);

	glRasterPos2i((w / 2) + 10, (h / 2) + 10);
	FontPrintf(pFont, 1, "%.0f degrees", azimuth);

	glRasterPos2i((w / 2) + 10, (h / 2) + 20);
	FontPrintf(pFont, 1, "%.4f, %.4f", ex, ez);

	glFinish();

}

DWORD WINAPI RenderingThreadEntryPoint(void* pVoid) 
{
	RENDER_THREAD_CONTEXT* ctx = (RENDER_THREAD_CONTEXT*)pVoid;

	float azimuth = 0.0f;
	float elevation = 0.0f;
	float px = 0.0f;
	float pz = 0.0f;

	FILE* log = NULL;
	fopen_s(&log, "c:\\temp\\rt.log", "w");


	fprintf(log, "create rc\n");
	HDC hdc = GetDC(ctx->hWnd);
	SetDCPixelFormat(hdc);
	HGLRC hrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hrc);

	RECT rect;
	GetClientRect(ctx->hWnd, &rect);
	ChangeSize(rect.right, rect.bottom);

	SetupRC();

	LARGE_INTEGER perfCount;
	LARGE_INTEGER perfFreq;

	QueryPerformanceFrequency(&perfFreq);
	fprintf(log, "perf freq %lli\n", perfFreq.QuadPart);

	LONGLONG lastCount = 0;
	unsigned char keystate[256];
	DIMOUSESTATE mouseState;

	GLfloat ex = 0.0f;
	GLfloat ez = 0.0f;
	float moveDir = 0.0f, erad = 0.0f;

	fprintf(log, "start loop\n");
	while (1) {
		// check state
		if (WaitForSingleObject(ctx->hQuitEvent, 0) == WAIT_OBJECT_0) {
			fprintf(log, "quit event is triggered\n");
			break;
		}

		QueryPerformanceCounter(&perfCount);

		float fps = (float)perfFreq.QuadPart / (float)(perfCount.QuadPart - lastCount);
		lastCount = perfCount.QuadPart;

		if (TRUE == ctx->useDI) {
			if (TRUE == ReadMouseState(&mouseState))
			{
				//fprintf(log, "mouse %i, %i, %i\n", mouseState.lX, mouseState.lY, mouseState.lZ);
				azimuth += (mouseState.lX / 8.0f);
				if (azimuth < 0.0f) azimuth = azimuth + 360.0f;
				if (azimuth > 360.0f) azimuth = azimuth - 360.0f;

				elevation += (mouseState.lY / 8.0f);
				if (elevation < -90.0f) elevation = -90.0f;
				if (elevation > 90.0f) elevation = 90.0f;
			}

			if (TRUE == ReadKeyboardState(&keystate[0]))
			{
				fprintf(log, "esc = %i\n", keystate[DIK_ESCAPE]);
				if (keystate[DIK_ESCAPE] == (unsigned char)128) {
					PostMessage(ctx->hWnd, CUSTOM_QUIT, 0, 0);
				}

				ex = ez = 0.0f;
				
				if (keystate[DIK_W] == (unsigned char)128) {
					if (keystate[DIK_A] == (unsigned char)128) {
						moveDir = azimuth - 45.0f;
						erad = DEG2RAD(moveDir);
						ex = sinf(erad);
						ez = cosf(erad);
					}
					else if (keystate[DIK_D] == (unsigned char)128) {
						moveDir = azimuth + 45.0f;
						erad = DEG2RAD(moveDir);
						ex = sinf(erad);
						ez = cosf(erad);
					}
					else {
						moveDir = azimuth;
						erad = DEG2RAD(moveDir);
						ex = sinf(erad);
						ez = cosf(erad);
					}
				} else if (keystate[DIK_S] == (unsigned char)128) {
					if (keystate[DIK_A] == (unsigned char)128) {
						moveDir = azimuth - 135.0f;
						erad = DEG2RAD(moveDir);
						ex = sinf(erad);
						ez = cosf(erad);
					}
					else if (keystate[DIK_D] == (unsigned char)128) {
						moveDir = azimuth + 135.0f;
						erad = DEG2RAD(moveDir);
						ex = sinf(erad);
						ez = cosf(erad);
					}
					else {
						moveDir = azimuth + 180.0f;
						erad = DEG2RAD(moveDir);
						ex = sinf(erad);
						ez = cosf(erad);
					}
				} else if (keystate[DIK_A] == (unsigned char)128) {
					moveDir = azimuth - 90.0f;
					erad = DEG2RAD(moveDir);
					ex = sinf(erad);
					ez = cosf(erad);
				} else if (keystate[DIK_D] == (unsigned char)128) {
					moveDir = azimuth + 90.0f;
					erad = DEG2RAD(moveDir);
					ex = sinf(erad);
					ez = cosf(erad);
				}
				pz += 0.01f * ez;
				px -= 0.01f * ex;
			}
		}

		RenderScene(rect.right, rect.bottom, fps, azimuth, elevation, px, pz, ex, ez);

		SwapBuffers(hdc);
	}

	fprintf(log, "destroy rc stuff\n");
	FontDestroy(pFont);
	wglMakeCurrent(hdc, NULL);
	wglDeleteContext(hrc);


	fprintf(log, "quit\n");
	fclose(log);
	return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

	fopen_s(&g_log, "c:\\temp\\ow.log", "w");

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
			hRenderingThread = (HANDLE)CreateThread(NULL, 0, RenderingThreadEntryPoint, (LPVOID)&g_ctx, 0, &RenderThreadId);
		}
		break;
	//case WM_SIZE:
	//	fprintf(g_log, "resizing\n");
	//	ChangeSize(LOWORD(lParam), HIWORD(lParam));
	//	break;
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

void SetDCPixelFormat(HDC hdc) 
{
	int nPixelFormat;
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1, // version
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32, // bit color
		0,0,0,0,0,0,
		0,0,
		0,0,0,0,0,
		16, // depth buffer
		0,
		0,
		0,
		0,
		0,0,0 };
	nPixelFormat = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, nPixelFormat, &pfd);
}

void ChangeSize(GLsizei w, GLsizei h) {
	fprintf(g_log, "ChangeSize start %i, %i\n", w, h);
	GLfloat fAspect;
	if (h == 0) h = 1;
	glViewport(0, 0, w, h);
	fAspect = (GLfloat)w / (GLfloat)h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(60.0f, fAspect, 1.0, 400.0);
	glOrtho(0.0, (GLfloat)w, 0.0, (GLfloat)h, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	fprintf(g_log, "ChangeSize end\n");
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
IDirectInputDevice8 *pKeyboard = NULL;
IDirectInputDevice8 *pMouse = NULL;

bool ReadKeyboardState(unsigned char* keystate)
{
	HRESULT result;
	result = pKeyboard->GetDeviceState(sizeof(unsigned char[256]), (LPVOID)keystate);
	if (FAILED(result)) {
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)) {
			pKeyboard->Acquire();
		}
		else {
			return false;
		}
	}
	return true;
}

bool ReadMouseState(DIMOUSESTATE *pMouseState)
{
	HRESULT result;

	// Read the mouse device.
	result = pMouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)pMouseState);
	if (FAILED(result))
	{
		// If the mouse lost focus or was not acquired then try to get control back.
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			pMouse->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}

void CleanupDirectInput()
{
	if (pMouse != NULL) {
		pMouse->Unacquire();
		pMouse->Release();
	}
	if (pKeyboard != NULL) {
		pKeyboard->Unacquire();
		pKeyboard->Release();
	}
	if (pDirectInput8 != NULL) pDirectInput8->Release();
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
