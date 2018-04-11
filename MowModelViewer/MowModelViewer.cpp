//MowModelViewer.cpp: 定义应用程序的入口点。
//

#include "stdafx.h"
#include "MowModelViewer.h"
#include "systemclass.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此放置代码。

	SystemClass System;
	bool result;

	// Initialize and run the system object.
	result = System.Initialize();
	if (result)
	{
		System.Run();
	}

	// Shutdown and release the system object.
	System.Shutdown();

	return 0;
}
