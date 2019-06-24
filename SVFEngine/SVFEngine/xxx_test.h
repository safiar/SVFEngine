// ----------------------------------------------------------------------- //
//
// MODULE  : xxx_test.h
//
// PURPOSE : Выделенный модуль для написания тестовых задач
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2019)
//
// ----------------------------------------------------------------------- //

#ifndef _XXX_TEST_H
#define _XXX_TEST_H

//#define ACTIVATE_XXX_TEST // отключить обычную загрузку, начать выполнять тестовый модуль

#ifdef ACTIVATE_XXX_TEST
	
#include "stdafx.h"
#include "Framework.h"
#include "InputAnswersMain.h"

//#include "NetworkWS2.h"
#include <iostream>
#include "exceptions.h"
#include "thread.h"

namespace TESTING
{
	inline void EngineException_TESTFUNC_THROW() 
	{ __TRY__ 
		throw std::runtime_error("<TEST_RUNTIME_ERROR_MESSAGE>");
	__CATCH__ }
	inline void EngineException_TESTFUNC_3_end() 
	{ __TRY__ 
		EngineException_TESTFUNC_THROW();
	__CATCH__ }
	struct EngineException_TEST
	{		
		std::exception_ptr e_ptr { nullptr };

		~EngineException_TEST()
		{
			printf("\n~EngineException_TEST()");
		}
		void AAA ()
		{
			
			try 
			{
				EngineException_TESTFUNC_3_end();
			}
			catch (...)
			{
				printf("\nAAA! e_ptr");
				e_ptr = std::current_exception();
			} //*/
		}
	};	
	inline void EngineException_TESTFUNC_2()
	{
	__TRY__
		
		EngineException_TEST test;
		THREADDATA thr;

		thr(&EngineException_TEST::AAA, &test);
		_SLEEP(100);
		printf("\nAAA!");
		if (test.e_ptr) std::rethrow_exception(test.e_ptr);

		//test.AAA();
		//EngineException_TESTFUNC_3_end();

	__CATCH__ 
	}
	inline void EngineException_TESTFUNC_1()
	{ __TRY__ 
		EngineException_TESTFUNC_2();
	__CATCH__ }
	inline void EngineException_TESTFUNC_start()
	{ __TRY__ 
		EngineException_TESTFUNC_1(); 
	__CATCH__ }
}

