#include "cr.hh"
#include "config.hh"
#include "houses.hh"
#include "info.hh"
#include "moveuse.hh"
#include "operate.hh"
#include "query.hh"
#include "threads.hh"
#include "writer.hh"

static Semaphore PlayerMutex(1);
static vector<TPlayer*> PlayerList(0, 100, 10, NULL);
static int FirstFreePlayer;

static Semaphore PlayerDataPoolMutex(1);
static TPlayerData PlayerDataPool[2000];

static TPlayerIndexInternalNode PlayerIndexHead;
static store<TPlayerIndexInternalNode, 100> PlayerIndexInternalNodes;
static store<TPlayerIndexLeafNode, 100> PlayerIndexLeafNodes;

// TPlayer
// =============================================================================
TPlayer::TPlayer(TConnection *Connection, uint32 CharacterID):
		TCreature(),
		AttackedPlayers(0, 10, 10),
		FormerAttackedPlayers(0, 10, 10)
{
	this->Type = PLAYER;
	this->LoseInventory = LOSE_INVENTORY_SOME;
	this->Profession = PROFESSION_NONE;
	this->AccountID = 0;
	this->Guild[0] = 0;
	this->Rank[0] = 0;
	this->Title[0] = 0;
	this->IPAddress[0] = 0;
	this->Depot = NONE;
	this->DepotNr = 0;
	this->DepotSpace = 0;
	this->ConstructError = NOERROR;
	this->PlayerData = NULL;
	this->TradeObject = NONE;
	this->TradePartner = 0;
	this->TradeAccepted = false;
	this->OldState = 0;
	this->Request = 0;
	this->RequestTimestamp = 0;
	this->RequestProcessingGamemaster = 0;
	this->TutorActivities = 0;
	this->NumberOfAttackedPlayers = 0;
	this->Aggressor = false;
	this->NumberOfFormerAttackedPlayers = 0;
	this->FormerAggressor = false;
	this->FormerLogoutRound = 0;
	this->PartyLeader = 0;
	this->PartyLeavingRound = 0;
	this->TalkBufferFullTime = 0;
	this->MutingEndRound = 0;
	this->NumberOfMutings = 0;

	memset(this->Rights, 0, sizeof(this->Rights));

	for(int SpellNr = 0;
			SpellNr < NARRAY(this->SpellList);
			SpellNr += 1){
		this->SpellList[SpellNr] = 0;
	}

	for(int QuestNr = 0;
			QuestNr < NARRAY(this->QuestValues);
			QuestNr += 1){
		this->QuestValues[QuestNr] = 0;
	}

	for(int ContainerNr = 0;
			ContainerNr < NARRAY(this->OpenContainer);
			ContainerNr += 1){
		this->OpenContainer[ContainerNr] = NONE;
	}

	STATIC_ASSERT(NARRAY(this->Addressees) == NARRAY(this->AddresseesTimes));
	for(int AddresseeNr = 0;
			AddresseeNr < NARRAY(this->Addressees);
			AddresseeNr += 1){
		this->Addressees[AddresseeNr] = 0;
		this->AddresseesTimes[AddresseeNr] = 0;
	}

	TPlayerData *PlayerData = AttachPlayerPoolSlot(CharacterID, false);
	if(PlayerData == NULL){
		error("TPlayer::TPlayer: PlayerData-Slot nicht gefunden.\n");
		this->ConstructError = ERROR;
		return;
	}

	SendMails(PlayerData);
	this->PlayerData = PlayerData;
	this->Connection = Connection;
	strcpy(this->Name, PlayerData->Name);

	// TODO(fusion): There is a try..catch block somewhere in here that also sets
	// `this->ConstructorError` but I couldn't figure its scope. It could be the
	// whole function from this point.
	//	I couldn't find any functions here with unhandled exceptions so we might
	// want to keep an eye out.
	this->SetID(CharacterID);
	InsertPlayerIndex(&PlayerIndexHead, 0, this->Name, CharacterID);
	this->LoadData();
	this->AccountID = PlayerData->AccountID;
	strcpy(this->IPAddress, Connection->GetIPAddress());

	STATIC_ASSERT(sizeof(this->Rights) == sizeof(PlayerData->Rights));
	memcpy(this->Rights, PlayerData->Rights, sizeof(this->Rights));

	this->Sex = PlayerData->Sex;
	strcpy(this->Guild, PlayerData->Guild);
	strcpy(this->Rank, PlayerData->Rank);
	strcpy(this->Title, PlayerData->Title);

	if(PlayerData->PlayerkillerEnd < (int)time(NULL)){
		PlayerData->PlayerkillerEnd = 0;
	}

	this->CheckOutfit();
	this->SetInList();

	// NOTE(fusion): Soul regen. Could be some inlined function `CheckSoul`.
	{
		TSkill *Soul = this->Skills[SKILL_SOUL];
		Soul->Max = (this->GetActivePromotion() ? 200 : 100);
		Soul->Check();

		// TODO(fusion): We're rounding up to an extra regen cycle here but I'm
		// not sure it is correct.
		int Timer = Soul->TimerValue();
		if(Timer >= 15){
			int Interval = (this->GetActivePromotion() ? 15 : 120);
			int Cycle = (Timer + Interval - 1) / Interval;
			int Count = (Timer + Interval - 1) % Interval;
			this->SetTimer(SKILL_SOUL, Cycle, Count, Interval, -1);
		}
	}

	// TODO(fusion): Handle login after death?
	if(this->Skills[SKILL_HITPOINTS]->Get() <= 0){
		for(int SkillNr = 0;
				SkillNr < NARRAY(this->Skills);
				SkillNr += 1){
			this->DelTimer(SkillNr);
			this->Skills[SkillNr]->SetMDAct(0);
			this->Skills[SkillNr]->DAct = 0;
		}

		this->Skills[SKILL_HITPOINTS]->SetMax();
		this->Skills[SKILL_MANA     ]->SetMax();
		this->Outfit = this->OrgOutfit;
		this->posx = this->startx;
		this->posy = this->starty;
		this->posz = this->startz;
	}

	if(PlayerData->LastLoginTime == 0 && CheckRight(CharacterID, GAMEMASTER_OUTFIT)){
		Log("game", "Gamemaster-Charakter %s loggt zum ersten Mal ein -> Level 2 setzen.\n", this->Name);
		this->Skills[SKILL_LEVEL]->Act = 2;
		this->Skills[SKILL_LEVEL]->Exp = 100;
		this->Skills[SKILL_LEVEL]->LastLevel = 100;
		this->Skills[SKILL_LEVEL]->NextLevel = 200;
	}

	if(CoordinateFlag(this->posx, this->posy, this->posz, BED)){
		this->Regenerate();
	}

	uint16 HouseID = GetHouseID(this->posx, this->posy, this->posz);
	if(HouseID != 0
			&& !IsInvited(HouseID, this, PlayerData->LastLogoutTime)
			&& !CheckRight(CharacterID, ENTER_HOUSES)){
		GetExitPosition(HouseID, &this->posx, &this->posy, &this->posz);
	}

	if(!CheckRight(CharacterID, PREMIUM_ACCOUNT)){
		if(IsPremiumArea(this->startx, this->starty, this->startz)){
			Log("game", "Spieler %s wird aus PayArea-Stadt ausgebürgert und erhält neue Heimatstadt.\n", this->Name);
			GetStartPosition(&this->startx, &this->starty, &this->startz, (this->Profession == PROFESSION_NONE));
		}

		if(IsPremiumArea(this->posx, this->posy, this->posz)){
			Log("game", "Spieler %s wird aus PayArea geworfen und in seine Heimatstadt gesetzt.\n", this->Name);
			this->posx = this->startx;
			this->posy = this->starty;
			this->posz = this->startz;
		}
	}else{
		Log("game", "Spieler besitzt Premium Account.\n");
	}

	this->SetOnMap();
	Connection->EnterGame();
	SendInitGame(Connection, CharacterID);
	SendRights(Connection);
	SendFullScreen(Connection);
	GraphicalEffect(this->CrObject, EFFECT_ENERGY);
	this->LoadInventory(PlayerData->LastLoginTime == 0);
	this->NotifyChangeInventory();
	SendAmbiente(Connection);
	AnnounceChangedCreature(CharacterID, CREATURE_LIGHT_CHANGED);
	SendPlayerSkills(Connection);
	this->SyncState();
	this->SendBuddies();

	if(PlayerData->LastLoginTime != 0){
		char TimeString[100];
		struct tm LastLogin = GetLocalTimeTM(PlayerData->LastLoginTime);
		strftime(TimeString, sizeof(TimeString), "%d. %b %Y %X %Z", &LastLogin);
		SendMessage(Connection, TALK_LOGIN_MESSAGE,
				"Your last visit in Tibia: %s.", TimeString);
	}else if(!CheckRight(this->ID, GAMEMASTER_OUTFIT)){
		Log("game", "Spieler %s loggt zum ersten Mal ein -> Outfitwahl.\n", this->Name);
		SendMessage(Connection, TALK_LOGIN_MESSAGE,
				"Welcome to Tibia! Please choose your outfit.");
		SendOutfit(Connection);
	}

	PlayerData->LastLoginTime = time(NULL);
}

TPlayer::~TPlayer(void){
	LogoutOrder(this);
	if(this->ConstructError != NOERROR){
		this->DelInList();
		return;
	}

	ASSERT(this->PlayerData);
	TPlayerData *PlayerData = this->PlayerData;
	PlayerData->LastLogoutTime = time(NULL);
	this->SaveData();

	if(!this->IsDead){
		Log("game", "Spieler %s loggt aus.\n", this->Name);
		GraphicalEffect(this->posx, this->posy, this->posz, EFFECT_POFF);
		this->SaveInventory();
	}else{
		Log("game", "Spieler %s ist gestorben.\n", this->Name);

		// NOTE(fusion): This is a disaster. We're deleting inventory data here
		// so `~TCreature` can handle dropping loot and then re-generate it with
		// `SaveInventory` if `LoseInventory` is not `LOSE_INVENTORY_ALL`, which
		// makes sense but is poorly executed.
		delete[] PlayerData->Inventory;
		PlayerData->Inventory = NULL;

		bool ResetCharacter = false;
		if(this->Profession != PROFESSION_NONE && this->Skills[SKILL_LEVEL]->Get() <= 5){
			Log("game", "Setze Spieler %s komplett zurück wegen Level.\n", this->Name);
			ResetCharacter = true;
		}else if(this->Skills[SKILL_HITPOINTS]->Max <= 0){
			Log("game", "Setze Spieler %s komplett zurück wegen MaxHitpoints.\n", this->Name);
			ResetCharacter = true;
		}

		if(ResetCharacter){
			PlayerData->CurrentOutfit = PlayerData->OriginalOutfit;
			PlayerData->startx = 0;
			PlayerData->starty = 0;
			PlayerData->startz = 0;
			PlayerData->posx = 0;
			PlayerData->posy = 0;
			PlayerData->posz = 0;
			PlayerData->Profession = PROFESSION_NONE;

			for(int SpellNr = 0;
					SpellNr < NARRAY(PlayerData->SpellList);
					SpellNr += 1){
				PlayerData->SpellList[SpellNr] = 0;
			}

			for(int QuestNr = 0;
					QuestNr < NARRAY(PlayerData->QuestValues);
					QuestNr += 1){
				PlayerData->QuestValues[QuestNr] = 0;
			}

			// NOTE(fusion): This is used to reset skills back to default. See
			// `TPlayer::LoadData`.
			for(int SkillNr = 0;
					SkillNr < NARRAY(PlayerData->Minimum);
					SkillNr += 1){
				PlayerData->Minimum[SkillNr] = INT_MIN;
			}

			this->LoseInventory = LOSE_INVENTORY_ALL;
		}

		if(PlayerData->PlayerkillerEnd != 0){
			this->LoseInventory = LOSE_INVENTORY_ALL;
		}

		if(CheckRight(this->ID, KEEP_INVENTORY)){
			this->LoseInventory = LOSE_INVENTORY_NONE;
		}
	}

	if(CheckRight(this->ID, READ_GAMEMASTER_CHANNEL)){
		CloseProcessedRequests(this->ID);
	}

	this->ClearRequest();
	LeaveAllChannels(this->ID);

	if(this->GetPartyLeader(false) != 0){
		::LeaveParty(this->ID, true);
	}

	this->ClearPlayerkillingMarks();
	this->DelInList();

	PlayerData->Dirty = true;
	// TODO(fusion): Something is telling me that `PlayerData->Sticky` is also poorly managed.
	DecreasePlayerPoolSlotSticky(PlayerData);
	ReleasePlayerPoolSlot(PlayerData);
}

void TPlayer::Death(void){
	if(CheckRight(this->ID, INVULNERABLE)){
		error("TPlayer::Death: Aha, so geht das aber nicht!! Goetter kann man nicht toeten!!\n");
		this->Skills[SKILL_HITPOINTS]->SetMax();
		return;
	}

	if(this->Connection != NULL){
		SendPlayerData(this->Connection);
		SendMessage(this->Connection, TALK_EVENT_MESSAGE, "You are dead.\n");
		this->Connection->Die();
	}

	TCreature::Death();

	if(WorldType == PVP_ENFORCED){
		this->Combat.DistributeExperiencePoints(this->Skills[SKILL_LEVEL]->Exp / 20);
	}

	// TODO(fusion): Probably related to blessings?
	int LossPercent = (this->GetActivePromotion() ? 7 : 10);
	for(int QuestNr = 101; QuestNr <= 105; QuestNr += 1){
		if(this->GetQuestValue(QuestNr) != 0){
			this->SetQuestValue(QuestNr, 0);
			LossPercent -= 1;
		}
	}

	this->Skills[SKILL_LEVEL      ]->DecreasePercent(LossPercent);
	this->Skills[SKILL_MAGIC_LEVEL]->DecreasePercent(LossPercent);
	this->Skills[SKILL_SHIELDING  ]->DecreasePercent(LossPercent);
	this->Skills[SKILL_DISTANCE   ]->DecreasePercent(LossPercent);
	this->Skills[SKILL_SWORD      ]->DecreasePercent(LossPercent);
	this->Skills[SKILL_CLUB       ]->DecreasePercent(LossPercent);
	this->Skills[SKILL_AXE        ]->DecreasePercent(LossPercent);
	this->Skills[SKILL_FIST       ]->DecreasePercent(LossPercent);
	this->Skills[SKILL_FISHING    ]->DecreasePercent(LossPercent);
}

