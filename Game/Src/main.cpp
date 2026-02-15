#include<Windows.h>

#if _DEBUG
#define _CRTDBG_MAP_ALLOC 
#include <crtdbg.h>
#endif // _DEBUG

/*
* テスト用の処理です
*/

#include<System/Service/ServiceLocator.hpp>
#include<System/Engine/Engine.hpp>
#include<System/Engine/EngineContext.hpp>

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	using namespace Ecse;

	System::Engine::Create();
	auto engine = System::ServiceLocator::Get<System::Engine>();
	if (engine == nullptr)
	{
		return -1;
	}
	System::EngineContext context;
	if (engine->Initialize(context) == false)
	{
		return -2;
	}

	while (engine->Run())
	{

	}

	engine->Shutdown();

	return 0;
}