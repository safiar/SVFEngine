#include "stdafx.h"
#include "Framework.h"
#include "InputAnswersMain.h"

using namespace SAVFGAME;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	setlocale(LC_NUMERIC, "C");	// разделитель float '.'
	
	CTAB::Init(); // таблицы констант

	CFramework * framework = new CFramework;
	CRenderT *   render    = new CRenderT;

	if (framework->Init(render))
	{	
		CMainInputAnswers * answers = new CMainInputAnswers;
		answers->SetRender(render);

		framework->AddAnswer(answers);
		framework->Run();
		framework->Close();

		_DELETE(answers);
	}
	_DELETE(framework);
	
	CTAB::Close();

	return 0;
}

//#ifdef _DEBUG
int main()
{
	setlocale(LC_ALL,"russian");
	WinMain(GetModuleHandle(NULL), NULL, NULL, SW_SHOW);

	printf("\nProgram ending, press ENTER to close...");
	getchar();
	return 0;
}
//#endif
