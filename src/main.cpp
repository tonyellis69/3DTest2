

#define GLM_ENABLE_EXPERIMENTAL


#include "3DtestApp.h"


#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW 
#endif


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow) {
	//_CrtSetBreakAlloc(12920);
	//C3DtestApp* testApp = new C3DtestApp;
	C3DtestApp testApp;
	//AllocConsole();
	testApp.SetWindow(1200,800,"3D test app :-)");
	testApp.start();
	//delete testApp;

	//_CrtDumpMemoryLeaks();


	return true;
}