bool TPlayer::MovePossible(int x, int y, int z, bool Execute, bool Jump){
	bool Result = TCreature::MovePossible(x, y, z, Execute, Jump);
	if(Result && Execute){
		if(this->EarliestProtectionZoneRound > RoundNr
				&& IsProtectionZone(x, y, z)
				&& !IsProtectionZone(this->posx, this->posy, this->posz)){
			throw ENTERPROTECTIONZONE;
		}

		uint16 HouseID = GetHouseID(x, y, z);
		if(HouseID != 0
				&& !IsInvited(HouseID, this, INT_MAX)
				&& !CheckRight(this->ID, ENTER_HOUSES)){
			throw NOTINVITED;
		}
	}
	return Result;
}

void TPlayer::DamageStimulus(uint32 AttackerID, int Damage, int DamageType){
	if(!this->IsDead){
		this->BlockLogout(60, false);
	}
}

void TPlayer::IdleStimulus(void){
	if(this->Combat.AttackDest != 0){
		try{
			this->ToDoAttack();
			this->ToDoStart();
		}catch(RESULT r){
			this->ToDoClear();
			if(r != NOERROR){
				if(r != NOWAY){
					SendResult(this->Connection, r);
				}

				this->ToDoWait(1000);
				this->ToDoStart();
			}
		}
	}
}

void TPlayer::AttackStimulus(uint32 AttackerID){
	if(!this->IsDead){
		this->BlockLogout(60, false);
	}
}

void TPlayer::SetInList(void){
	this->SetInCrList();

	PlayerMutex.down();
	*PlayerList.at(FirstFreePlayer) = this;
	FirstFreePlayer += 1;
	PlayerMutex.up();

	NotifyBuddies(this->ID, this->Name, true);
	IncrementPlayersOnline();
	if(this->Profession == PROFESSION_NONE){
		IncrementNewbiesOnline();
	}
}

void TPlayer::DelInList(void){
	NotifyBuddies(this->ID, this->Name, false);

	for(int Index = 0; Index < FirstFreePlayer; Index += 1){
		if(*PlayerList.at(Index) == this){
			// TODO(fusion): This can't be right? If the player at `Index` changes
			// before entering the critical section, we could end up removing the
			// wrong player from the list..
			PlayerMutex.down();
			FirstFreePlayer -= 1;
			*PlayerList.at(Index) = *PlayerList.at(FirstFreePlayer);
			*PlayerList.at(FirstFreePlayer) = NULL;
			PlayerMutex.up();

			DecrementPlayersOnline();
			if(this->Profession == PROFESSION_NONE){
				DecrementNewbiesOnline();
			}

			break;
		}
	}
}

void TPlayer::ClearRequest(void){
	if(this->Request != 0){
		if(this->RequestProcessingGamemaster != 0){
			TCreature *Gamemaster = GetPlayer(this->RequestProcessingGamemaster);
			if(Gamemaster != NULL){
				SendFinishRequest(Gamemaster->Connection, this->Name);
			}
		}else{
			DeleteGamemasterRequest(this->Name);
		}

		this->Request = 0;
		this->RequestTimestamp = 0;
		this->RequestProcessingGamemaster = 0;
	}
}

void TPlayer::ClearConnection(void){
	this->Connection = NULL;
	this->ClearRequest();
}

void TPlayer::LoadData(void){
	TPlayerData *PlayerData = this->PlayerData;
	if(PlayerData == NULL){
		error("TPlayer::LoadData: PlayerData ist NULL.\n");
		return;
	}

	this->Race = PlayerData->Race;
	this->OrgOutfit = PlayerData->OriginalOutfit;
	this->Outfit = PlayerData->CurrentOutfit;

	GetStartPosition(&this->startx, &this->starty, &this->startz, true);
	this->posx = this->startx;
	this->posy = this->starty;
	this->posz = this->startz;
	this->Direction = DIRECTION_SOUTH;

	if(PlayerData->startx != 0){
		this->startx = PlayerData->startx;
		this->starty = PlayerData->starty;
		this->startz = PlayerData->startz;
	}

	if(PlayerData->posx != 0){
		this->posx = PlayerData->posx;
		this->posy = PlayerData->posy;
		this->posz = PlayerData->posz;
	}

	this->Profession = PlayerData->Profession;
	this->EarliestYellRound = PlayerData->EarliestYellRound;
	this->EarliestTradeChannelRound = PlayerData->EarliestTradeChannelRound;
	this->EarliestSpellTime = PlayerData->EarliestSpellTime;
	this->EarliestMultiuseTime = PlayerData->EarliestMultiuseTime;
	this->TalkBufferFullTime = PlayerData->TalkBufferFullTime;
	this->MutingEndRound = PlayerData->MutingEndRound;
	this->NumberOfMutings = PlayerData->NumberOfMutings;


	STATIC_ASSERT(NARRAY(this->SpellList) == NARRAY(PlayerData->SpellList));
	for(int SpellNr = 0;
			SpellNr < NARRAY(this->SpellList);
			SpellNr += 1){
		this->SpellList[SpellNr] = PlayerData->SpellList[SpellNr];
	}

	STATIC_ASSERT(NARRAY(this->QuestValues) == NARRAY(PlayerData->QuestValues));
	for(int QuestNr = 0;
			QuestNr < NARRAY(this->QuestValues);
			QuestNr += 1){
		this->QuestValues[QuestNr] = PlayerData->QuestValues[QuestNr];
	}

	// NOTE(fusion): `Minimum` is set to `INT_MIN` to skip loading a skill, and
	// stick with the race's default.
	this->SetSkills(PlayerData->Race);
	STATIC_ASSERT(NARRAY(this->Skills) == NARRAY(PlayerData->Actual));
	for(int SkillNr = 0;
			SkillNr < NARRAY(this->Skills);
			SkillNr += 1){
		if(PlayerData->Minimum[SkillNr] == INT_MIN){
			continue;
		}

		this->Skills[SkillNr]->Load(
				PlayerData->Actual[SkillNr],
				PlayerData->Maximum[SkillNr],
				PlayerData->Minimum[SkillNr],
				PlayerData->DeltaAct[SkillNr],
				PlayerData->MagicDeltaAct[SkillNr],
				PlayerData->Cycle[SkillNr],
				PlayerData->MaxCycle[SkillNr],
				PlayerData->Count[SkillNr],
				PlayerData->MaxCount[SkillNr],
				PlayerData->AddLevel[SkillNr],
				PlayerData->Experience[SkillNr],
				PlayerData->FactorPercent[SkillNr],
				PlayerData->NextLevel[SkillNr],
				PlayerData->Delta[SkillNr]);
	}
}

void TPlayer::SaveData(void){
	TPlayerData *PlayerData = this->PlayerData;
	if(PlayerData == NULL){
		error("TPlayer::SaveData: PlayerData ist NULL.\n");
		return;
	}

	PlayerData->OriginalOutfit = this->OrgOutfit;
	PlayerData->CurrentOutfit = this->Outfit;

	PlayerData->startx = this->startx;
	PlayerData->starty = this->starty;
	PlayerData->startz = this->startz;
	PlayerData->posx = this->posx;
	PlayerData->posy = this->posy;
	PlayerData->posz = this->posz;

	PlayerData->Profession = this->Profession;
	PlayerData->EarliestYellRound = this->EarliestYellRound;
	PlayerData->EarliestTradeChannelRound = this->EarliestTradeChannelRound;
	PlayerData->EarliestSpellTime = this->EarliestSpellTime;
	PlayerData->EarliestMultiuseTime = this->EarliestMultiuseTime;
	PlayerData->TalkBufferFullTime = this->TalkBufferFullTime;
	PlayerData->MutingEndRound = this->MutingEndRound;
	PlayerData->NumberOfMutings = this->NumberOfMutings;

	STATIC_ASSERT(NARRAY(this->SpellList) == NARRAY(PlayerData->SpellList));
	for(int SpellNr = 0;
			SpellNr < NARRAY(this->SpellList);
			SpellNr += 1){
		PlayerData->SpellList[SpellNr] = this->SpellList[SpellNr];
	}

	STATIC_ASSERT(NARRAY(this->QuestValues) == NARRAY(PlayerData->QuestValues));
	for(int QuestNr = 0;
			QuestNr < NARRAY(this->QuestValues);
			QuestNr += 1){
		PlayerData->QuestValues[QuestNr] = this->QuestValues[QuestNr];
	}

	STATIC_ASSERT(NARRAY(this->Skills) == NARRAY(PlayerData->Actual));
	for(int SkillNr = 0;
			SkillNr < NARRAY(this->Skills);
			SkillNr += 1){
		// TODO(fusion): Is this even possible? I've seen a few checks here and
		// here but it seems all skills are created in TCreature's constructor.
		if(this->Skills[SkillNr] == NULL){
			PlayerData->Minimum[SkillNr] = INT_MIN;
			continue;
		}

		this->Skills[SkillNr]->Save(
				&PlayerData->Actual[SkillNr],
				&PlayerData->Maximum[SkillNr],
				&PlayerData->Minimum[SkillNr],
				&PlayerData->DeltaAct[SkillNr],
				&PlayerData->MagicDeltaAct[SkillNr],
				&PlayerData->Cycle[SkillNr],
				&PlayerData->MaxCycle[SkillNr],
				&PlayerData->Count[SkillNr],
				&PlayerData->MaxCount[SkillNr],
				&PlayerData->AddLevel[SkillNr],
				&PlayerData->Experience[SkillNr],
				&PlayerData->FactorPercent[SkillNr],
				&PlayerData->NextLevel[SkillNr],
				&PlayerData->Delta[SkillNr]);
	}
}

void TPlayer::LoadInventory(bool SetStandardInventory){
	TPlayerData *PlayerData = this->PlayerData;
	if(PlayerData == NULL){
		error("TPlayer::LoadInventory: PlayerData ist NULL.\n");
		return;
	}

	if(PlayerData->Inventory != NULL){
		try{
			TReadBuffer ReadBuffer(PlayerData->Inventory, PlayerData->InventorySize);
			while(true){
				int Position = (int)ReadBuffer.readByte();
				if(Position == 0xFF){
					break;
				}

				LoadObjects(&ReadBuffer, GetBodyContainer(this->ID, Position));
				Object BodyObj = GetBodyObject(this->ID, Position);
				if(BodyObj != NONE){
					SendSetInventory(this->Connection, Position, BodyObj);
				}
			}
		}catch(const char *str){
			error("TPlayer::LoadInventory: Kann Inventory von Spieler %s nicht lesen.\n", this->Name);
			error("# Fehler: %s\n", str);
		}
	}else if(SetStandardInventory
			&& this->Profession == PROFESSION_NONE
			&& !CheckRight(this->ID, ZERO_CAPACITY)){
		try{
			Create(GetBodyContainer(this->ID, INVENTORY_RIGHTHAND),
					GetSpecialObject(DEFAULT_RIGHTHAND), 0);
			Create(GetBodyContainer(this->ID, INVENTORY_LEFTHAND),
					GetSpecialObject(DEFAULT_LEFTHAND), 0);
			if(this->Sex == 1){
				Create(GetBodyContainer(this->ID, INVENTORY_TORSO),
						GetSpecialObject(DEFAULT_BODY_MALE), 0);
			}else{
				Create(GetBodyContainer(this->ID, INVENTORY_TORSO),
						GetSpecialObject(DEFAULT_BODY_FEMALE), 0);
			}

			Object Bag = Create(GetBodyContainer(this->ID, INVENTORY_BAG),
								GetSpecialObject(DEFAULT_CONTAINER), 0);
			Create(Bag, GetSpecialObject(DEFAULT_FOOD), 1);
		}catch(RESULT r){
			error("TPlayer::LoadInventory: Exception %d beim Erstellen des Standard-Inventorys.\n", r);
		}
	}
}

void TPlayer::SaveInventory(void){
	TPlayerData *PlayerData = this->PlayerData;
	if(PlayerData == NULL){
		error("TPlayer::SaveInventory: PlayerData ist NULL.\n");
		return;
	}

	// TODO(fusion): Set `PlayerData->Dirty`?
	delete[] PlayerData->Inventory;
	PlayerData->Inventory = NULL;
	PlayerData->InventorySize = 0;
	if(this->CrObject == NONE){
		return;
	}

	try{
		TDynamicWriteBuffer HelpBuffer(KB(16));
		for(int Position = INVENTORY_FIRST;
				Position <= INVENTORY_LAST;
				Position += 1){
			Object Obj = GetBodyObject(this->ID, Position);
			if(Obj.exists()){
				HelpBuffer.writeByte((uint8)Position);
				SaveObjects(Obj, &HelpBuffer, false);
			}
		}

		if(HelpBuffer.Position > 0){
			HelpBuffer.writeByte(0xFF);

			int InventorySize = HelpBuffer.Position;
			PlayerData->Inventory = new uint8[InventorySize];
			PlayerData->InventorySize = InventorySize;
			memcpy(PlayerData->Inventory, HelpBuffer.Data, InventorySize);
		}
	}catch(const char *str){
		error("TPlayer::SaveInventory: Kann Inventory von Spieler %s nicht schreiben.\n", this->Name);
		error("# Fehler: %s\n", str);
	}
}

void TPlayer::StartCoordinates(void){
	GetStartPosition(&this->startx, &this->starty, &this->startz, true);
}

