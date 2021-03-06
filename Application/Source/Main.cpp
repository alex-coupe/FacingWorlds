#include <Windows.h>
#include "Application.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	
	//Create Editor On The Heap
	std::shared_ptr<Application> application = std::make_shared<Application>();

	//Initialize Engine Systems
	application->Init(hInstance);

	//Enter Main App Loop
	while (application->Update());

	return 0;
}