inline void _XXX_TEST_SOUND_ENGINE()
{
	CSoundT sound;
	sound.Init();
	//sound.SetVolumeMaster(0.75f);
	//sound.__MixerWavTest();

	uint32 eRunSet = eSoundRunAsync | eSoundRunMix;  //eSoundRunAsync | eSoundRunStopAtEnd;
	uint64 GID[100];
	
	//sound.Load(DIRECTORY_GAME, L"fire.wav", GID);
	//sound.Load(DIRECTORY_GAME, L"a-ha - Take On Me.mp3", 3, GID[0]);
	//sound.Load(GID[0], 3, GID[1]);
	//sound.Load(GID[0], 7, GID[2]);
	//sound.Load(GID[1], 3, GID[3]);

	int N = 1;
	for (int i = 0; i < N; i++)
	{
		sound.Load(DIRECTORY_GAME, L"rc_loop.mp3", GID[i*2 + 0]);
		sound.Load(DIRECTORY_GAME, L"kclick.wav", GID[i*2 + 1]);
	}
//	for (int i = 0; i < N; i++)
//	{
//		sound.RunThreadPlay(GID[i*2 + 0], eRunSet);
//		sound.RunThreadPlay(GID[i*2 + 1], eRunSet);
//	}
//	for (int i = 0; i < N * 2; i++)
//	{
//		sound.RunThreadPlay(GID[i], eRunSet);
//		printf("\n\n --------- %i --------- \n\n", i+1);
//		getchar();
//	}

/*	
	sound.RunThreadPlay(GID[0], eSoundRunAsync | eSoundRunLoop);
	_SLEEP(100);
	sound.RunThreadPlay(GID[1], eSoundRunAsync);
	printf("\nRUNNED");
	uint32 thr, DBG, mix;
	thr = DBG = mix = 0;
	for (;;)
	{
		uint32 thr_cur, DBG_cur, mix_cur;
		sound.GetNumThreadsInUse(thr_cur);
		sound.GetNumTasksInMixer(mix_cur);
		sound._GetNumThreadsInUse_alt(DBG_cur);
		if (thr != thr_cur || DBG != DBG_cur || mix != mix_cur)
		{
			thr = thr_cur; DBG = DBG_cur; mix = mix_cur;
			printf("\nThr %2i (%2i) Mix %2i", thr_cur, DBG_cur, mix_cur);
		}
		_SLEEP(8);
		//getchar();
	}
	getchar();
	return; //*/


	//CSoundLoadList list;
	//list.info.Create(4);
	//list.gamepath = DIRECTORY_GAME;
	//for (int i = 0; i < 4; i++)
	//{
	//	list.info[i]->filename = L"a-ha - Take On Me.mp3";
	//	list.info[i]->GID.group = 3;
	//}
	//list.info[2]->GID.group = 7;
	//list.info[1]->linked_GID = & list.info[0]->GID;
	//list.info[2]->linked_GID = & list.info[0]->GID;
	//list.info[3]->linked_GID = & list.info[1]->GID;
	//sound.Load(list);
	//for (int i = 0; i < 4; i++)
	//	GID[i] = list.info[i]->GID;

	//getchar();
	//for (int i = 0; i < 4; i++)
	//{
	//	sound.RunThreadPlay(GID[i], eRunSet);
	//	getchar();
	//	sound.ControlInterrupt(GID[i], true);
	//}
	//printf("\n000");
	//getchar();
	//sound.DeleteOne(GID[0], eSoundDelete_simple);
	//getchar();
	//sound.RunThreadPlay(GID[1], eRunSet);
	//sound.DeleteGroup(7);
	//sound.DeleteGroup(3);

	//for (int i = 0; i < 60; i++)
	//	sound.RunThreadPlay(GID[0], eRunSet);
	printf("\nRUNNED");
	getchar();
	//sound.ControlPause(GID[3]);
	//getchar();
	//sound.ControlResume(GID[3]);
	//getchar();
	//sound.ControlRewind(GID[3], 100);
//	sound.DeleteGroup(3, eSoundDelete_simple);
//	sound.Load(DIRECTORY_GAME, L"a-ha - Take On Me.mp3", 3, GID[0]);
	//sound.ChangeGroup(GID[0], 7);
	//sound.SetVolumeGroup(3, 0.3f);

	eRunSet = eSoundRunAsync | eSoundRunStopAtEnd | eSoundRunMix;
	sound.RunThreadPlay(GID[0], eRunSet);

	vector <CSoundState*> state;
	uint32 num_tasks;

	//getchar();
	//sound.SetVolumeChannelsStereo(GID[0], 0.0f, 0.1f);

	getchar();
	{
		auto time = _TIME;
		sound.ControlPause(GID[0], true);
		printf("\nTIME %3i ms", _TIMER(time));
		printf(" (pause)");

		sound.GetStateSound(state, num_tasks, GID[0]);
		for (uint32 i = 0; i < num_tasks; i++)
		{
			printf("\n0x%016llX : time %f [%7.3f %%] auto %i user %i",
				state[i]->GID,
				state[i]->time_t, state[i]->time_p * 100,
				state[i]->auto_pause,
				state[i]->user_pause);
		}
	}

	getchar();
	{
		auto time = _TIME;
		sound.ControlRewind(GID[0], 1055.f, false, true);
		printf("\nTIME %3i ms", _TIMER(time));
		printf(" (rewind 150)");

		sound.GetStateSound(state, num_tasks, GID[0]);
		for (uint32 i = 0; i < num_tasks; i++)
		{
			printf("\n0x%016llX : time %f [%7.3f %%] auto %i user %i",
				state[i]->GID,
				state[i]->time_t, state[i]->time_p * 100,
				state[i]->auto_pause,
				state[i]->user_pause);
		}
	}

	getchar();
	{
		sound.GetStateSound(state, num_tasks, GID[0]);
		for (uint32 i = 0; i < num_tasks; i++)
		{
			printf("\n0x%016llX : time %f [%7.3f %%] auto %i user %i",
				state[i]->GID,
				state[i]->time_t, state[i]->time_p * 100,
				state[i]->auto_pause,
				state[i]->user_pause);
		}
	}

	getchar();
	{
		auto time = _TIME;
		sound.ControlResume(GID[0], true);
		printf("\nTIME %3i ms", _TIMER(time));
		printf(" (resume)");

		sound.GetStateSound(state, num_tasks, GID[0]);
		for (uint32 i = 0; i < num_tasks; i++)
		{
			printf("\n0x%016llX : time %f [%7.3f %%] auto %i user %i",
				state[i]->GID,
				state[i]->time_t, state[i]->time_p * 100,
				state[i]->auto_pause,
				state[i]->user_pause);
		}
	}


/*	eRunSet = eSoundRunAsync | eSoundRunMix;
	sound.RunThreadPlay(GID[0], eRunSet);
	printf("\n2");
	getchar();	
	eRunSet = eSoundRunMix;
	sound.RunThreadPlay(GID[1], eRunSet);
	printf("\n3");
	getchar();

	sound.ControlInterrupt(GID[0], false);
	getchar();

	sound.ControlInterrupt(GID[1], false);
	getchar();

	eRunSet = eSoundRunAsync | eSoundRunMix;
	sound.RunThreadPlay(GID[0], eRunSet);
	printf("\n2");
	getchar();
	eRunSet = eSoundRunMix;
	sound.RunThreadPlay(GID[1], eRunSet);
	printf("\n3");
	getchar(); //*/

	for (int i = 0; i < 10; i++)
	{
		//sound.ControlPause(SOUND_ALL, true);
		//sound.ControlResume(SOUND_ALL, false);
		//
		//sound.ControlRewind(0, 0, false, false);
		//sound.ControlRewind(0, 100, true, true);
		//sound.ControlResume(0, true);
		//
		//printf("\n%i ", i);
	}

	//sound.ControlPause(SOUND_ALL, true);
	//getchar();
	//printf("\n ___ Resume");
	//sound.ControlResume(SOUND_ALL);
	//getchar();
	//printf("\n ___ Next pause after Resume() ... ");
	//sound.ControlPause(SOUND_ALL, true);
	//printf("\n ___ Next pause after Resume() completed");
	////////////
	//getchar();
	//printf("\n ___ Resume 2 = Rewind");
	//sound.ControlRewind(SOUND_ALL, 100, false, false);
	//getchar();
	//printf("\n ___ Next pause after Rewind() ... ");
	//sound.ControlPause(SOUND_ALL, true);
	//printf("\n ___ Next pause after Rewind() completed");
	//getchar();
	//sound.ControlResume(SOUND_ALL);

	//uint64 task_id = sound.ControlTaskRewind(true, SOUND_ALL, 10000.f, true);
	//uint32 task_result = 0;
	//////sound.ControlTaskStatusWait(task_id);
	//while (sound.ControlTaskStatus(task_id, task_result) == false)
	//{
	//	printf("\n%i ", ++task_result);
	//}
	//printf("\nresult = %i 0x%X", (int)task_result, task_result);
	//sound.ControlResume(SOUND_ALL);

	//vector <CSoundState*> state;
	//uint32 threads_num;
	//for (;;)
	//{		
	//	_SLEEP(250);
	//	sound.GetStateSound(state, threads_num);
	//	uint32 exit = 0;
	//	for (uint32 i = 0; i < threads_num; i++)
	//	{
	//		printf("\n0x%016llX : time %f [%7.3f %%] auto %i user %i",
	//			state[i]->GID,
	//			state[i]->time_t, state[i]->time_p * 100,
	//			state[i]->auto_pause,
	//			state[i]->user_pause);
	//		if (state[i]->auto_pause) exit++;
	//	}
	//	if (exit == threds_num) break;
	//}

	printf("\n @@@@@@ \n");
	getchar();
	//printf("\nnum in pause  %i", sound.snd[0]->debug_num_in_pause);
	//printf("\ntotal players %i", sound.snd[0]->players);

	//sound.Close();

	/*
	for (int i = 0; i < 10; i++)
	{
		getchar();
		float left = 1.f - (float)i / 10;
		float right = 1.f;
		sound.SetVolumeChannelsMono(0, left);

		sound.GetVolumeChannelsStereo(0, left, right);
		printf("Volume L-R %f - %f", left, right);
	}
	//*/

	/*
	for (;;)
	{
		sound.PlayThread(0, true, false);
		sound.PlayThread(1, true, false);
		sound.PlayThread(2, true, false);
		sound.PlayThread(0, true, false);
		sound.PlayThread(1, true, false);
		sound.PlayThread(2, true, false);
		sound.ControlInterrupt(MISSING, true);
	} //*/

	/*
	getchar();
	for (;;)
	{
		std::thread(&CSound::ControlPause, &sound, 0).detach();
		std::thread(&CSound::ControlInterrupt, &sound, 0, true).detach();	
		//sound.ControlInterrupt(0, true);
		//sound.ControlPause(0);
		//sound.ControlResume(0);
		break;
	}
	//*/

	//sound.SaveSound(0, DIRECTORY_GAME \
	//	L"\\data\\sounds", L"__test.wav");
}