void TPlayer::TakeOver(TConnection *Connection){
	Log("game", "Spieler %s übernimmt Verbindung.\n", this->Name);

	this->LoggingOut = false;
	this->LogoutAllowed = false;
	this->Connection = Connection;
	Connection->EnterGame();

	TPlayerData *PlayerData = GetPlayerPoolSlot(this->ID);
	if(PlayerData == NULL){
		error("TPlayer::TakeOver: PlayerData-Slot nicht gefunden.\n");
		return;
	}

	if(PlayerData->Locked != gettid()){
		error("TPlayer::TakeOver: PlayerData-Slot ist nicht korrekt gesperrt (%d).\n", PlayerData->Locked);
	}

	if(PlayerData->Sticky > 1){
		DecreasePlayerPoolSlotSticky(PlayerData);
	}else{
		error("TPlayer::TakeOver: Falscher Sticky-Wert %d.\n", PlayerData->Sticky);
	}

	strcpy(this->Name, PlayerData->Name);
	InsertPlayerIndex(&PlayerIndexHead, 0, this->Name, this->ID);
	strcpy(this->IPAddress, Connection->GetIPAddress());
	memcpy(this->Rights, PlayerData->Rights, sizeof(this->Rights));
	this->Sex = PlayerData->Sex;
	strcpy(this->Guild, PlayerData->Guild);
	strcpy(this->Rank, PlayerData->Rank);
	strcpy(this->Title, PlayerData->Title);

	this->CheckOutfit();
	this->ClearRequest();
	this->Combat.StopAttack(0);
	LeaveAllChannels(this->ID);
	this->CloseAllContainers();
	this->RejectTrade();

	if(CheckRight(this->ID, READ_GAMEMASTER_CHANNEL)){
		CloseProcessedRequests(this->ID);
	}

	SendInitGame(Connection, this->ID);
	SendRights(Connection);
	SendFullScreen(Connection);
	SendBodyInventory(Connection, this->ID);
	SendAmbiente(Connection);
	SendPlayerData(Connection);
	SendPlayerSkills(Connection);
	this->SyncState();
	this->SendBuddies();
}

void TPlayer::SetOpenContainer(int ContainerNr, Object Con){
	if(ContainerNr < 0 || ContainerNr >= NARRAY(this->OpenContainer)){
		error("TPlayer::SetOpenContainer: Ungültige Fensternummer %d.\n", ContainerNr);
		return;
	}

	if(Con != NONE && (!Con.exists() || !Con.getObjectType().getFlag(CONTAINER))){
		error("TPlayer::SetOpenContainer: Container existiert nicht.\n");
		return;
	}

	this->OpenContainer[ContainerNr] = Con;
}

Object TPlayer::GetOpenContainer(int ContainerNr){
	if(ContainerNr < 0 || ContainerNr >= NARRAY(this->OpenContainer)){
		error("TPlayer::GetOpenContainer: Ungültige Fensternummer %d.\n", ContainerNr);
		return NONE;
	}

	return this->OpenContainer[ContainerNr];
}

void TPlayer::CloseAllContainers(void){
	for(int ContainerNr = 0;
			ContainerNr < NARRAY(this->OpenContainer);
			ContainerNr += 1){
		Object Con = this->GetOpenContainer(ContainerNr);
		if(Con != NONE){
			this->SetOpenContainer(ContainerNr, NONE);
			SendCloseContainer(this->Connection, ContainerNr);
		}
	}
}

Object TPlayer::InspectTrade(bool OwnOffer, int Position){
	Object Obj = this->TradeObject;
	if(Obj == NONE){
		return NONE;
	}

	if(!OwnOffer){
		if(this->TradePartner == 0){
			return NONE;
		}

		TPlayer *Partner = GetPlayer(this->TradePartner);
		if(Partner == NULL
				|| Partner->TradeObject == NONE
				|| Partner->TradePartner != this->ID){
			return NONE;
		}

		Obj = Partner->TradeObject;
	}

	while(Obj != NONE && Position > 0){
		int ObjCount = CountObjects(Obj);
		if(Position < ObjCount){
			Obj = GetFirstContainerObject(Obj);
			Position -= 1;
		}else{
			Obj = Obj.getNextObject();
			Position -= ObjCount;
		}
	}

	return Obj;
}

void TPlayer::AcceptTrade(void){
	if(this->TradeObject == NONE){
		return;
	}

	TPlayer *Partner = GetPlayer(this->TradePartner);
	if(Partner == NULL
			|| Partner->TradeObject == NONE
			|| Partner->TradePartner != this->ID){
		return;
	}

	this->TradeAccepted = true;
	if(!Partner->TradeAccepted){
		return;
	}

	TPlayer *Player[2]		= { this, Partner };
	Object Dest[2]			= { NONE, NONE };
	Object Obj[2]			= { this->TradeObject, Partner->TradeObject };
	ObjectType ObjType[2]	= { Obj[0].getObjectType(), Obj[1].getObjectType() };
	for(int i = 0; i < 2; i += 1){
		int Cur = i;
		int Other = 1 - i;

		try{
			if(!ObjectAccessible(Player[Cur]->ID, Obj[Cur], 1)){
				throw NOTACCESSIBLE;
			}

			if(ObjType[Cur].getFlag(UNMOVE)){
				throw NOTMOVABLE;
			}

			if(!ObjType[Cur].getFlag(TAKE)){
				throw NOTTAKABLE;
			}

			if(CheckRight(Player[Other]->ID, ZERO_CAPACITY)){
				throw TOOHEAVY;
			}

			if(!CheckRight(Player[Other]->ID, UNLIMITED_CAPACITY)){
				TSkill *CarryStrength = Player[Other]->Skills[SKILL_CARRY_STRENGTH];
				if(CarryStrength == NULL){
					error("TPlayer::AcceptTrade: Skill CARRYSTRENGTH existiert nicht.\n");
					throw ERROR;
				}

				// NOTE(fusion): Check if the other player has enough carry strength,
				// considering the object that will be added and the object that may
				// be removed.
				int MaxWeight = CarryStrength->Get() * 100;
				int FinalWeight = GetInventoryWeight(Player[Other]->ID)
								+ GetCompleteWeight(Obj[Cur]);
				if(GetObjectCreatureID(Obj[Other]) == Player[Other]->ID){
					FinalWeight -= GetCompleteWeight(Obj[Other]);
				}

				if(FinalWeight > MaxWeight){
					throw TOOHEAVY;
				}
			}

			// NOTE(fusion): We're looking for the object's destination on the
			// other player's inventory. It is similar to `CreateAtCreature` but
			// not quite. This should probably be its own function.
			bool CheckContainers = ObjType[Cur].getFlag(MOVEMENTEVENT);
			for(int j = 0; j < 2; j += 1){
				for(int Position = INVENTORY_FIRST;
						Position <= INVENTORY_LAST;
						Position += 1){
					// TODO(fusion): It was only a matter of time until one of these showed up.
					try{
						Object BodyObj = GetBodyObject(Player[Other]->ID, Position);
						if(CheckContainers){
							if(BodyObj != NONE && BodyObj != Obj[Other]
									&& BodyObj.getObjectType().getFlag(CONTAINER)){
								// NOTE(fusion): Check if the container has enough capacity,
								// considering the object that will be added and the object
								// that may be removed.
								int MaxCount = BodyObj.getObjectType().getAttribute(CAPACITY);
								int FinalCount = CountObjectsInContainer(BodyObj) + 1;
								if(Obj[Other].getContainer() == BodyObj){
									FinalCount -= 1;
								}

								if(FinalCount > MaxCount){
									throw CONTAINERFULL;
								}

								Dest[Other] = BodyObj;
								break;
							}
						}else{
							if(BodyObj == NONE){
								Object BodyCon = GetBodyContainer(Player[Other]->ID, Position);
								CheckInventoryPlace(ObjType[Cur], BodyCon, Obj[Other]);
								Dest[Other] = BodyCon;
								break;
							}
						}
					}catch(RESULT r){
						// no-op
					}
				}

				if(Dest[Other] != NONE){
					break;
				}

				CheckContainers = !CheckContainers;
			}

			if(Dest[Other] == NONE){
				throw NOROOM;
			}
		}catch(RESULT r){
			if(r == TOOHEAVY || r == NOROOM){
				SendResult(Player[Other]->Connection, r);
			}else{
				SendResult(Player[Cur]->Connection, r);
			}

			Player[Cur]->TradeObject = NONE;
			Player[Other]->TradeObject = NONE;
			SendCloseTrade(Player[Cur]->Connection);
			SendCloseTrade(Player[Other]->Connection);
			return;
		}
	}

	this->TradeObject = NONE;
	Partner->TradeObject = NONE;
	SendCloseTrade(this->Connection);
	SendCloseTrade(Partner->Connection);

	// TODO(fusion): I feel this could be problematic.
	::Move(0, Obj[0], GetMapContainer(Player[1]->CrObject), -1, true, NONE);
	::Move(0, Obj[1], GetMapContainer(Player[0]->CrObject), -1, true, NONE);
	::Move(0, Obj[0], Dest[1], -1, true, NONE);
	::Move(0, Obj[1], Dest[0], -1, true, NONE);
}

void TPlayer::RejectTrade(void){
	if(this->TradeObject != NONE){
		this->TradeObject = NONE;
		TPlayer *Partner = GetPlayer(this->TradePartner);
		if(Partner != NULL
				&& Partner->TradeObject != NONE
				&& Partner->TradePartner == this->ID){
			Partner->TradeObject = NONE;
			SendCloseTrade(Partner->Connection);
			SendMessage(Partner->Connection, TALK_FAILURE_MESSAGE, "Trade cancelled.");
		}
	}
}

void TPlayer::ClearProfession(void){
	if(this->Profession != PROFESSION_NONE){
		this->Profession = PROFESSION_NONE;
		this->Combat.CheckCombatValues();
		IncrementNewbiesOnline();
	}
}

void TPlayer::SetProfession(uint8 Profession){
	if(Profession == PROFESSION_PROMOTION){
		if(this->Profession == PROFESSION_NONE){
			error("TPlayer::SetProfession: Spieler hat noch keinen Beruf für Veredelung.\n");
			return;
		}

		if(this->Profession >= PROFESSION_PROMOTION){
			error("TPlayer::SetProfession: Spieler hat seinen Beruf schon veredelt.\n");
			return;
		}

		this->Profession += PROFESSION_PROMOTION;
		this->Combat.CheckCombatValues();

		// TODO(fusion): This is similar to the TPlayer's constructor. It is
		// problably some `CheckSoul` inlined function?
		{
			TSkill *Soul = this->Skills[SKILL_SOUL];
			Soul->Max = 200;

			int Timer = Soul->TimerValue();
			if(Timer >= 15){
				int Cycle = (Timer + 14) / 15;
				int Count = (Timer + 14) % 15;
				this->SetTimer(SKILL_SOUL, Cycle, Count, 15, -1);
			}
		}

		return;
	}

	if(this->Profession != PROFESSION_NONE){
		error("TPlayer::SetProfession: Player '%s' hat bereits einen Beruf!\n", this->Name);
		return;
	}

	if(Profession == PROFESSION_KNIGHT){
		this->Skills[SKILL_HITPOINTS     ]->AddLevel = 15;
		this->Skills[SKILL_MANA          ]->AddLevel = 5;
		this->Skills[SKILL_CARRY_STRENGTH]->AddLevel = 25;
		this->Skills[SKILL_MAGIC_LEVEL   ]->ChangeSkill(3000, 1600);
		this->Skills[SKILL_SHIELDING     ]->ChangeSkill(1100, 100);
		this->Skills[SKILL_DISTANCE      ]->ChangeSkill(1400, 30);
		this->Skills[SKILL_SWORD         ]->ChangeSkill(1100, 50);
		this->Skills[SKILL_CLUB          ]->ChangeSkill(1100, 50);
		this->Skills[SKILL_AXE           ]->ChangeSkill(1100, 50);
		this->Skills[SKILL_FIST          ]->ChangeSkill(1100, 50);
	}else if(Profession == PROFESSION_PALADIN){
		this->Skills[SKILL_HITPOINTS     ]->AddLevel = 10;
		this->Skills[SKILL_MANA          ]->AddLevel = 15;
		this->Skills[SKILL_CARRY_STRENGTH]->AddLevel = 20;
		this->Skills[SKILL_MAGIC_LEVEL   ]->ChangeSkill(1400, 1600);
		this->Skills[SKILL_SHIELDING     ]->ChangeSkill(1100, 100);
		this->Skills[SKILL_DISTANCE      ]->ChangeSkill(1100, 30);
		this->Skills[SKILL_SWORD         ]->ChangeSkill(1200, 50);
		this->Skills[SKILL_CLUB          ]->ChangeSkill(1200, 50);
		this->Skills[SKILL_AXE           ]->ChangeSkill(1200, 50);
		this->Skills[SKILL_FIST          ]->ChangeSkill(1200, 50);
	}else if(Profession == PROFESSION_SORCERER){
		this->Skills[SKILL_HITPOINTS     ]->AddLevel = 5;
		this->Skills[SKILL_MANA          ]->AddLevel = 30;
		this->Skills[SKILL_CARRY_STRENGTH]->AddLevel = 10;
		this->Skills[SKILL_MAGIC_LEVEL   ]->ChangeSkill(1100, 1600);
		this->Skills[SKILL_SHIELDING     ]->ChangeSkill(1500, 100);
		this->Skills[SKILL_DISTANCE      ]->ChangeSkill(2000, 30);
		this->Skills[SKILL_SWORD         ]->ChangeSkill(2000, 50);
		this->Skills[SKILL_CLUB          ]->ChangeSkill(2000, 50);
		this->Skills[SKILL_AXE           ]->ChangeSkill(2000, 50);
		this->Skills[SKILL_FIST          ]->ChangeSkill(1500, 50);
	}else if(Profession == PROFESSION_DRUID){
		this->Skills[SKILL_HITPOINTS     ]->AddLevel = 5;
		this->Skills[SKILL_MANA          ]->AddLevel = 30;
		this->Skills[SKILL_CARRY_STRENGTH]->AddLevel = 10;
		this->Skills[SKILL_MAGIC_LEVEL   ]->ChangeSkill(1100, 1600);
		this->Skills[SKILL_SHIELDING     ]->ChangeSkill(1500, 100);
		this->Skills[SKILL_DISTANCE      ]->ChangeSkill(1800, 30);
		this->Skills[SKILL_SWORD         ]->ChangeSkill(1800, 50);
		this->Skills[SKILL_CLUB          ]->ChangeSkill(1800, 50);
		this->Skills[SKILL_AXE           ]->ChangeSkill(1800, 50);
		this->Skills[SKILL_FIST          ]->ChangeSkill(1500, 50);
	}else{
		error("TPlayer::SetProfession: Beruf %d existiert nicht!\n", Profession);
		return;
	}

	this->Profession = Profession;
	this->Combat.CheckCombatValues();
	DecrementNewbiesOnline();
}

