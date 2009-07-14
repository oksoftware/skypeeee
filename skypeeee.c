#include <stdio.h>
#include <windows.h>

#include "strque.h"
#include "httpd.h"

//Skype control API's Constant
enum {
	SKYPECONTROLAPI_ATTACH_SUCCESS = 0,
	SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION,
	SKYPECONTROLAPI_ATTACH_REFUSED,
	SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE
};

//Global variables
UINT g_uSkypeControlAPIAttach;
UINT g_uSkypeControlAPIDiscover;
HWND g_hSkypeAPIWindow;
HWND g_hWnd;

//Read and write pointers of string que
STRQUE *pSkypeAPIMessageRead;
STRQUE *pSkypeAPIMessageWrite;

//Send command to Skype
LRESULT SendSkypeAPICommand(char *pCommand){
	COPYDATASTRUCT copyData;
	
	if(g_hSkypeAPIWindow == NULL){
		return 0;
	}
	
	copyData.dwData = 0;
	copyData.lpData = pCommand;
	copyData.cbData = strlen(pCommand) + 1;
	
	return SendMessage(g_hSkypeAPIWindow, WM_COPYDATA, (WPARAM)g_hWnd, (LPARAM)&copyData);
}

//Window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	char *pSkypeAPIMessageStr = NULL;
	
	switch(msg){
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		
		case WM_COPYDATA:
			pSkypeAPIMessageStr = (char *)malloc(((PCOPYDATASTRUCT)lParam)->cbData);
			if(pSkypeAPIMessageStr == NULL){
				fprintf(stderr, "Can not allocate memories.\n");
				break;
			}
			
			memcpy(pSkypeAPIMessageStr, ((PCOPYDATASTRUCT)lParam)->lpData, ((PCOPYDATASTRUCT)lParam)->cbData);
			
			pSkypeAPIMessageWrite = pushStrQue(pSkypeAPIMessageWrite, pSkypeAPIMessageStr);
			break;
		
		default:
			//If the message is g_uSkypeControlAPIAttach
			if(msg == g_uSkypeControlAPIAttach){
				switch(lParam){
					case SKYPECONTROLAPI_ATTACH_SUCCESS:
						fprintf(stderr, "Attached to Skype.\n");
						g_hSkypeAPIWindow = (HWND)wParam;
						
						//Starting HTTP server daemon
						StartHTTPServerDaemon();
						break;
					
					case SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION:
						fprintf(stderr, "Pending authorization.\n");
						break;
					
					case SKYPECONTROLAPI_ATTACH_REFUSED:
						fprintf(stderr, "Attach refused.\n");
						DestroyWindow(hWnd);
						break;
					
					case SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE:
						fprintf(stderr, "Skype API not available.\n");
						DestroyWindow(hWnd);
						break;
				}
			}else{
				return DefWindowProc(hWnd, msg, wParam, lParam);
			}
	}
	return 0;
}

//WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
	WNDCLASSEX wndClassEx;
	MSG msg;
	
	//Create a window class
	wndClassEx.cbSize = sizeof(WNDCLASSEX);
	wndClassEx.style = 0;
	wndClassEx.lpfnWndProc = WndProc;
	wndClassEx.cbClsExtra = 0;
	wndClassEx.cbWndExtra = 0;
	wndClassEx.hInstance = hInstance;
	wndClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClassEx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClassEx.lpszMenuName = NULL;
	wndClassEx.lpszClassName = "Skypeeee";
	wndClassEx.hIconSm = wndClassEx.hIcon;
	
	//Register the window class
	if(RegisterClassEx(&wndClassEx) == 0){
		fprintf(stderr, "Can not register the window class.\n");
		return FALSE;
	}
	
	//Create window
	g_hWnd = CreateWindow("Skypeeee", "Skypeeee", 0,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	if(g_hWnd == NULL){
		fprintf(stderr, "Can not create the window.\n");
		return FALSE;
	}
	
	//Create Skype API message que
	pSkypeAPIMessageRead = newStrQue();
	pSkypeAPIMessageWrite = pSkypeAPIMessageRead;
	
	//Register window message
	g_uSkypeControlAPIAttach = 0;
	g_uSkypeControlAPIDiscover = 0;
	
	g_uSkypeControlAPIAttach = RegisterWindowMessage("SkypeControlAPIAttach");
	g_uSkypeControlAPIDiscover = RegisterWindowMessage("SkypeControlAPIDiscover");
	
	if((g_uSkypeControlAPIAttach == 0) || (g_uSkypeControlAPIDiscover == 0)){
		fprintf(stderr, "Can not register window messages.\n");
		return FALSE;
	}
	
	//Send a message for discovering the Skype window
	g_hSkypeAPIWindow = NULL;
	
	if(SendMessage(HWND_BROADCAST, g_uSkypeControlAPIDiscover, (WPARAM)g_hWnd, 0) == 0){
		fprintf(stderr, "Can not send a message for discovering the Skype window.\n");
		return FALSE;
	}
	
	//Message loop
	while(GetMessage(&msg, NULL, 0, 0) != FALSE){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	DestroyWindow(g_hWnd);
	UnregisterClass(wndClassEx.lpszClassName, hInstance);
	
	return msg.wParam;
}