inline void _XXX_TEST_SOUND_VOLUME()
{
	CSoundT sound;
	sound.Init();

	//sound.__ConvertWavTestVOLUME();
}

inline void _XXX_TEST_NETWORK()
{
/*	CNetworkF network;
	char command[256];

	THREADPOOL		client;
	THREADPOOL		server;
	atomic<bool>	client_exit = false;
	atomic<bool>	server_exit = false;

	auto virtual_client = [&network, &client_exit] ()
	{
		for (;;)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			if (_ATM_LD(client_exit))
			{
				_ATM_0(client_exit);
				network.ClientDisconnect();
				break;
			}
			network.ClientListen(5);
		};
	};
	auto virtual_server = [&network, &server_exit] ()
	{
		for (;;)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			if (_ATM_LD(server_exit))
			{
				_ATM_0(server_exit);
				network.ServerShutdown();
				break;
			}
			network.ServerListen(5);
		};
	};

	std::cout << "Enter '-connect' for set server connection...\n\n";
	for (;;)
	{
		std::cin.getline(command, 256); // 127.0.0.1 ; 92.243.162.64 ; fe80::91e1:75d9:27e2:1517

			 if (Compare(command, "-exit",        5)) break;
		else if (Compare(command, "-wakeup",      7))
		{
			network.ServerWakeup("127.0.0.1", 0, 0); 
			server(virtual_server);
		}
		else if (Compare(command, "-shutdown", 9)) _ATM_1(server_exit);
		else if (Compare(command, "-connect",  8))
		{
			network.ClientConnect("127.0.0.1", 0);
			client(virtual_client);
		}
		else if (Compare(command, "-disconnect", 11)) _ATM_1(client_exit);
		else network.Send(command, strlen(command) + 1, nullptr);
	} //*/
}