uint8 TPlayer::GetRealProfession(void){
	return this->Profession;
}

uint8 TPlayer::GetEffectiveProfession(void){
	uint8 Profession = this->Profession;
	if(Profession >= PROFESSION_PROMOTION){
		Profession -= PROFESSION_PROMOTION;
	}
	return Profession;
}

uint8 TPlayer::GetActiveProfession(void){
	uint8 Profession;
	if(CheckRight(this->ID, PREMIUM_ACCOUNT)){
		Profession = this->GetRealProfession();
	}else{
		Profession = this->GetEffectiveProfession();
	}
	return Profession;
}

bool TPlayer::GetActivePromotion(void){
	return CheckRight(this->ID, PREMIUM_ACCOUNT)
		&& this->Profession >= PROFESSION_PROMOTION;
}

bool TPlayer::SpellKnown(int SpellNr){
	if(SpellNr < 0 || SpellNr >= NARRAY(this->SpellList)){
		error("TPlayer::SpellKnown: Ungültige Spruchnummer %d.\n", SpellNr);
		return false;
	}

	return this->SpellList[SpellNr] != 0;
}

void TPlayer::LearnSpell(int SpellNr){
	if(SpellNr < 0 || SpellNr >= NARRAY(this->SpellList)){
		error("TPlayer::LearnSpell: Ungültige Spruchnummer %d.\n", SpellNr);
		return;
	}

	if(this->SpellList[SpellNr] != 0){
		error("TPlayer::LearnSpell: Der Spieler kennt den Spruch schon.\n");
		return;
	}

	this->SpellList[SpellNr] = 1;
}

int TPlayer::GetQuestValue(int QuestNr){
	if(QuestNr < 0 || QuestNr >= NARRAY(this->QuestValues)){
		error("TPlayer::GetQuestValue: Ungültige Nummer %d.\n", QuestNr);
		return 0;
	}

	print(3, "Wert der Questvariablen %d von %s: %d.\n",
			QuestNr, this->Name, this->QuestValues[QuestNr]);
	return this->QuestValues[QuestNr];
}

void TPlayer::SetQuestValue(int QuestNr, int Value){
	if(QuestNr < 0 || QuestNr >= NARRAY(this->QuestValues)){
		error("TPlayer::SetQuestValue: Ungültige Nummer %d.\n", QuestNr);
		return;
	}

	print(3, "Neuer Wert für Questvariable %d von %s: %d.\n",
			QuestNr, this->Name, Value);
	this->QuestValues[QuestNr] = Value;
}

void TPlayer::CheckOutfit(void){
	if(CheckRight(this->ID, GAMEMASTER_OUTFIT)){
		if(this->OrgOutfit.OutfitID == 75){
			return;
		}

		this->OrgOutfit.OutfitID = 75;
		this->OrgOutfit.Colors[0] = 0;
		this->OrgOutfit.Colors[1] = 0;
		this->OrgOutfit.Colors[2] = 0;
		this->OrgOutfit.Colors[3] = 0;
	}else if(this->Sex == 1){
		if(this->OrgOutfit.OutfitID >= 128 && this->OrgOutfit.OutfitID <= 134){
			return;
		}

		this->OrgOutfit.OutfitID = 128;
		this->OrgOutfit.Colors[0] = 78;
		this->OrgOutfit.Colors[1] = 69;
		this->OrgOutfit.Colors[2] = 58;
		this->OrgOutfit.Colors[3] = 76;
	}else{
		if(this->OrgOutfit.OutfitID >= 136 && this->OrgOutfit.OutfitID <= 142){
			return;
		}

		this->OrgOutfit.OutfitID = 136;
		this->OrgOutfit.Colors[0] = 78;
		this->OrgOutfit.Colors[1] = 69;
		this->OrgOutfit.Colors[2] = 58;
		this->OrgOutfit.Colors[3] = 76;
	}

	this->Outfit = this->OrgOutfit;
}

void TPlayer::CheckState(void){
	if(this->Connection != NULL){
		uint8 State = 0;

		if(this->Skills[SKILL_POISON]->TimerValue() > 0){
			State |= 0x01;
		}

		if(this->Skills[SKILL_BURNING]->TimerValue() > 0){
			State |= 0x02;
		}

		if(this->Skills[SKILL_ENERGY]->TimerValue() > 0){
			State |= 0x04;
		}

		// TODO(fusion): I think the result from `Get()` here tells whether the
		// player has some drunk suppression item equipped?
		if(this->Skills[SKILL_DRUNKEN]->TimerValue() > 0
				&& this->Skills[SKILL_DRUNKEN]->Get() == 0){
			State |= 0x08;
		}

		if(this->Skills[SKILL_MANASHIELD]->TimerValue() > 0
				|| this->Skills[SKILL_MANASHIELD]->Get() > 0){
			State |= 0x10;
		}

		if(this->Skills[SKILL_GO_STRENGTH]->MDAct < 0){
			State |= 0x20;
		}else if(this->Skills[SKILL_GO_STRENGTH]->MDAct > 0){
			State |= 0x40;
		}

		if(RoundNr < this->EarliestLogoutRound){
			State |= 0x80;
		}

		if(State != this->OldState){
			SendPlayerState(this->Connection, State);
			this->OldState = State;
		}
	}
}

void TPlayer::SyncState(void){
	// NOTE(fusion): This is only called after a new connection is established,
	// when we know that the current client state is ZERO but `OldState` may not.
	//	It'll typically be the case in `TPlayer::TakeOver` but can also happen
	// in the player constructor if `TPlayer::CheckState` is called from within
	// any helper function before `TConnection::EnterGame` (see `BeginSendData`).
	this->OldState = 0;
	this->CheckState();
}

void TPlayer::AddBuddy(const char *Name){
	if(Name == NULL){
		error("TPlayer::AddBuddy: Name ist NULL.\n");
		return;
	}

	TPlayerData *PlayerData = this->PlayerData;
	if(PlayerData == NULL){
		error("TPlayer::AddBuddy: PlayerData ist NULL.\n");
		return;
	}

	if(PlayerData->Buddies >= 100){
		SendMessage(this->Connection, TALK_FAILURE_MESSAGE,
				"You cannot add more buddies.");
		return;
	}

	if(PlayerData->Buddies >= 20
			&& !CheckRight(this->ID, PREMIUM_ACCOUNT)
			&& !CheckRight(this->ID, READ_GAMEMASTER_CHANNEL)){
		SendMessage(this->Connection, TALK_FAILURE_MESSAGE,
				"You cannot add more buddies.");
		return;
	}

	TPlayer *Buddy = NULL;
	uint32 BuddyID = 0;
	char BuddyName[30] = {};
	bool Online = false;
	if(IdentifyPlayer(Name, true, true, &Buddy) == 0){
		BuddyID = Buddy->ID;
		strcpy(BuddyName, Buddy->Name);
		Online = true;
	}else{
		BuddyID = GetCharacterID(Name);
		if(BuddyID == 0){
			SendResult(this->Connection, PLAYERNOTEXISTING);
			return;
		}
		strcpy(BuddyName, GetCharacterName(Name));
		Online = false;
	}

	for(int i = 0; i < PlayerData->Buddies; i += 1){
		if(PlayerData->Buddy[i] == BuddyID){
			SendMessage(this->Connection, TALK_FAILURE_MESSAGE,
					"This player is already in your list.");
			return;
		}
	}

	PlayerData->Buddy[PlayerData->Buddies] = BuddyID;
	strcpy(PlayerData->BuddyName[PlayerData->Buddies], BuddyName);
	PlayerData->Buddies += 1;

	Online = Online && (!CheckRight(BuddyID, NO_STATISTICS)
			|| CheckRight(this->ID, READ_GAMEMASTER_CHANNEL));
	SendBuddyData(this->Connection, BuddyID, BuddyName, Online);
	AddBuddyOrder(this, BuddyID);
}

void TPlayer::RemoveBuddy(uint32 CharacterID){
	TPlayerData *PlayerData = this->PlayerData;
	if(PlayerData == NULL){
		error("TPlayer::RemoveBuddy: PlayerData ist NULL.\n");
		return;
	}

	int BuddyIndex = 0;
	while(BuddyIndex < PlayerData->Buddies){
		if(PlayerData->Buddy[BuddyIndex] == CharacterID){
			break;
		}
		BuddyIndex += 1;
	}

	if(BuddyIndex < PlayerData->Buddies){
		// NOTE(fusion): A little swap and pop action.
		PlayerData->Buddies -= 1;
		PlayerData->Buddy[BuddyIndex] = PlayerData->Buddy[PlayerData->Buddies];
		strcpy(PlayerData->BuddyName[BuddyIndex], PlayerData->BuddyName[PlayerData->Buddies]);
		RemoveBuddyOrder(this, CharacterID);
	}
}

void TPlayer::SendBuddies(void){
	TPlayerData *PlayerData = this->PlayerData;
	if(PlayerData == NULL){
		error("TPlayer::SendBuddies: PlayerData ist NULL.\n");
		return;
	}

	bool ReadGamemasterChannel = CheckRight(this->ID, READ_GAMEMASTER_CHANNEL);
	for(int i = 0; i < PlayerData->Buddies; i += 1){
		bool Online = false;
		uint32 BuddyID = PlayerData->Buddy[i];
		const char *BuddyName = PlayerData->BuddyName[i];
		TPlayer *Buddy = GetPlayer(BuddyID);
		if(Buddy != NULL){
			Online = ReadGamemasterChannel || !CheckRight(BuddyID, NO_STATISTICS);
			BuddyName = Buddy->Name;
		}
		SendBuddyData(this->Connection, BuddyID, BuddyName, Online);
	}
}

void TPlayer::Regenerate(void){
	// TODO(fusion): This is probably some inlined function that searches for an
	// object with an specific flag on a field.
	Object Bed = GetFirstObject(this->posx, this->posy, this->posz);
	while(Bed != NONE){
		if(Bed.getObjectType().getFlag(BED)){
			break;
		}
		Bed = Bed.getNextObject();
	}

	if(Bed == NONE){
		error("TPlayer::Regenerate: Bett nicht gefunden.\n");
		return;
	}

	if(!Bed.getObjectType().getFlag(TEXT)){
		error("TPlayer::Regenerate: Bett trägt keinen Text.\n");
		return;
	}

	uint32 Text = Bed.getAttribute(TEXTSTRING);
	if(Text == 0 || stricmp(GetDynamicString(Text), this->Name) != 0){
		return;
	}

	int OfflineTime = 0;
	if(this->PlayerData != NULL && this->PlayerData->LastLogoutTime != 0){
		OfflineTime = (int)(time(NULL) - this->PlayerData->LastLogoutTime);
	}

	int FoodTime = this->Skills[SKILL_FED]->TimerValue();
	int Regen = std::min<int>(FoodTime / 3, OfflineTime / 15);
	if(Regen > 0){
		this->Skills[SKILL_HITPOINTS]->Change(Regen / 4);
		this->Skills[SKILL_MANA     ]->Change(Regen);
		this->SetTimer(SKILL_FED, (FoodTime - Regen * 3), 0, 0, -1);
	}

	if(OfflineTime > 900){
		this->Skills[SKILL_SOUL]->Change(OfflineTime / 900);
	}

	try{
		UseObjects(0, Bed, Bed);
	}catch(RESULT r){
		error("TPlayer::Regenerate: Exception %d beim Aufräumen des Bettes.\n", r);
	}
}

bool TPlayer::IsAttacker(uint32 VictimID, bool CheckFormer){
	for(int i = 0; i < this->NumberOfAttackedPlayers; i += 1){
		if(*this->AttackedPlayers.at(i) == VictimID){
			return true;
		}
	}

	if(CheckFormer && (this->FormerLogoutRound + 5) >= RoundNr){
		for(int i = 0; i < this->NumberOfFormerAttackedPlayers; i += 1){
			if(*this->FormerAttackedPlayers.at(i) == VictimID){
				return true;
			}
		}
	}

	return false;
}

bool TPlayer::IsAggressor(bool CheckFormer){
	return this->Aggressor
		|| (CheckFormer && this->FormerAggressor
			&& (this->FormerLogoutRound + 5) >= RoundNr);
}

bool TPlayer::IsAttackJustified(uint32 VictimID){
	TPlayer *Victim = GetPlayer(VictimID);
	if(Victim == NULL){
		error("TPlayer::IsAttackJustified: Opfer existiert nicht.\n");
		return true;
	}

	if(WorldType != PVP_ENFORCED // TODO(fusion): Probably `WorldType == NORMAL` ?
			&& Victim->PlayerData != NULL
			&& Victim->PlayerData->PlayerkillerEnd == 0){
		if(Victim->IsAggressor(true)){
			return true;
		}

		if(Victim->InPartyWith(this, true)){
			return true;
		}

		return Victim->IsAttacker(this->ID, true);
	}

	return true;
}

void TPlayer::RecordAttack(uint32 VictimID){
	if(WorldType != NORMAL || VictimID == this->ID){
		return;
	}

	TPlayer *Victim = GetPlayer(VictimID);
	if(Victim == NULL){
		error("TPlayer::RecordAttack: Opfer existiert nicht.\n");
		return;
	}

	if(!Victim->InPartyWith(this, true)
			&& !Victim->IsAttacker(this->ID, true)
			&& !this->IsAttacker(VictimID, false)){
		*this->AttackedPlayers.at(this->NumberOfAttackedPlayers) = VictimID;
		this->NumberOfAttackedPlayers += 1;
		print(3, "Spieler %s ist Angreifer für Spieler %s.\n", this->Name, Victim->Name);
		if(Victim->Connection != NULL
		&& Victim->Connection->KnownCreature(this->ID, false) == KNOWNCREATURE_UPTODATE){
			SendCreatureSkull(Victim->Connection, this->ID);
		}
	}

	if(!this->IsAttackJustified(VictimID) && !this->Aggressor){
		this->Aggressor = true;
		print(3, "Spieler %s ist Aggressor.\n", this->Name);
		AnnounceChangedCreature(this->ID, CREATURE_SKULL_CHANGED);
	}
}

