// XavanteTray.cpp : Defines the entry point for the application.
//

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers


// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <errno.h>
#include <process.h>
#include "resource.h"
#include "SystemTraySDK.h"
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#define MAX_LOADSTRING 100
#define	WM_ICON_NOTIFY WM_APP+10
#define SELECT_TIMEOUT 1

// Global Variables:
HINSTANCE hInst;								// current instance
HWND currenthWnd;
HANDLE hStopSemaphore;
CSystemTray TrayIcon;
lua_State *L;
int xavante_started;
long exit_thread;
const char* xavante_log;
FILE *log;
FILE *aux;
TCHAR xavante_start[10000];

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int Alert(lua_State *L) {
  const char *errmsg;
  errmsg = lua_tostring(L, 1);
  return 1;
}	

int CheckExit(lua_State *L) {
  if(!xavante_log) {
    lua_getglobal(L, "XAVANTE_LOG");  
    xavante_log = lua_tostring(L, -1);
  }
  if(InterlockedExchange(&exit_thread, 0))
    lua_pushboolean(L, 1);
  else
    lua_pushboolean(L, 0);
  return 1;
}

static DWORD GetModulePath( HINSTANCE hInst, LPTSTR pszBuffer, DWORD dwSize )
//
//	Return the size of the path in bytes.
{
	DWORD dwLength = GetModuleFileName( hInst, pszBuffer, dwSize );
	if( dwLength )
	{
		while( dwLength && pszBuffer[ dwLength ] != '.' )
		{
			dwLength--;
		}

		if( dwLength )
			pszBuffer[ dwLength ] = '\000';
	}
	return dwLength;
}

void LuaThread(void *) {
  char name[MAX_PATH];
  DWORD dwLength;
  exit_thread = 0;

  // Initialize Lua
  L = lua_open();
  luaL_openlibs(L);

  lua_pushcfunction(L, (lua_CFunction)Alert);
  lua_setglobal(L, "_ALERT");
	
  // Set finish function
  lua_pushcfunction(L, (lua_CFunction)CheckExit);
  lua_setglobal(L, "XAVANTE_ISFINISHED");

  // Set select timeout
  lua_pushnumber(L, SELECT_TIMEOUT);
  lua_setglobal(L, "XAVANTE_TIMEOUT");

  // Call helper to find paths
  dwLength = GetModulePath( NULL, name, MAX_PATH );
  if(dwLength) { /* Optional bootstrap */
    strcat(name, ".lua");
    if(!luaL_loadfile (L, name)) {
      if(lua_pcall (L, 0, LUA_MULTRET, 0)) {
	MessageBox(currenthWnd, lua_tostring(L, -1),
	       "Error in Bootstrap Helper", MB_ICONERROR | MB_OK | MB_APPLMODAL);
	xavante_started = 0;
	lua_close(L);
	if(aux) fclose(aux);
	if(log) fclose(log);
	ReleaseSemaphore(hStopSemaphore, 1, NULL);
	_endthread();
      }
    }
  }

  // Start Xavante
  if (luaL_loadstring(L, xavante_start)) {
    MessageBox(currenthWnd, lua_tostring(L, -1),
	       "Error loading xavante_start", MB_ICONERROR | MB_OK | MB_APPLMODAL);
  } else if (lua_pcall(L, 0, 0, 0)) {
    MessageBox(currenthWnd, lua_tostring(L, -1),
	       "Lua Error", MB_ICONERROR | MB_OK | MB_APPLMODAL);
  }
  xavante_started = 0;

  lua_close(L);

  if(aux) fclose(aux);
  if(log) fclose(log);

  ReleaseSemaphore(hStopSemaphore, 1, NULL);

  _endthread();
}

void StartXavante() {
  xavante_started = 1;
  xavante_log = NULL;
  hStopSemaphore = CreateSemaphore(NULL, 0, 1, NULL);
  _beginthread(LuaThread, 0, NULL);
}