inline void _XXX_TEST_EXCEPTIONS()
{
	//std::exception qwerty;
	//std::runtime_error qwerty(;

	//auto file = __FILE__ ;
	//auto line = __LINE__ ;
	//printf("\nFILE = %s\nLINE = %i\n", __FILE__, __LINE__);

	//std::uncaught_exception() ;

	std::exception_ptr eptr = nullptr; 

	try
	{
		TESTING::EngineException_TESTFUNC_start();
	}
	catch (...)
	{
		eptr = std::current_exception();
	}

	EngineExceptionHandle(eptr);

}

inline void _XXX_TEST_SOME()
{
//	CObject O;
//	float Q=0;
//
//	auto time = _TIME;
//	for (int i = 0; i < 10000000; i++)
//		O.WPosSetX(Q);
//	printf("\ntime = %i", _TIMER(time));
//
//	time = _TIME;
//	for (int i = 0; i < 10000000; i++)
//		O.WPosSetX_(Q);
//	printf("\ntime = %i", _TIMER(time));

/*	auto time = _TIME;	
	for (int i = 0; i < 5000000; i++)
	{
		ptr = VP[999].get();
	}
	printf("\ntime = %i", _TIMER(time)); //*/
}

inline void _XXX_TEST()
{
	CSystem::Init();
	//CSystem::Printf();

	_XXX_TEST_SOUND_ENGINE();
	//_XXX_TEST_SOUND_VOLUME();
	//_XXX_TEST_EXCEPTIONS();
}

#endif

#endif // _XXX_TEST_H