void TPlayer::RecordMurder(uint32 VictimID){
	if(WorldType != NORMAL || VictimID == this->ID
			|| this->IsAttackJustified(VictimID)){
		return;
	}

	TPlayer *Victim = GetPlayer(VictimID);
	if(Victim == NULL){
		error("TPlayer::RecordMurder: Opfer existiert nicht.\n");
		return;
	}

	print(3, "Ungerechtfertigter Mord von %s an %s.\n", this->Name, Victim->Name);

	int Now = (int)time(NULL);

	// TODO(fusion): It's just annoying that we check `PlayerData` in some places
	// but not others.
	ASSERT(this->PlayerData != NULL);
	TPlayerData *PlayerData = this->PlayerData;
	for(int i = 1; i < NARRAY(PlayerData->MurderTimestamps); i += 1){
		PlayerData->MurderTimestamps[i - 1] = PlayerData->MurderTimestamps[i];
	}
	PlayerData->MurderTimestamps[NARRAY(PlayerData->MurderTimestamps) - 1] = Now;
	SendMessage(this->Connection, TALK_ADMIN_MESSAGE,
			"Warning! The murder of %s was not justified.", Victim->Name);

	int Playerkilling = this->CheckPlayerkilling(Now);
	if(Playerkilling != 0){
		int OldPlayerkillerEnd = PlayerData->PlayerkillerEnd;
		PlayerData->PlayerkillerEnd = Now + 2592000; // 30 days
		if(OldPlayerkillerEnd == 0){
			print(3, "Spieler %s ist Playerkiller.\n", this->Name);
			AnnounceChangedCreature(this->ID, CREATURE_SKULL_CHANGED);
		}

		if(Playerkilling == 2){
			// TODO(fusion): This might be an inlined function that calls `PunishmentOrder`?
			PunishmentOrder(NULL, this->Name, this->IPAddress, 28, 2,
					"Exceeding the limit of unjustified kills by 100%.",
					0, NULL, 0, false);
		}
	}
}

void TPlayer::RecordDeath(uint32 AttackerID, int OldLevel, const char *Remark){
	bool Justified = true;
	if(AttackerID != 0 && AttackerID != this->ID && IsCreaturePlayer(AttackerID)){
		TPlayer *Attacker = GetPlayer(AttackerID);
		if(Attacker == NULL){
			return;
		}

		Justified = Attacker->IsAttackJustified(this->ID);
		Attacker->RecordMurder(this->ID);
	}
	CharacterDeathOrder(this, OldLevel, AttackerID, Remark, !Justified);
}

int TPlayer::CheckPlayerkilling(int Now){
	int LastDay = 0;
	int LastWeek = 0;
	int LastMonth = 0;

	ASSERT(this->PlayerData != NULL);
	TPlayerData *PlayerData = this->PlayerData;
	for(int i = 0; i < NARRAY(PlayerData->MurderTimestamps); i += 1){
		int Timestamp = PlayerData->MurderTimestamps[i];
		if(Timestamp == 0){
			continue;
		}

		if((Now - Timestamp) < 86000){
			LastDay += 1;
		}

		if((Now - Timestamp) < 604800){
			LastWeek += 1;
		}

		if((Now - Timestamp) < 2592000){
			LastMonth += 1;
		}
	}

	if(LastDay >= 6 || LastWeek >= 10 || LastMonth >= 20){
		return 2; // EXCESSIVE_KILLING ?
	}else if(LastDay >= 3 || LastWeek >= 5 || LastMonth >= 10){
		return 1; // PLAYERKILLER ?
	}else{
		return 0;
	}
}

void TPlayer::ClearAttacker(uint32 VictimID){
	int AttackedIndex = 0;
	while(AttackedIndex < this->NumberOfAttackedPlayers){
		if(*this->AttackedPlayers.at(AttackedIndex) == VictimID){
			break;
		}
		AttackedIndex += 1;
	}

	if(AttackedIndex >= this->NumberOfAttackedPlayers){
		return;
	}

	// NOTE(fusion): A little swap and pop action.
	this->NumberOfAttackedPlayers -= 1;
	*this->AttackedPlayers.at(AttackedIndex) = *this->AttackedPlayers.at(this->NumberOfAttackedPlayers);

	TPlayer *Victim = GetPlayer(VictimID);
	if(Victim != NULL
			&& Victim->Connection != NULL
			&& Victim->Connection->KnownCreature(this->ID, false) == KNOWNCREATURE_UPTODATE){
		SendCreatureSkull(Victim->Connection, this->ID);
	}
}

void TPlayer::ClearPlayerkillingMarks(void){
	print(3, "Lösche Markierungen von Spieler %s.\n", this->Name);

	for(int i = 0; i < this->NumberOfAttackedPlayers; i += 1){
		*this->FormerAttackedPlayers.at(i) = *this->AttackedPlayers.at(i);
	}

	this->NumberOfFormerAttackedPlayers = this->NumberOfAttackedPlayers;
	this->NumberOfAttackedPlayers = 0;
	this->FormerAggressor = this->Aggressor;
	this->FormerLogoutRound = RoundNr;

	if(this->Aggressor){
		this->Aggressor = false;
		print(3, "Spieler %s ist kein Aggressor mehr.\n", this->Name);
		AnnounceChangedCreature(this->ID, CREATURE_SKULL_CHANGED);
	}else{
		for(int i = 0; i < this->NumberOfFormerAttackedPlayers; i += 1){
			TPlayer *Victim = GetPlayer(*this->FormerAttackedPlayers.at(i));
			if(Victim != NULL){
				print(3, "Spieler %s ist kein Angreifer mehr für Spieler %s.\n", this->Name, Victim->Name);
				if(Victim->Connection != NULL
				&& Victim->Connection->KnownCreature(this->ID, false) == KNOWNCREATURE_UPTODATE){
					SendCreatureSkull(Victim->Connection, this->ID);
				}
			}
		}
	}

	for(int i = 0; i < FirstFreePlayer; i += 1){
		TPlayer *Player = *PlayerList.at(i);
		if(Player != NULL){
			Player->ClearAttacker(this->ID);
		}else{
			error("TPlayer::ClearPlayerkillingMarks: Spieler %d existiert nicht.\n", i);
		}
	}
}

int TPlayer::GetPlayerkillingMark(TPlayer *Observer){
	if(WorldType == NORMAL){
		if(Observer == NULL){
			error("TPlayer::GetPlayerkillingMark: Beobachter existiert nicht.\n");
			return SKULL_NONE;
		}

		ASSERT(this->PlayerData);
		if(this->PlayerData->PlayerkillerEnd != 0){
			return SKULL_RED;
		}

		if(this->Aggressor){
			return SKULL_WHITE;
		}

		if(this->InPartyWith(Observer, false)){
			return SKULL_GREEN;
		}

		if(this->IsAttacker(Observer->ID, false)){
			return SKULL_YELLOW;
		}
	}

	return SKULL_NONE;
}

uint32 TPlayer::GetPartyLeader(bool CheckFormer){
	if(this->PartyLeavingRound == 0 || (CheckFormer && (this->PartyLeavingRound + 5) >= RoundNr)){
		return this->PartyLeader;
	}else{
		return 0;
	}
}

bool TPlayer::InPartyWith(TPlayer *Other, bool CheckFormer){
	if(Other == NULL){
		error("TPlayer::InPartyWith: Other player is NULL.\n");
		return false;
	}

	return this->GetPartyLeader(CheckFormer) != 0
		&& this->GetPartyLeader(CheckFormer) == Other->GetPartyLeader(CheckFormer);
}

void TPlayer::JoinParty(uint32 LeaderID){
	this->PartyLeavingRound = 0;
	this->PartyLeader = LeaderID;
}

void TPlayer::LeaveParty(void){
	this->PartyLeavingRound = RoundNr;
}

int TPlayer::GetPartyMark(TPlayer *Observer){
	if(Observer == NULL){
		error("TPlayer::GetPartyMark: Beobachter existiert nicht.\n");
		return PARTY_SHIELD_NONE;
	}

	if(Observer->GetPartyLeader(false) == this->ID){
		return PARTY_SHIELD_LEADER;
	}

	if(this->InPartyWith(Observer, false)){
		return PARTY_SHIELD_MEMBER;
	}

	if(Observer->GetPartyLeader(false) == Observer->ID
			&& IsInvitedToParty(this->ID, Observer->ID)){
		return PARTY_SHIELD_GUEST;
	}

	if(this->GetPartyLeader(false) == this->ID
			&& IsInvitedToParty(Observer->ID, this->ID)){
		return PARTY_SHIELD_HOST;
	}

	return PARTY_SHIELD_NONE; // PARTY_NONE
}

int TPlayer::RecordTalk(void){
	int Muting = 0;
	if(this->TalkBufferFullTime > ServerMilliseconds){
		if(this->TalkBufferFullTime > (ServerMilliseconds + 7500)){
			this->NumberOfMutings += 1;
			Muting = (this->NumberOfMutings * this->NumberOfMutings) * 5;
			this->MutingEndRound = RoundNr + (uint32)Muting;
		}else{
			this->TalkBufferFullTime += 2500;
		}
	}else{
		this->TalkBufferFullTime = ServerMilliseconds + 2500;
	}
	return Muting;
}

int TPlayer::RecordMessage(uint32 AddresseeID){
	STATIC_ASSERT(NARRAY(this->Addressees) == NARRAY(this->AddresseesTimes));
	int AddresseeNr = -1;
	for(int i = 0; i < NARRAY(this->Addressees); i += 1){
		if(this->Addressees[i] == AddresseeID){
			AddresseeNr = i;
			break;
		}

		if(this->Addressees[i] == 0 || RoundNr > (this->AddresseesTimes[i] + 600)){
			AddresseeNr = i;
		}
	}

	int Muting = 0;
	if(AddresseeNr == -1){
		this->NumberOfMutings += 1;
		Muting = (this->NumberOfMutings * this->NumberOfMutings) * 5;
		this->MutingEndRound = RoundNr + (uint32)Muting;
	}else{
		this->Addressees[AddresseeNr] = AddresseeID;
		this->AddresseesTimes[AddresseeNr] = RoundNr;
	}
	return Muting;
}

int TPlayer::CheckForMuting(void){
	int Muting = 0;
	if(this->MutingEndRound > RoundNr){
		Muting = (int)(this->MutingEndRound - RoundNr);
	}
	return Muting;
}

// Player Utility
// =============================================================================
int GetNumberOfPlayers(void){
	return FirstFreePlayer;
}

TPlayer *GetPlayer(uint32 CharacterID){
	TCreature *Creature = GetCreature(CharacterID);
	if(Creature != NULL && Creature->Type != PLAYER){
		error("GetPlayer: Kreatur ist kein Spieler.\n");
		Creature = NULL;
	}
	return (TPlayer*)Creature;
}

TPlayer *GetPlayer(const char *Name){
	TPlayer *Result = NULL;
	for(int Index = 0; Index < FirstFreePlayer; Index += 1){
		TPlayer *Player = *PlayerList.at(Index);
		if(stricmp(Player->Name, Name) == 0){
			Result = Player;
			break;
		}
	}
	return Result;
}

bool IsPlayerOnline(const char *Name){
	// TODO(fusion): What is `PlayerMutex` actually doing and where is it used?
	bool Result;
	PlayerMutex.down();
	Result = (GetPlayer(Name) != NULL);
	PlayerMutex.up();
	return Result;
}

int IdentifyPlayer(const char *Name, bool ExactMatch, bool IgnoreGamemasters, TPlayer **OutPlayer){
	if(Name == NULL){
		error("IdentifyPlayer: Name ist NULL.\n");
		return -1; // NOTFOUND ?
	}

	if(Name[0] == 0){
		error("IdentifyPlayer: Name ist leer.\n");
		return -1; // NOTFOUND ?
	}

	int NameLength = (int)strlen(Name);
	if(!ExactMatch){
		if(Name[NameLength - 1] != '~'){
			ExactMatch = true;
		}else{
			NameLength -= 1;
		}
	}

	int Hits = 0;
	for(int Index = 0; Index < FirstFreePlayer; Index += 1){
		TPlayer *Player = *PlayerList.at(Index);
		if(stricmp(Player->Name, Name, NameLength) == 0){
			if(NameLength == (int)strlen(Player->Name)){
				*OutPlayer = Player;
				return 0; // FOUND ?
			}else if(!ExactMatch){
				if(IgnoreGamemasters && CheckRight(Player->ID, NO_STATISTICS)){
					continue;
				}

				*OutPlayer = Player;
				Hits += 1;
			}
		}
	}

	if(Hits == 0){
		return -1; // NOTFOUND ?
	}else if(Hits > 1){
		return -2; // AMBIGUOUS ?
	}

	return 0; // FOUND ?
}

void LogoutAllPlayers(void){
	print(1, "LogoutAllPlayers: Werde alle Spieler ausloggen!\n");
	for(int Index = 0; Index < FirstFreePlayer; Index += 1){
		TPlayer *Player = *PlayerList.at(Index);

		// TODO(fusion): Should we be checking if `Player` is NULL, because
		// `GetPlayer` and `IdentifyPlayer` do not.
		if(Player == NULL){
			error("LogoutAllPlayers: Eintrag %d in der Playerlist ist NULL.\n", Index);
			continue;
		}

		// TODO(fusion): Same thing as with `ProcessCreatures`. Players are removed
		// from `PlayerList` by the player's destructor, in a swap and pop fashion.
		// What a disaster.
		delete Player;
		Index -= 1;
	}
}

void CloseProcessedRequests(uint32 CharacterID){
	for(int Index = 0; Index < FirstFreePlayer; Index += 1){
		TPlayer *Player = *PlayerList.at(Index);
		if(Player == NULL){
			error("CloseProcessedRequests: Spieler %d existiert nicht.\n", Index);
			continue;
		}

		if(Player->Request != 0 && Player->RequestProcessingGamemaster == CharacterID){
			SendCloseRequest(Player->Connection);
			Player->Request = 0;
		}
	}
}

