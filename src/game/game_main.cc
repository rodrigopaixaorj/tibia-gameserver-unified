#include "common.hh"
#include "communication.hh"
#include "config.hh"
#include "houses.hh"
#include "info.hh"
#include "map.hh"
#include "magic.hh"
#include "moveuse.hh"
#include "objects.hh"
#include "operate.hh"
#include "query.hh"
#include "reader.hh"
#include "writer.hh"
#include "cr.hh"
#include "connections.hh"

#include <iostream>

static bool Reboot = false;
static bool SaveMapOn = false;

void LoadWorldConfig(void){
	TQueryManagerConnection Connection(KB(16));
	if(!Connection.isConnected()){
		error("LoadWorldConfig: Kann nicht zum Query-Manager verbinden.\n");
		throw "cannot connect to querymanager";
	}

	int HelpWorldType;
	int HelpGameAddress[4];
	int Ret = Connection.loadWorldConfig(&HelpWorldType, &RebootTime,
			HelpGameAddress, &GamePort,
			&MaxPlayers, &PremiumPlayerBuffer,
			&MaxNewbies, &PremiumNewbieBuffer);
	if(Ret != 0){
		error("LoadWorldConfig: Kann Konfigurationsdaten nicht holen.\n");
		throw "cannot load world config";
	}

	WorldType = (TWorldType)HelpWorldType;
	snprintf(GameAddress, sizeof(GameAddress), "%d.%d.%d.%d",
			HelpGameAddress[0], HelpGameAddress[1],
			HelpGameAddress[2], HelpGameAddress[3]);
}

void InitAll(void){
	try{
		std::cout << ":: Loading config... " << std::flush;
		ReadConfig();
		std::cout << "[done]\n";

		std::cout << ":: Loading world config... " << std::flush;
		LoadWorldConfig();
		std::cout << "[done]\n";

		std::cout << ":: Initializing shared memory... " << std::flush;
		InitSHM(false);
		std::cout << "[done]\n";

		srand((unsigned int)time(NULL));

		std::cout << ":: Initializing connections & network... " << std::flush;
		InitConnections();
		InitCommunication();
		std::cout << "[done]\n";

		std::cout << ":: Initializing database threads & queues... " << std::flush;
		InitStrings();
		InitWriter();
		InitReader();
		std::cout << "[done]\n";

		std::cout << ":: Loading objects and items... " << std::flush;
		InitObjects();
		std::cout << "[done]\n";

		std::cout << ":: Loading map sectors... " << std::flush;
		InitMap();
		std::cout << "[done]\n";

		std::cout << ":: Loading game mechanics... " << std::flush;
		InitInfo();
		InitMoveUse();
		std::cout << "[done]\n";

		std::cout << ":: Loading spells and magic... " << std::flush;
		InitMagic();
		std::cout << "[done]\n";

		std::cout << ":: Loading monsters and creatures... " << std::flush;
		InitCr();
		std::cout << "[done]\n";

		std::cout << ":: Loading houses... " << std::flush;
		InitHouses();
		std::cout << "[done]\n";

		std::cout << ":: Initializing world clock... " << std::flush;
		InitTime();
		std::cout << "[done]\n";

		std::cout << ":: Applying engine patches... " << std::flush;
		ApplyPatches();
		std::cout << "[done]\n";
	}catch(const char *str){
		std::cout << "[failed]\n";
		error("Initialisierungsfehler: %s\n", str);
		throw str;
	}
}

void ExitAll(void){
	EndGame();
	ExitTime();
	ExitCr();
	ExitMagic();
	ExitMoveUse();
	ExitInfo();
	ExitHouses();
	ExitMap(SaveMapOn);
	ExitObjects();
	ExitReader();
	ExitWriter();
	ExitStrings();
	ExitCommunication();
	ExitConnections();
	ExitSHM();
}

static void ProcessCommand(void){
	int Command = GetCommand();
	if(Command != 0){
		char *Buffer = GetCommandBuffer();
		if(Command == 1){
			if(Buffer != NULL){
				BroadcastMessage(TALK_ADMIN_MESSAGE, "%s", Buffer);
			}else{
				error("ProcessCommand: Text für Broadcast ist NULL.\n");
			}
		}else{
			error("ProcessCommand: Unbekanntes Kommando %d.\n", Command);
		}

		SetCommand(0, NULL);
	}
}

