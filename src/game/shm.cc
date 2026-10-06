#include "common.hh"
#include "config.hh"
#include "enums.hh"
#include "threads.hh"
#include "writer.hh"

struct TSharedMemory {
	int Command;
	char CommandBuffer[256];
	uint32 RoundNr;
	uint32 ObjectCounter;
	uint32 Errors;
	int PlayersOnline;
	int NewbiesOnline;
	int PrintBufferPosition;
	char PrintBuffer[200][128];
	GAMESTATE GameState;
};

static TSharedMemory g_SHMInstance = {};
static TSharedMemory *SHM = &g_SHMInstance;
static bool VerboseOutput = false;

void StartGame(void){
	if(SHM != NULL){
		if(SHM->GameState == GAME_STARTING){
			SHM->GameState = GAME_RUNNING;
		}
	}
}

void CloseGame(void){
	if(SHM != NULL){
		if(SHM->GameState == GAME_RUNNING){
			SHM->GameState = GAME_CLOSING;
		}
	}
}

void EndGame(void){
	if(SHM != NULL){
		SHM->GameState = GAME_ENDING;
	}
}

bool LoginAllowed(void){
	if(SHM != NULL){
		return SHM->GameState == GAME_RUNNING;
	}
	return false;
}

bool GameRunning(void){
	if(SHM != NULL){
		return SHM->GameState == GAME_STARTING
			|| SHM->GameState == GAME_RUNNING
			|| SHM->GameState == GAME_CLOSING;
	}
	return false;
}

bool GameStarting(void){
	if(SHM != NULL){
		return SHM->GameState == GAME_STARTING;
	}
	return false;
}

bool GameEnding(void){
	if(SHM != NULL){
		return SHM->GameState == GAME_CLOSING
			|| SHM->GameState == GAME_ENDING;
	}
	return false;
}

static void ErrorHandler(const char *Text){
	if(VerboseOutput){
		printf("%s", Text);
	}

	if(SHM != NULL){
		SHM->Errors += 1;
		if(SHM->Errors <= 0x8000){
			Log("error", "%s", Text);
			if(SHM->Errors == 0x8000){
				Log("error", "Zu viele Fehler. Keine weitere Protokollierung.\n");
			}
		}
	}
}

static void PrintHandler(int Level, const char *Text){
	static Semaphore LogfileMutex(1);

	if(Level > DebugLevel){
		return;
	}

	if(VerboseOutput){
		printf("%s", Text);
	}

	if(SHM != NULL){
		LogfileMutex.down();
		int CurrentLine = SHM->PrintBufferPosition;
		strncpy(SHM->PrintBuffer[CurrentLine], Text, sizeof(SHM->PrintBuffer[CurrentLine]));
		SHM->PrintBuffer[CurrentLine][sizeof(SHM->PrintBuffer[CurrentLine]) - 1] = 0;
		SHM->PrintBufferPosition = (CurrentLine + 1) % NARRAY(SHM->PrintBuffer);
		LogfileMutex.up();
	}
}

int GetPrintlogPosition(void){
	int Position = 0;
	if(SHM != NULL){
		Position = SHM->PrintBufferPosition;
	}
	return Position;
}

char *GetPrintlogLine(int Line){
	char *Text = NULL;
	if(SHM != NULL){
		if(Line >= 0 && Line < NARRAY(SHM->PrintBuffer)){
			Text = SHM->PrintBuffer[Line];
		}
	}
	return Text;
}

void IncrementObjectCounter(void){
	if(SHM != NULL){
		SHM->ObjectCounter += 1;
	}
}

void DecrementObjectCounter(void){
	if(SHM != NULL){
		SHM->ObjectCounter -= 1;
	}
}

uint32 GetObjectCounter(void){
	uint32 Counter = 0;
	if(SHM != NULL){
		Counter = SHM->ObjectCounter;
	}
	return Counter;
}

void IncrementPlayersOnline(void){
	if(SHM != NULL){
		SHM->PlayersOnline += 1;
	}
}

void DecrementPlayersOnline(void){
	if(SHM != NULL){
		SHM->PlayersOnline -= 1;
	}
}

int GetPlayersOnline(void){
	int PlayersOnline = 0;
	if(SHM != NULL){
		PlayersOnline = SHM->PlayersOnline;
	}
	return PlayersOnline;
}

void IncrementNewbiesOnline(void){
	if(SHM != NULL){
		SHM->NewbiesOnline += 1;
	}
}

void DecrementNewbiesOnline(void){
	if(SHM != NULL){
		SHM->NewbiesOnline -= 1;
	}
}

int GetNewbiesOnline(void){
	int NewbiesOnline = 0;
	if(SHM != NULL){
		NewbiesOnline = SHM->NewbiesOnline;
	}
	return NewbiesOnline;
}

void SetRoundNr(uint32 RoundNr){
	if(SHM != NULL){
		SHM->RoundNr = RoundNr;
	}
}

uint32 GetRoundNr(void){
	uint32 RoundNr = 0;
	if(SHM != NULL){
		RoundNr = SHM->RoundNr;
	}
	return RoundNr;
}

void SetCommand(int Command, char *Text){
	if(SHM != NULL){
		SHM->Command = Command;
		if(Text != NULL){
			strncpy(SHM->CommandBuffer, Text, sizeof(SHM->CommandBuffer));
			SHM->CommandBuffer[sizeof(SHM->CommandBuffer) - 1] = 0;
		}else{
			SHM->CommandBuffer[0] = 0;
		}
	}
}

int GetCommand(void){
	int Command = 0;
	if(SHM != NULL){
		Command = SHM->Command;
	}
	return Command;
}

char *GetCommandBuffer(void){
	char *Buffer = NULL;
	if(SHM != NULL){
		Buffer = SHM->CommandBuffer;
	}
	return Buffer;
}

void InitSHM(bool Verbose){
	VerboseOutput = Verbose;
	SHM = &g_SHMInstance;
	memset(SHM, 0, sizeof(TSharedMemory));

	SetErrorFunction(ErrorHandler);
	SetPrintFunction(PrintHandler);

	strncpy(SHM->PrintBuffer[0],
			"SHM initialized. System printing is working!\n",
			sizeof(SHM->PrintBuffer[0]));
	SHM->PrintBuffer[0][sizeof(SHM->PrintBuffer[0]) - 1] = 0;

	SHM->PrintBufferPosition = 1;
	SHM->GameState = GAME_STARTING;
}

pid_t GetGameProcessID(void){
	return (pid_t)getpid();
}

pid_t GetGameThreadID(void){
	return (pid_t)gettid();
}

void ExitSHM(void){
	SetErrorFunction(NULL);
	SetPrintFunction(NULL);
}