void NotifyBuddies(uint32 CharacterID, const char *Name, bool Login){
	if(Name == NULL || Name[0] == 0){
		error("NotifyBuddies: Name existiert nicht.\n");
		return;
	}

	bool Hide = CheckRight(CharacterID, NO_STATISTICS);
	for(int Index = 0; Index < FirstFreePlayer; Index += 1){
		TPlayer *Player = *PlayerList.at(Index);
		if(Player->Connection == NULL || Player->PlayerData == NULL){
			continue;
		}

		if(Hide && !CheckRight(Player->ID, READ_GAMEMASTER_CHANNEL)){
			continue;
		}

		TPlayerData *PlayerData = Player->PlayerData;
		for(int BuddyIndex = 0;
				BuddyIndex < PlayerData->Buddies;
				BuddyIndex += 1){
			if(PlayerData->Buddy[BuddyIndex] == CharacterID){
				if(strcmp(PlayerData->BuddyName[BuddyIndex], Name) == 0){
					SendBuddyStatus(Player->Connection, CharacterID, Login);
				}else{
					SendBuddyData(Player->Connection, CharacterID, Name, Login);
					strcpy(PlayerData->BuddyName[BuddyIndex], Name);
				}
			}
		}
	}
}

void CreatePlayerList(bool Online){
	// TODO(fusion): Same as `WriteKillStatistics` for names.

	// TODO(fusion): We can avoid allocating these buffers when `Online` is false.
	// We'll be sure when we dive into `writer.cc` and `reader.cc` and learn more
	// about these "orders".

	char *PlayerNames = new char[FirstFreePlayer * 30];
	int *PlayerLevels = new int[FirstFreePlayer];
	int *PlayerProfessions = new int[FirstFreePlayer];
	int NumberOfPlayers = -1;
	if(Online){
		NumberOfPlayers = 0;
		for(int Index = 0; Index < FirstFreePlayer; Index += 1){
			TPlayer *Player = *PlayerList.at(Index);
			if(CheckRight(Player->ID, NO_STATISTICS)){
				continue;
			}

			strcpy(&PlayerNames[NumberOfPlayers * 30], Player->Name);
			PlayerLevels[NumberOfPlayers] = Player->Skills[SKILL_LEVEL]->Get();
			PlayerProfessions[NumberOfPlayers] = Player->GetActiveProfession();
			NumberOfPlayers += 1;
		}
		Log("load", "%d %d\n", (int)time(NULL), FirstFreePlayer);
	}

	PlayerlistOrder(NumberOfPlayers, PlayerNames, PlayerLevels, PlayerProfessions);
}

void PrintPlayerPositions(void){
	for(int Index = 0; Index < FirstFreePlayer; Index += 1){
		TPlayer *Player = *PlayerList.at(Index);
		Log("players", "[%d,%d,%d] %d %d\n",
				Player->posx, Player->posy, Player->posz,
				Player->Skills[SKILL_LEVEL]->Get(),
				Player->GetActiveProfession());
	}
}

void LoadDepot(TPlayerData *PlayerData, int DepotNr, Object Con){
	if(PlayerData == NULL){
		error("LoadDepot: PlayerData ist NULL.\n");
		throw ERROR;
	}

	if(!Con.exists()){
		error("LoadDepot: Übergebener Container existiert nicht.\n");
		throw ERROR;
	}

	if(!Con.getObjectType().getFlag(CONTAINER)){
		error("LoadDepot: Übergebenes Objekt ist kein Container.\n");
		throw ERROR;
	}

	if(DepotNr < 0 || DepotNr >= MAX_DEPOTS){
		error("LoadDepot: Ungültige Depotnummer %d.\n", DepotNr);
		throw ERROR;
	}

	if(PlayerData->Depot[DepotNr] == NULL){
		Create(Con, GetSpecialObject(DEPOT_CHEST), 0);
	}else{
		try{
			TReadBuffer ReadBuffer(
					PlayerData->Depot[DepotNr],
					PlayerData->DepotSize[DepotNr]);
			LoadObjects(&ReadBuffer, Con);
		}catch(const char *str){
			error("LoadDepot: Kann Depot nicht lesen (%s).\n", str);
			throw ERROR;
		}
	}
}

void SaveDepot(TPlayerData *PlayerData, int DepotNr, Object Con){
	if(PlayerData == NULL){
		error("SaveDepot: PlayerData ist NULL.\n");
		throw ERROR;
	}

	if(!Con.exists()){
		error("SaveDepot: Übergebener Container existiert nicht.\n");
		throw ERROR;
	}

	if(!Con.getObjectType().getFlag(CONTAINER)){
		error("SaveDepot: Übergebenes Objekt ist kein Container.\n");
		throw ERROR;
	}

	if(DepotNr < 0 || DepotNr >= MAX_DEPOTS){
		error("SaveDepot: Ungültige Depotnummer %d.\n", DepotNr);
		throw ERROR;
	}

	PlayerData->Dirty = true;
	delete[] PlayerData->Depot[DepotNr];
	PlayerData->Depot[DepotNr] = NULL;
	PlayerData->DepotSize[DepotNr] = 0;

	try{
		TDynamicWriteBuffer HelpBuffer(KB(16));
		SaveObjects(GetFirstContainerObject(Con), &HelpBuffer, false);

		if(HelpBuffer.Position > 0){
			int DepotSize = HelpBuffer.Position;
			PlayerData->Depot[DepotNr] = new uint8[DepotSize];
			PlayerData->DepotSize[DepotNr] = DepotSize;
			memcpy(PlayerData->Depot[DepotNr], HelpBuffer.Data, DepotSize);
		}
	}catch(const char *str){
		error("SaveDepot: Kann Depot nicht schreiben (%s).\n", str);
		PlayerData->Depot[DepotNr] = NULL;
		PlayerData->DepotSize[DepotNr] = 0;
	}
}

void GetProfessionName(char *Buffer, int Profession, bool Article, bool Capitals){
	Buffer[0] = 0;

	if(Article){
		if(Profession == PROFESSION_ELITE_KNIGHT
		|| Profession == PROFESSION_ELDER_DRUID){
			strcat(Buffer, "an ");
		}else{
			strcat(Buffer, "a ");
		}
	}

	switch(Profession){
		case PROFESSION_NONE:				strcat(Buffer, "None"); break;
		case PROFESSION_KNIGHT:				strcat(Buffer, "Knight"); break;
		case PROFESSION_PALADIN:			strcat(Buffer, "Paladin"); break;
		case PROFESSION_SORCERER:			strcat(Buffer, "Sorcerer"); break;
		case PROFESSION_DRUID:				strcat(Buffer, "Druid"); break;
		case PROFESSION_ELITE_KNIGHT:		strcat(Buffer, "Elite Knight"); break;
		case PROFESSION_ROYAL_PALADIN:		strcat(Buffer, "Royal Paladin"); break;
		case PROFESSION_MASTER_SORCERER:	strcat(Buffer, "Master Sorcerer"); break;
		case PROFESSION_ELDER_DRUID:		strcat(Buffer, "Elder Druid"); break;
		// NOTE(fusion): Not in the original function but w/e.
		default:							strcat(Buffer, "Unknown"); break;
	}

	if(!Capitals){
		strLower(Buffer);
	}
}

void SendExistingRequests(TConnection *Connection){
	if(Connection == NULL){
		error("SendExistingRequests: Verbindung ist NULL.\n");
		return;
	}

	// TODO(fusion): This function would originally use `alloca` to allocate
	// the `Players` array.

	int NumPlayers = 0;
	TPlayer **Players = new TPlayer*[FirstFreePlayer];
	for(int Index = 0; Index < FirstFreePlayer; Index += 1){
		TPlayer *Player = *PlayerList.at(Index);
		if(Player == NULL){
			error("SendExistingRequests: Spieler %d existiert nicht.\n", Index);
			continue;
		}

		if(Player->Request != 0 && Player->RequestProcessingGamemaster == 0){
			Players[NumPlayers] = Player;
			NumPlayers += 1;
		}
	}

	// NOTE(fusion): Little selection sort by request timestamp.
	for(int i = 0; i < (NumPlayers - 1); i += 1){
		for(int j = i + 1; j < NumPlayers; j += 1){
			if(Players[j]->RequestTimestamp < Players[i]->RequestTimestamp){
				std::swap(Players[i], Players[j]);
			}
		}
	}

	for(int i = 0; i < NumPlayers; i += 1){
		TPlayer *Player = Players[i];
		SendTalk(Connection, 0, Player->Name, TALK_GAMEMASTER_REQUEST,
				GetDynamicString(Player->Request),
				(RoundNr - Player->RequestTimestamp));
	}

	delete []Players;
}

// Player Loader
// =============================================================================
void PlayerDataPath(char *Buffer, int BufferSize, uint32 CharacterID){
	snprintf(Buffer, BufferSize, "%s/%02u/%u.usr",
			USERPATH, (CharacterID % 100), CharacterID);
}

bool PlayerDataExists(uint32 CharacterID){
	char FileName[4096];
	PlayerDataPath(FileName, sizeof(FileName), CharacterID);
	return FileExists(FileName);
}

bool LoadPlayerData(TPlayerData *Slot){
	if(Slot == NULL){
		error("LoadPlayerData: Slot ist NULL.\n");
		return false;
	}

	if(Slot->CharacterID == 0){
		error("LoadPlayerData: Slot enthält keinen Charakter.\n");
		return false;
	}

	// IMPORTANT(fusion): This function is only called from `AssignPlayerPoolSlot`
	// which zero initializes it before hand. Note that it would be a problem
	// otherwise, since we shouldn't write to `CharacterID`, `Locked`, or `Sticky`
	// outside a critical section, making `memset` not viable and turning this
	// into an assignment fiesta for no good reason.
	Slot->Race = 1;
	Slot->Profession = PROFESSION_NONE;
	for(int SkillNr = 0;
			SkillNr < NARRAY(Slot->Minimum);
			SkillNr += 1){
		// NOTE(fusion): See `TPlayer::LoadData`.
		Slot->Minimum[SkillNr] = INT_MIN;
	}

	// NOTE(fusion): First login. Use defaults.
	char FileName[4096];
	PlayerDataPath(FileName, sizeof(FileName), Slot->CharacterID);
	if(!FileExists(FileName)){
		return true;
	}

	bool Result = false;
	try{
		// TODO(fusion): Same thing as house loaders. Data is expected to be in
		// an exact order and we don't check identifiers
		TDynamicWriteBuffer HelpBuffer(KB(16));
		TReadScriptFile Script;
		Script.open(FileName);

		Script.readIdentifier(); // "id"
		Script.readSymbol('=');
		Script.readNumber();

		Script.readIdentifier(); // "name"
		Script.readSymbol('=');
		strcpy(Slot->Name, Script.readString());

		Script.readIdentifier(); // "race"
		Script.readSymbol('=');
		Slot->Race = Script.readNumber();

		Script.readIdentifier(); // "profession"
		Script.readSymbol('=');
		Slot->Profession = (uint8)Script.readNumber();

		Script.readIdentifier(); // "originaloutfit"
		Script.readSymbol('=');
		Slot->OriginalOutfit = ReadOutfit(&Script);

		Script.readIdentifier(); // "currentoutfit"
		Script.readSymbol('=');
		Slot->CurrentOutfit = ReadOutfit(&Script);

		Script.readIdentifier(); // "lastlogin"
		Script.readSymbol('=');
		Slot->LastLoginTime = (time_t)Script.readNumber();

		Script.readIdentifier(); // "lastlogout"
		Script.readSymbol('=');
		Slot->LastLogoutTime = (time_t)Script.readNumber();

		Script.readIdentifier(); // "startposition"
		Script.readSymbol('=');
		Script.readCoordinate(&Slot->startx, &Slot->starty, &Slot->startz);

		Script.readIdentifier(); // "currentposition"
		Script.readSymbol('=');
		Script.readCoordinate(&Slot->posx, &Slot->posy, &Slot->posz);

		Script.readIdentifier(); // "playerkillerend"
		Script.readSymbol('=');
		Slot->PlayerkillerEnd = Script.readNumber();

		while(strcmp(Script.readIdentifier(), "skill") == 0){
			Script.readSymbol('=');
			Script.readSymbol('(');
			int SkillNr = Script.readNumber();
			if(SkillNr < 0 || SkillNr >= NARRAY(Slot->Minimum)){
				Script.error("illegal skill number");
			}
			Script.readSymbol(',');
			Slot->Actual[SkillNr] = Script.readNumber();
			Script.readSymbol(',');
			Slot->Maximum[SkillNr] = Script.readNumber();
			Script.readSymbol(',');
			Slot->Minimum[SkillNr] = Script.readNumber();
			Script.readSymbol(',');
			Slot->DeltaAct[SkillNr] = Script.readNumber();
			Script.readSymbol(',');
			Slot->MagicDeltaAct[SkillNr] = Script.readNumber();
			Script.readSymbol(',');
			Slot->Cycle[SkillNr] = Script.readNumber();
			Script.readSymbol(',');
			Slot->MaxCycle[SkillNr] = Script.readNumber();
			Script.readSymbol(',');
			Slot->Count[SkillNr] = Script.readNumber();
			Script.readSymbol(',');
			Slot->MaxCount[SkillNr] = Script.readNumber();
			Script.readSymbol(',');
			Slot->AddLevel[SkillNr] = Script.readNumber();
			Script.readSymbol(',');
			Slot->Experience[SkillNr] = Script.readNumber();
			Script.readSymbol(',');
			Slot->FactorPercent[SkillNr] = Script.readNumber();
			Script.readSymbol(',');
			Slot->NextLevel[SkillNr] = Script.readNumber();
			Script.readSymbol(',');
			Slot->Delta[SkillNr] = Script.readNumber();
			Script.readSymbol(')');
		}

		// "spells", already primed in the loop condition above
		Script.readSymbol('=');
		Script.readSymbol('{');
		while(true){
			Script.nextToken();
			if(Script.Token == SPECIAL){
				if(Script.getSpecial() == '}'){
					break;
				}else if(Script.getSpecial() == ','){
					continue;
				}
			}

			int SpellNr = Script.getNumber();
			if(SpellNr < 0 || SpellNr >= NARRAY(Slot->SpellList)){
				Script.error("illegal spell number");
			}

			Slot->SpellList[SpellNr] = 1;
		}

		Script.readIdentifier(); // "questvalues"
		Script.readSymbol('=');
		Script.readSymbol('{');
		while(true){
			char Special = Script.readSpecial();
			if(Special == '}'){
				break;
			}else if(Special == ','){
				continue;
			}else if(Special != '('){
				Script.error("'(' expected");
			}

			int QuestNr = Script.readNumber();
			if(QuestNr < 0 || QuestNr >= NARRAY(Slot->QuestValues)){
				Script.error("illegal quest number");
			}
			Script.readSymbol(',');
			Slot->QuestValues[QuestNr] = Script.readNumber();
			Script.readSymbol(')');
		}

		Script.readIdentifier(); // "murders"
		Script.readSymbol('=');
		Script.readSymbol('{');
		while(true){
			Script.nextToken();
			if(Script.Token == SPECIAL){
				if(Script.getSpecial() == '}'){
					break;
				}else if(Script.getSpecial() == ','){
					continue;
				}
			}

			for(int i = 1; i < NARRAY(Slot->MurderTimestamps); i += 1){
				Slot->MurderTimestamps[i - 1] = Slot->MurderTimestamps[i];
			}

			Slot->MurderTimestamps[NARRAY(Slot->MurderTimestamps) - 1] = Script.getNumber();
		}

		HelpBuffer.Position = 0;
		Script.readIdentifier(); // "inventory"
		Script.readSymbol('=');
		Script.readSymbol('{');
		while(true){
			Script.nextToken();
			if(Script.Token == SPECIAL){
				if(Script.getSpecial() == '}'){
					break;
				}else if(Script.getSpecial() == ','){
					continue;
				}
			}

			int Position = Script.getNumber();
			if(Position < INVENTORY_FIRST || Position > INVENTORY_LAST){
				Script.error("illegal inventory position");
			}

			Script.readIdentifier(); // "content"
			Script.readSymbol('=');
			HelpBuffer.writeByte((uint8)Position);
			LoadObjects(&Script, &HelpBuffer, false);
		}
		HelpBuffer.writeByte(0xFF);
		Slot->Inventory = new uint8[HelpBuffer.Position];
		Slot->InventorySize = HelpBuffer.Position;
		memcpy(Slot->Inventory, HelpBuffer.Data, HelpBuffer.Position);

		Script.readIdentifier(); // "depots"
		Script.readSymbol('=');
		Script.readSymbol('{');
		while(true){
			Script.nextToken();
			if(Script.Token == SPECIAL){
				if(Script.getSpecial() == '}'){
					break;
				}else if(Script.getSpecial() == ','){
					continue;
				}
			}

			int DepotNr = Script.getNumber();
			if(DepotNr < 0 || DepotNr >= MAX_DEPOTS){
				Script.error("illegal depot number");
			}

			Script.readIdentifier(); // "content"
			Script.readSymbol('=');
			HelpBuffer.Position = 0;
			LoadObjects(&Script, &HelpBuffer, false);
			Slot->Depot[DepotNr] = new uint8[HelpBuffer.Position];
			Slot->DepotSize[DepotNr] = HelpBuffer.Position;
			memcpy(Slot->Depot[DepotNr], HelpBuffer.Data, HelpBuffer.Position);
		}

		Script.nextToken();
		if(Script.Token != ENDOFFILE){
			Script.error("end of file expected");
		}

		Script.close();
		Result = true;
	}catch(const char *str){
		error("LoadPlayerData: Kann Gegenstände des Spielers %u nicht laden.\n",
				Slot->CharacterID);
		error("# Fehler: %s\n", str);
	}catch(const std::bad_alloc &e){
		error("LoadPlayerData: Kein Speicher frei beim Laden von Spieler %u.\n",
				Slot->CharacterID);
	}

	if(!Result){
		delete[] Slot->Inventory;
		Slot->Inventory = NULL;
		Slot->InventorySize = 0;

		for(int DepotNr = 0; DepotNr < MAX_DEPOTS; DepotNr += 1){
			delete[] Slot->Depot[DepotNr];
			Slot->Depot[DepotNr] = NULL;
			Slot->DepotSize[DepotNr] = 0;
		}
	}

	return Result;
}