void AdvanceGame(int Delay){
	static int CreatureTimeCounter = 0;
	static int CronTimeCounter = 0;
	static int SkillTimeCounter = 0;
	static int OtherTimeCounter = 0;
	static int OldAmbiente = -1;
	static uint32 NextMinute = 30;
	static bool Lag = false;

	CreatureTimeCounter += Delay;
	CronTimeCounter += Delay;
	SkillTimeCounter += Delay;
	OtherTimeCounter += Delay;

	if(CreatureTimeCounter >= 1750){
		CreatureTimeCounter -= 1000;
		ProcessCreatures();
	}

	if(CronTimeCounter >= 1500){
		CronTimeCounter -= 1000;
		ProcessCronSystem();
	}

	if(SkillTimeCounter >= 1250){
		SkillTimeCounter -= 1000;
		ProcessSkills();
	}

	if(OtherTimeCounter >= 1000){
		OtherTimeCounter -= 1000;

		RoundNr += 1;
		SetRoundNr(RoundNr);

		ProcessConnections();
		ProcessMonsterhomes();
		ProcessMonsterRaids();
		ProcessCommunicationControl();
		ProcessReaderThreadReplies(RefreshSector, SendMails);
		ProcessWriterThreadReplies();
		ProcessCommand();

		int Brightness, Color;
		GetAmbiente(&Brightness, &Color);
		if(OldAmbiente != Brightness){
			OldAmbiente = Brightness;
			TConnection *Connection = GetFirstConnection();
			while(Connection != NULL){
				if(Connection->Live()){
					SendAmbiente(Connection);
				}
				Connection = GetNextConnection();
			}
		}

		if(RoundNr % 10 == 0){
			NetLoadCheck();
		}

		if(RoundNr >= NextMinute){
			int Hour, Minute;
			GetRealTime(&Hour, &Minute);

			RefreshCylinders();
			if(Minute % 5 == 0){
				CreatePlayerList(true);
			}
			if(Minute % 15 == 0){
				SavePlayerDataOrder();
			}
			if(Minute == 0){
				NetLoadSummary();
			}
			if(Minute == 55){
				WriteKillStatistics();
			}

			int RealTime = Minute + Hour * 60;
			if(RebootTime >= 0 && RebootTime < 1440){
				if((RealTime + 5) % 1440 == RebootTime){
					if(Reboot){
						BroadcastMessage(TALK_ADMIN_MESSAGE,
							"Server is saving game in 5 minutes.\nPlease come back in 10 minutes.");
					}else{
						BroadcastMessage(TALK_ADMIN_MESSAGE,
							"Server is going down in 5 minutes.\nPlease log out.");
					}
					CloseGame();
				}else if((RealTime + 3) % 1440 == RebootTime){
					if(Reboot){
						BroadcastMessage(TALK_ADMIN_MESSAGE,
							"Server is saving game in 3 minutes.\nPlease come back in 10 minutes.");
					}else{
						BroadcastMessage(TALK_ADMIN_MESSAGE,
							"Server is going down in 3 minutes.\nPlease log out.");
					}
				}else if((RealTime + 1) % 1440 == RebootTime){
					if(Reboot){
						BroadcastMessage(TALK_ADMIN_MESSAGE,
							"Server is saving game in one minute.\nPlease log out.");
					}else{
						BroadcastMessage(TALK_ADMIN_MESSAGE,
							"Server is going down in one minute.\nPlease log out.");
					}
				}else if(RealTime == RebootTime){
					CloseGame();
					LogoutAllPlayers();
					SendAll();
					if(Reboot){
						RefreshMap();
					}
					SaveMap();
					SaveMapOn = false;
					EndGame();
				}
			}

			NextMinute = GetRoundForNextMinute();
		}
		CleanupDynamicStrings();
	}

	if(Delay > Beat){
		Log("lag", "Verzögerung %d msec.\n", Delay);
	}

	if(Delay < 1000){
		MoveCreatures(Delay);
		Lag = false;
	}else{
		if(!Lag && RoundNr > 10){
			error("AdvanceGame: Keine Kreaturbewegung wegen Lag (Verzögerung: %d msec).\n", Delay);
		}
		Lag = true;
	}

	SendAll();
}
