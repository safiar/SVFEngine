#include <stdexcept>

#include "stdafx.h"
#include "Framework.h"
#include "InputAnswersMain.h"
#include "xxx_test.h"

#if _OS_WINDOWS
	//>> Точка входа в движок для OS Windows
	int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
	{
		bool success_run = false;

		try
		{
			setlocale(LC_NUMERIC, "C"); // разделитель float '.'
			
			CFramework *        framework   = nullptr;
			CRenderT *          render      = nullptr;
			CMainInputAnswers * answers     = nullptr;

			uint32 syserr = CSystem::Init(); // первичная информация о системе
	
			if (syserr == SYS_NO_ERRORS)
			{
				CTAB::Init(); // таблицы констант

				framework = new CFramework;
				render    = new CRenderT;

				if (framework != nullptr && render != nullptr)
				{
					if (framework->Init(render))
					{
						answers = new CMainInputAnswers;

						if (answers != nullptr)
						{
							success_run = true;

							answers->Init(&render->world);

							framework->AddAnswer(answers);
							framework->Run();
							framework->Close();
						}
					}
					else
					{
						printf("\nFramework init failed");
					}
				}	
			}

			if (!success_run)
			{
				if (syserr != SYS_NO_ERRORS) CSystem::Printf(syserr);

				if (framework == nullptr)
					printf("\nAllocation memory failed : framework");
				if (render == nullptr)
					printf("\nAllocation memory failed : render");
				if (answers == nullptr)
					printf("\nAllocation memory failed : i/o answers");

				printf("\n\nENGINE RUN ABORTED\n");	
			}

			_DELETE(answers);
			_DELETE(render);
			_DELETE(framework);	
			CSystem::Release();
		}
		catch (std::exception & e)
		{
			printf("\nAn exception occurred : %s", e.what());
		}

		return success_run;
	}
#endif

int main()
{	
	setlocale(LC_ALL, "russian");

#ifndef ACTIVATE_XXX_TEST
	#if _OS_WINDOWS
		WinMain(GetModuleHandle(NULL), NULL, NULL, SW_SHOW);
		setlocale(LC_NUMERIC, "C");	
	#endif
#else
	setlocale(LC_NUMERIC, "C");
	_XXX_TEST();
#endif

	printf("\nProgram ending, press ENTER to close...");
	getchar();
	return 0;
}