void SavePlayerData(TPlayerData *Slot){
	if(Slot == NULL){
		error("SavePlayerData: Slot ist NULL.\n");
		return;
	}

	if(Slot->CharacterID == 0){
		error("SavePlayerData: Slot enthält keinen Charakter.\n");
		return;
	}

	// TODO(fusion): This is prone to problems if we don't backup user files.
	// Even if we did automatic backups, would only this user get rolled back?
	// This is probably one of the sources of whole day rollbacks.
	char FileName[4096];
	PlayerDataPath(FileName, sizeof(FileName), Slot->CharacterID);
	try{
		TWriteScriptFile Script;
		Script.open(FileName);

		Script.writeText("ID              = ");
		Script.writeNumber(Slot->CharacterID);
		Script.writeLn();

		Script.writeText("Name            = ");
		Script.writeString(Slot->Name);
		Script.writeLn();

		Script.writeText("Race            = ");
		Script.writeNumber(Slot->Race);
		Script.writeLn();

		Script.writeText("Profession      = ");
		Script.writeNumber(Slot->Profession);
		Script.writeLn();

		Script.writeText("OriginalOutfit  = ");
		WriteOutfit(&Script, Slot->OriginalOutfit);
		Script.writeLn();

		Script.writeText("CurrentOutfit   = ");
		WriteOutfit(&Script, Slot->CurrentOutfit);
		Script.writeLn();

		Script.writeText("LastLogin       = ");
		Script.writeNumber((int)Slot->LastLoginTime);
		Script.writeLn();

		Script.writeText("LastLogout      = ");
		Script.writeNumber((int)Slot->LastLogoutTime);
		Script.writeLn();

		Script.writeText("StartPosition   = ");
		Script.writeCoordinate(Slot->startx, Slot->starty, Slot->startz);
		Script.writeLn();

		Script.writeText("CurrentPosition = ");
		Script.writeCoordinate(Slot->posx, Slot->posy, Slot->posz);
		Script.writeLn();

		Script.writeText("PlayerkillerEnd = ");
		Script.writeNumber(Slot->PlayerkillerEnd);
		Script.writeLn();
		Script.writeLn();

		for(int SkillNr = 0;
				SkillNr < NARRAY(Slot->Minimum);
				SkillNr += 1){
			if(Slot->Minimum[SkillNr] == INT_MIN){
				continue;
			}

			Script.writeText("Skill = (");
			Script.writeNumber(SkillNr);
			Script.writeText(",");
			Script.writeNumber(Slot->Actual[SkillNr]);
			Script.writeText(",");
			Script.writeNumber(Slot->Maximum[SkillNr]);
			Script.writeText(",");
			Script.writeNumber(Slot->Minimum[SkillNr]);
			Script.writeText(",");
			Script.writeNumber(Slot->DeltaAct[SkillNr]);
			Script.writeText(",");
			Script.writeNumber(Slot->MagicDeltaAct[SkillNr]);
			Script.writeText(",");
			Script.writeNumber(Slot->Cycle[SkillNr]);
			Script.writeText(",");
			Script.writeNumber(Slot->MaxCycle[SkillNr]);
			Script.writeText(",");
			Script.writeNumber(Slot->Count[SkillNr]);
			Script.writeText(",");
			Script.writeNumber(Slot->MaxCount[SkillNr]);
			Script.writeText(",");
			Script.writeNumber(Slot->AddLevel[SkillNr]);
			Script.writeText(",");
			Script.writeNumber(Slot->Experience[SkillNr]);
			Script.writeText(",");
			Script.writeNumber(Slot->FactorPercent[SkillNr]);
			Script.writeText(",");
			Script.writeNumber(Slot->NextLevel[SkillNr]);
			Script.writeText(",");
			Script.writeNumber(Slot->Delta[SkillNr]);
			Script.writeText(")");
			Script.writeLn();
		}
		Script.writeLn();

		bool FirstSpell = true;
		Script.writeText("Spells      = {");
		for(int SpellNr = 0;
				SpellNr < NARRAY(Slot->SpellList);
				SpellNr += 1){
			if(Slot->SpellList[SpellNr] != 0){
				if(!FirstSpell){
					Script.writeText(",");
				}
				Script.writeNumber(SpellNr);
				FirstSpell = false;
			}
		}
		Script.writeText("}");
		Script.writeLn();

		bool FirstQuest = true;
		Script.writeText("QuestValues = {");
		for(int QuestNr = 0;
				QuestNr < NARRAY(Slot->QuestValues);
				QuestNr += 1){
			if(Slot->QuestValues[QuestNr] != 0){
				if(!FirstQuest){
					Script.writeText(",");
				}
				Script.writeText("(");
				Script.writeNumber(QuestNr);
				Script.writeText(",");
				Script.writeNumber(Slot->QuestValues[QuestNr]);
				Script.writeText(")");
				FirstQuest = false;
			}
		}
		Script.writeText("}");
		Script.writeLn();

		bool FirstMurder = true;
		int Now = (int)time(NULL);
		Script.writeText("Murders     = {");
		for(int i = 0; i < NARRAY(Slot->MurderTimestamps); i += 1){
			// NOTE(fusion): Save murder timestamps for up to a month.
			if((Now - Slot->MurderTimestamps[i]) < (30 * 24 * 60 * 60)){
				if(!FirstMurder){
					Script.writeText(",");
				}
				Script.writeNumber(Slot->MurderTimestamps[i]);
				FirstMurder = false;
			}
		}
		Script.writeText("}");
		Script.writeLn();
		Script.writeLn();

		bool FirstPosition = true;
		Script.writeText("Inventory   = {");
		if(Slot->Inventory != NULL){
			TReadBuffer Buffer(Slot->Inventory, Slot->InventorySize);
			while(true){
				int Position = (int)Buffer.readByte();
				if(Position == 0xFF){
					break;
				}

				if(!FirstPosition){
					Script.writeText(",");
					Script.writeLn();
					Script.writeText("               ");
				}
				Script.writeNumber(Position);
				Script.writeText(" Content=");
				SaveObjects(&Buffer, &Script);
				FirstPosition = false;
			}
		}
		Script.writeText("}");
		Script.writeLn();
		Script.writeLn();

		bool FirstDepot = true;
		Script.writeText("Depots      = {");
		for(int DepotNr = 0; DepotNr < MAX_DEPOTS; DepotNr += 1){
			if(Slot->Depot[DepotNr] != NULL){
				TReadBuffer Buffer(Slot->Depot[DepotNr], Slot->DepotSize[DepotNr]);
				if(!FirstDepot){
					Script.writeText(",");
					Script.writeLn();
					Script.writeText("               ");
				}
				Script.writeNumber(DepotNr);
				Script.writeText(" Content=");
				SaveObjects(&Buffer, &Script);
				FirstDepot = false;
			}
		}
		Script.writeText("}");
		Script.writeLn();
		Script.close();

	}catch(const char *str){
		error("SavePlayerData: Kann Gegenstände des Spielers %u nicht schreiben.\n", Slot->CharacterID);
		error("# Fehler: %s\n", str);
		unlink(FileName);
	}
}

void UnlinkPlayerData(uint32 CharacterID){
	char FileName[4096];
	PlayerDataPath(FileName, sizeof(FileName), CharacterID);
	unlink(FileName);
}

// Player Pool
// =============================================================================
void SavePlayerPoolSlot(TPlayerData *Slot){
	if(Slot == NULL){
		error("SavePlayerPoolSlot: Slot existiert nicht.\n");
		return;
	}

	if(Slot->Dirty){
		SavePlayerData(Slot);
		Slot->Dirty = false;
	}
}

void FreePlayerPoolSlot(TPlayerData *Slot){
	if(Slot == NULL){
		error("FreePlayerPoolSlot: Slot ist NULL.\n");
		return;
	}

	print(3, "Gebe Slot von Charakter %u frei.\n", Slot->CharacterID);
	if(Slot->CharacterID == 0){
		return;
	}

	if(Slot->Sticky > 0){
		error("FreePlayerPoolSlot: Slot wird noch benötigt; darf nicht freigegeben werden.\n");
		return;
	}

	if(Slot->Locked != 0 && Slot->Locked != gettid()){
		error("FreePlayerPoolSlot: Slot ist von einem anderen Thread gesperrt.\n");
		return;
	}

	if(Slot->Dirty){
		SavePlayerData(Slot);
		Slot->Dirty = false;
	}

	delete[] Slot->Inventory;
	for(int DepotNr = 0; DepotNr < MAX_DEPOTS; DepotNr += 1){
		delete[] Slot->Depot[DepotNr];
	}

	Slot->CharacterID = 0;
}

TPlayerData *GetPlayerPoolSlot(uint32 CharacterID){
	if(CharacterID == 0){
		error("GetPlayerPoolSlot: CharacterID ist Null.\n");
		return NULL;
	}

	TPlayerData *Slot = NULL;
	for(int i = 0; i < NARRAY(PlayerDataPool); i += 1){
		if(PlayerDataPool[i].CharacterID == CharacterID){
			Slot = &PlayerDataPool[i];
			break;
		}
	}
	return Slot;
}