void StopXavante() {
  InterlockedExchange(&exit_thread, 1);
  WaitForSingleObject(hStopSemaphore, INFINITE);
  CloseHandle(hStopSemaphore);
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
  // TODO: Place code here.
  MSG msg;
  HACCEL hAccelTable;
  HANDLE singleton_mutex;

  LoadString(GetModuleHandle(NULL), 1, xavante_start,
	     sizeof(xavante_start)/sizeof(TCHAR));

  singleton_mutex = CreateMutex(NULL, TRUE, "XavanteTray");

  if(GetLastError() == ERROR_ALREADY_EXISTS) return 1;

  // Initialize global strings
  MyRegisterClass(hInstance);

  // Perform application initialization:
  if (!InitInstance (hInstance, nCmdShow)) 
    {
      return FALSE;
    }

  hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_XAVANTETRAY);

  if(strcmp(lpCmdLine, "--start") == 0) {
    StartXavante();
    TrayIcon.SetIcon(IDI_XAVANTESTART);
    TrayIcon.SetMenuID(IDR_POPUP_CLOSE);
  }

  // Main message loop:
  while (GetMessage(&msg, NULL, 0, 0)) 
    {
      if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
	{
	  TranslateMessage(&msg);
	  DispatchMessage(&msg);
	}
    }

  return msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
  WNDCLASSEX wcex;

  wcex.cbSize = sizeof(WNDCLASSEX); 

  wcex.style			= CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc	= (WNDPROC)WndProc;
  wcex.cbClsExtra		= 0;
  wcex.cbWndExtra		= 0;
  wcex.hInstance		= hInstance;
  wcex.hIcon			= (HICON) LoadImage(hInstance, 
						    (LPCTSTR)IDI_XAVANTETRAY,
						    IMAGE_ICON, 
						    32, 32,
						    LR_DEFAULTCOLOR);
  wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
  wcex.lpszMenuName	= NULL;
  wcex.lpszClassName	= _T("XavanteWndClass");
  wcex.hIconSm		= (HICON) LoadImage(hInstance, 
					    (LPCTSTR)IDI_XAVANTETRAY,
					    IMAGE_ICON, 
					    16, 16,
					    LR_DEFAULTCOLOR);

  return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  HWND hWnd;

  hInst = hInstance; // Store instance handle in our global variable

  hWnd = CreateWindow(_T("XavanteWndClass"), _T("Xavante Launcher"), WS_OVERLAPPEDWINDOW,
		      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

  if (!hWnd)
    {
      return FALSE;
    }

  // Create the tray icon
  if (!TrayIcon.Create(hInstance,
		       hWnd,                            // Parent window
		       WM_ICON_NOTIFY,                  // Icon notify message to use
		       _T("Xavante Launcher - Right click for options"),  // tooltip
		       (HICON) LoadImage(hInstance, 
					 (LPCTSTR)IDI_XAVANTESTOP,
					 IMAGE_ICON, 
					 16, 16,
					 LR_DEFAULTCOLOR),
		       IDR_POPUP_MENU)) 
    return FALSE;

  //   ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  int wmId, wmEvent;

  currenthWnd = hWnd;
    
  switch (message) 
    {
    case WM_ICON_NOTIFY:
      return TrayIcon.OnTrayNotification(wParam, lParam);

    case WM_COMMAND:
      wmId    = LOWORD(wParam); 
      wmEvent = HIWORD(wParam); 
      // Parse the menu selections:
      switch (wmId)
	{
	case IDM_CYCLE:
	  if(xavante_started) {
	    StopXavante();
	    TrayIcon.SetIcon(IDI_XAVANTESTOP);
	    TrayIcon.SetMenuID(IDR_POPUP_MENU);
	  } else {
	    StartXavante();
	    TrayIcon.SetIcon(IDI_XAVANTESTART);
	    TrayIcon.SetMenuID(IDR_POPUP_CLOSE);
	  }
	  break;
	case IDM_ABOUT:
	  DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
	  break;
	  /*	case IDM_SHOWLOG:
	  if(xavante_log) {
	    execlp("cmd.exe", "cmd.exe", "/c", "start", "notepad.exe", 
		   "\"C:\\LuaRocks\\kepler\\log\\xavante.log\"", NULL);
	  } else MessageBox(hWnd, "Please start Xavante at least once.",
			  "Log not Found", MB_ICONERROR | MB_OK | MB_APPLMODAL);
			  break;*/
	case IDM_EXIT:
	  if(xavante_started)
	    StopXavante();
	  DestroyWindow(hWnd);
	  break;
	default:
	  return DefWindowProc(hWnd, message, wParam, lParam);
	}
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
  return 0;
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
    {
    case WM_INITDIALOG:
      return TRUE;

    case WM_COMMAND:
      if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
	{
	  EndDialog(hDlg, LOWORD(wParam));
	  return TRUE;
	}
      break;
    }
  return FALSE;
}