TPlayerData *AssignPlayerPoolSlot(uint32 CharacterID, bool DontWait){
	if(CharacterID == 0){
		error("AssignPlayerPoolSlot: CharacterID ist Null.\n");
		return NULL;
	}

	print(3, "Reserviere Slot für Charakter %u.\n", CharacterID);

	// TODO(fusion): `PlayerDataPoolMutex` usage here could be improved, perhaps
	// with some guard class.

	TPlayerData *Slot = NULL;
	while(true){
		PlayerDataPoolMutex.down();
		Slot = GetPlayerPoolSlot(CharacterID);
		if(Slot == NULL){
			break;
		}

		if(Slot->Locked == 0){
			Slot->Locked = gettid();
			PlayerDataPoolMutex.up();
			return Slot;
		}

		if(DontWait){
			Slot->Sticky += 1;
			PlayerDataPoolMutex.up();
			return Slot;
		}

		PlayerDataPoolMutex.up();
		DelayThread(0, 100);
	}

	// NOTE(fusion): Player data for `CharacterID` isn't loaded so we need to
	// assign it one slot and load it.
	ASSERT(Slot == NULL);

	// NOTE(fusion): Try to find an empty slot.
	for(int i = 0; i < NARRAY(PlayerDataPool); i += 1){
		if(PlayerDataPool[i].CharacterID == 0){
			Slot = &PlayerDataPool[i];
			break;
		}
	}

	if(Slot == NULL){
		// NOTE(fusion): Try to find a non-empty slot that is not being used and
		// doesn't need to be saved.
		for(int i = 0; i < NARRAY(PlayerDataPool); i += 1){
			if(PlayerDataPool[i].Locked == 0
					&& PlayerDataPool[i].Sticky == 0
					&& !PlayerDataPool[i].Dirty){
				Slot = &PlayerDataPool[i];
				FreePlayerPoolSlot(Slot);
				break;
			}
		}
	}

	if(Slot == NULL){
		// NOTE(fusion): Try to find a non-empty slot that is not being used.
		for(int i = 0; i < NARRAY(PlayerDataPool); i += 1){
			if(PlayerDataPool[i].Locked == 0 && PlayerDataPool[i].Sticky == 0){
				Slot = &PlayerDataPool[i];
				FreePlayerPoolSlot(Slot);
				break;
			}
		}
	}

	if(Slot == NULL){
		PlayerDataPoolMutex.up();
		error("AssignPlayerPoolSlot: Kein Slot mehr frei.\n");
		return NULL;
	}

	memset(Slot, 0, sizeof(TPlayerData));
	Slot->CharacterID = CharacterID;
	Slot->Locked = gettid();
	PlayerDataPoolMutex.up();

	print(3, "Lade Daten für Spieler %u.\n", CharacterID);

	if(!PlayerDataExists(CharacterID)){
		Log("game", "Spieler %u loggt sich zum ersten Mal ein.\n", CharacterID);
	}

	if(!LoadPlayerData(Slot)){
		Slot->CharacterID = 0;
		Slot->Locked = 0;
		Slot = NULL;
	}

	return Slot;
}

TPlayerData *AttachPlayerPoolSlot(uint32 CharacterID, bool DontWait){
	if(CharacterID == 0){
		error("AttachPlayerPoolSlot: CharacterID ist Null.\n");
		return NULL;
	}

	print(3, "Attache Slot von Spieler %u.\n", CharacterID);

	// TODO(fusion): Same as `AssignPlayerPoolSlot`.

	while(true){
		TPlayerData *Slot = NULL;
		PlayerDataPoolMutex.down();
		Slot = GetPlayerPoolSlot(CharacterID);
		if(Slot == NULL){
			PlayerDataPoolMutex.up();
			error("AttachPlayerPoolSlot: Daten des Charakters sind nicht vorhanden.\n");
			return NULL;
		}

		if(Slot->Locked == 0){
			Slot->Locked = gettid();
			PlayerDataPoolMutex.up();
			return Slot;
		}

		if(DontWait){
			PlayerDataPoolMutex.up();
			return NULL;
		}

		PlayerDataPoolMutex.up();
		DelayThread(0, 100);
	}
}

void AttachPlayerPoolSlot(TPlayerData *Slot, bool DontWait){
	if(Slot == NULL){
		error("AttachPlayerPoolSlot: Slot ist NULL.\n");
		return;
	}

	print(3, "Attache Slot von Spieler %u.\n", Slot->CharacterID);
	while(true){
		PlayerDataPoolMutex.down();
		if(Slot->Locked == 0){
			Slot->Locked = gettid();
			PlayerDataPoolMutex.up();
			return;
		}

		if(DontWait){
			PlayerDataPoolMutex.up();
			return;
		}

		PlayerDataPoolMutex.up();
		DelayThread(0, 100);
	}
}

void IncreasePlayerPoolSlotSticky(TPlayerData *Slot){
	if(Slot == NULL){
		error("IncreasePlayerPoolSlotSticky: Slot ist NULL.\n");
		return;
	}

	PlayerDataPoolMutex.down();
	Slot->Sticky += 1;
	PlayerDataPoolMutex.up();
}

void DecreasePlayerPoolSlotSticky(TPlayerData *Slot){
	if(Slot == NULL){
		error("DecreasePlayerPoolSlotSticky: Slot ist NULL.\n");
		return;
	}

	PlayerDataPoolMutex.down();
	Slot->Sticky -= 1;
	PlayerDataPoolMutex.up();
}

void DecreasePlayerPoolSlotSticky(uint32 CharacterID){
	if(CharacterID == 0){
		error("DecreasePlayerPoolSlotSticky: CharacterID ist Null.\n");
		return;
	}

	PlayerDataPoolMutex.down();
	TPlayerData *Slot = GetPlayerPoolSlot(CharacterID);
	if(Slot != NULL){
		Slot->Sticky -= 1;
	}else{
		error("DecreasePlayerPoolSlotSticky: Slot von Spieler %u nicht gefunden.\n", CharacterID);
	}
	PlayerDataPoolMutex.up();
}

void ReleasePlayerPoolSlot(TPlayerData *Slot){
	if(Slot == NULL){
		error("ReleasePlayerPoolSlot: Slot ist NULL.\n");
		return;
	}

	print(3, "Gebe Slot von Spieler %u frei.\n", Slot->CharacterID);
	if(Slot->Locked == 0){
		error("ReleasePlayerPoolSlot: Slot ist nicht gesperrt.\n");
		return;
	}

	if(Slot->Locked != gettid()){
		error("ReleasePlayerPoolSlot: Slot ist von einem anderen Thread gesperrt.\n");
		return;
	}

	Slot->Locked = 0;
}

void SavePlayerPoolSlots(void){
	time_t Now = time(NULL);
	print(3, "Speichere alle Spielerdaten...\n");
	for(int i = 0; i < NARRAY(PlayerDataPool); i += 1){
		TPlayerData *Slot = &PlayerDataPool[i];
		if(Slot->CharacterID == 0
				|| Slot->Locked != 0
				|| Slot->Sticky > 0
				|| !Slot->Dirty){
			continue;
		}

		// TODO(fusion): Logged out for at least 15 minutes?
		if((Now - Slot->LastLogoutTime) < 900){
			continue;
		}

		AttachPlayerPoolSlot(Slot, true);
		if(Slot->Locked == gettid()){
			SavePlayerPoolSlot(Slot);
			ReleasePlayerPoolSlot(Slot);
		}
	}
}

void InitPlayerPool(void){
	memset(PlayerDataPool, 0, sizeof(PlayerDataPool));
}

void ExitPlayerPool(void){
	// TODO(fusion): I assume the order in `ExitAll` is such to make this work?
	for(int i = 0; i < NARRAY(PlayerDataPool); i += 1){
		TPlayerData *Slot = &PlayerDataPool[i];
		if(Slot->CharacterID == 0){
			continue;
		}

		while(Slot->Locked != 0){
			print(2, "Warte auf Freigabe von Slot %d...\n", i);
			DelayThread(1, 0);
		}

		while(Slot->Sticky > 0){
			DelayThread(1, 0);
			AttachPlayerPoolSlot(Slot, false);
			SendMails(Slot);
			DecreasePlayerPoolSlotSticky(Slot);
			ReleasePlayerPoolSlot(Slot);
		}

		AttachPlayerPoolSlot(Slot, false);
		FreePlayerPoolSlot(Slot);
		ReleasePlayerPoolSlot(Slot);
	}
	print(1, "Alle Spielerdaten gespeichert.\n");
}

// Player Index
// =============================================================================
int GetPlayerIndexEntryNumber(const char *Name, int Position){
	if(Name == NULL){
		error("GetPlayerIndexEntryNumber: Name ist NULL.\n");
		return 0;
	}

	if(Position < (int)strlen(Name)){
		int ch = Name[Position];
		if(ch >= 'A' && ch <= 'Z'){
			return ch - 'A';
		}else if(ch >= 'a' && ch <= 'z'){
			return ch - 'a';
		}
	}
	return 0;
}

void InsertPlayerIndex(TPlayerIndexInternalNode *Node,
		int Position, const char *Name, uint32 CharacterID){
	while(true){
		if(Node == NULL){
			error("InsertPlayerIndex: Node ist NULL.\n");
			return;
		}

		if(Name == NULL){
			error("InsertPlayerIndex: Name ist NULL.\n");
			return;
		}

		int ChildIndex = GetPlayerIndexEntryNumber(Name, Position);
		TPlayerIndexNode *Child = Node->Child[ChildIndex];
		if(Child == NULL){
			TPlayerIndexLeafNode *Leaf = PlayerIndexLeafNodes.getFreeItem();
			memset(Leaf, 0, sizeof(TPlayerIndexLeafNode));
			Leaf->InternalNode = false;
			Leaf->Count = 1;
			strcpy(Leaf->Entry[0].Name, Name);
			Leaf->Entry[0].CharacterID = CharacterID;
			Node->Child[ChildIndex] = Leaf;
			return;
		}

		if(Child->InternalNode){
			Position += 1;
			Node = (TPlayerIndexInternalNode*)Child;
			continue;
		}

		TPlayerIndexLeafNode *ChildLeaf = (TPlayerIndexLeafNode*)Child;
		for(int i = 0; i < ChildLeaf->Count; i += 1){
			// NOTE(fusion): Check if entry is already in the index?
			// TODO(fusion): We should probably be using `stricmp` here.
			if(strcmp(ChildLeaf->Entry[i].Name, Name) == 0){
				return;
			}
		}

		if(ChildLeaf->Count < NARRAY(ChildLeaf->Entry)){
			strcpy(ChildLeaf->Entry[ChildLeaf->Count].Name, Name);
			ChildLeaf->Entry[ChildLeaf->Count].CharacterID = CharacterID;
			ChildLeaf->Count += 1;
			return;
		}

		// NOTE(fusion): Child leaf node is full so we need to create a new
		// internal node and move all previous leaf entries to it. Note that
		// the position is also increased here, so entries should get different
		// child indices.
		TPlayerIndexInternalNode *ChildInternal = PlayerIndexInternalNodes.getFreeItem();
		memset(ChildInternal, 0, sizeof(TPlayerIndexInternalNode));
		ChildInternal->InternalNode = true;
		Node->Child[ChildIndex] = ChildInternal;

		for(int i = 0; i < ChildLeaf->Count; i += 1){
			InsertPlayerIndex(ChildInternal, Position + 1,
					ChildLeaf->Entry[i].Name,
					ChildLeaf->Entry[i].CharacterID);
		}
		PlayerIndexLeafNodes.putFreeItem(ChildLeaf);

		Position += 1;
		Node = ChildInternal;
	}
}

TPlayerIndexEntry *SearchPlayerIndex(const char *Name){
	if(Name == NULL){
		error("SearchPlayerIndex: Name ist NULL.\n");
		return NULL;
	}

	TPlayerIndexNode *Node = &PlayerIndexHead;
	for(int Position = 0; Node->InternalNode; Position += 1){
		int ChildIndex = GetPlayerIndexEntryNumber(Name, Position);
		Node = ((TPlayerIndexInternalNode*)Node)->Child[ChildIndex];
		if(Node == NULL){
			return NULL;
		}
	}

	ASSERT(!Node->InternalNode);
	TPlayerIndexLeafNode *Leaf = (TPlayerIndexLeafNode*)Node;
	for(int i = 0; i < Leaf->Count; i += 1){
		if(stricmp(Leaf->Entry[i].Name, Name) == 0){
			return &Leaf->Entry[i];
		}
	}

	return NULL;
}

bool PlayerExists(const char *Name){
	if(Name == NULL){
		error("PlayerExists: Name ist NULL.\n");
		return false;
	}

	return SearchPlayerIndex(Name) != NULL;
}

uint32 GetCharacterID(const char *Name){
	if(Name == NULL){
		error("GetCharacterID: Name ist NULL.\n");
		return 0;
	}

	uint32 Result = 0;
	TPlayerIndexEntry *Entry = SearchPlayerIndex(Name);
	if(Entry != NULL){
		Result = Entry->CharacterID;
	}
	return Result;
}

const char *GetCharacterName(const char *Name){
	if(Name == NULL){
		error("GetCharacterName: Name ist NULL.\n");
		return NULL;
	}

	const char *Result = "Unknown";
	TPlayerIndexEntry *Entry = SearchPlayerIndex(Name);
	if(Entry != NULL){
		Result = Entry->Name;
	}
	return Result;
}

void InitPlayerIndex(void){
	memset(&PlayerIndexHead, 0, sizeof(TPlayerIndexInternalNode));
	PlayerIndexHead.InternalNode = true;

	// TODO(fusion): The `QueryBufferSize` parameter to `TQueryManagerConnection`
	// is probably related to the size of both these arrays. There gotta be some
	// type of limit per load query or we'd easily overflow these buffers. It may
	// be hardcoded.
	uint32 CharacterIDs[10000];
	char Names[10000][30];
	TQueryManagerConnection QueryManager(360007);
	if(!QueryManager.isConnected()){
		error("InitPlayerIndex: Kann nicht zum Query-Manager verbinden.\n");
		return;
	}

	int MinimumCharacterID = 0;
	while(true){
		int NumberOfPlayers;
		int Ret = QueryManager.loadPlayers(MinimumCharacterID,
				&NumberOfPlayers, Names, CharacterIDs);
		if(Ret != 0){
			error("InitPlayerIndex: Kann Spielerdaten nicht ermitteln.\n");
			break;
		}

		for(int i = 0; i < NumberOfPlayers; i += 1){
			InsertPlayerIndex(&PlayerIndexHead, 0, Names[i], CharacterIDs[i]);
		}

		if(NumberOfPlayers < 10000){
			break;
		}

		MinimumCharacterID = CharacterIDs[9999] + 1;
	}
}

void ExitPlayerIndex(void){
	// no-op
}

// Initialization
// =============================================================================
void InitPlayer(void){
	InitPlayerPool();
	CreatePlayerList(true);
	InitPlayerIndex();
}

void ExitPlayer(void){
	ExitPlayerPool();
	CreatePlayerList(false);
	ExitPlayerIndex();
}
