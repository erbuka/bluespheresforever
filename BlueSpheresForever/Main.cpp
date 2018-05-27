#include <iostream>
#include <string>
#include "GameData.h"
#include "GameRendering.h"
#include "InterfaceRendering.h"
#include "MatrixStack.h"
#include "ShaderManager.h"
#include "Timer.h"
#include "GameLogic.h"
#include "AudioEngine.h"
#include "DeferredRendering.h"
#include "StageGenerator.h"
#include "GameMenu.h"
#include "Global.h"
#include "GLWindow.h"

// #define CONVERT_MODELS

#define SKYBOX_SIZE 30;

#define RND(i) ((float) rand() / RAND_MAX * (i))

using namespace std;
using namespace glutils;
using namespace GameMenu;

void InitGL(); // OpenGL init routine
void Render(); // Rendering routine

void InitTextures(); // Intialize textures
void InitThreading(); // Create threads
DWORD WINAPI Animate(LPVOID param); // Animation Routine

// Game components
GameLogic * gAnimation = NULL;
GameData * gGameData = NULL;
GameRendering * gGameRendering = NULL;
InterfaceRendering * gInterfaceRendering = NULL;
AudioEngine * gAudioEngine = NULL;
DeferredRendering * gDeferredRendering = NULL;

// Game menu
GameMenuParentItem * mainMenuRoot;
GameMenuParentItem * activeMenu;

// Program context
Global::ProgramContext programContext = Global::CTX_DISCLAIMER;
Global::GameMode gameMode;
Global::GameConfig gameConfig;
Global::GameStats gameStats;
vector<string> stageFiles;
bool showGameStats = false;


// Threading
HANDLE animMutex;
HANDLE animThread;

GLuint texChecker = 0, texStarSphere = 0, texNormalMap = 0;

GLfloat viewWidth = 640, viewHeight = 480;

LightingParameters pLighting;
ViewParameters pView;
ProjectionParameters pProjection;
TextureParameters pTexture;

class MainLogicListener : public GameLogicListener {
public:
	void InGameMessage(string message) {
		gInterfaceRendering->DisplayInGameMessage(message);
	}
};

class MainMenuActionListener: public GameMenuActionListener {
	void ActionPerformed(int action, GameMenuItem * source) {
		GameMenuSelectItem * selectMenu = dynamic_cast<GameMenuSelectItem*>(source);
		GameMenuInputItem * inputMenu = dynamic_cast<GameMenuInputItem*>(source);
		if(inputMenu) {
			inputMenu->SetInputMode(!inputMenu->IsInputMode());

			if(inputMenu->GetID() == MENU_NEWGAME_INSERTCODE) {
				GameMenuCodeInputItem * inputCodeMenu = dynamic_cast<GameMenuCodeInputItem*>(inputMenu);

				inputCodeMenu->SetSelectedIndex(0);

				if(!inputMenu->IsInputMode()) {
					int stage = StageGenerator::GetStageNumber(inputCodeMenu->GetCode());
					if(stage == -1) {
						gAudioEngine->PlayFX(AudioEngine::FX_WRONG);
						inputCodeMenu->SetCode(StageGenerator::GetStageCode(dynamic_cast<GameMenuCurrentLevel*>(GameMenu::GetMenuById(MENU_NEWGAME_START))->GetCurrentLevel()));
					} else {
						gAudioEngine->PlayFX(AudioEngine::FX_OK);
						dynamic_cast<GameMenuCurrentLevel*>(GameMenu::GetMenuById(MENU_NEWGAME_START))->SetCurrentLevel(stage);
					}
				}
			}
		} else if(selectMenu) {
			switch(selectMenu->GetID()) {
				case MENU_VIDEO_STEREO3D:
					gameConfig.stereo3D = *((int*)selectMenu->GetSelectedValue());
					break;
				case MENU_VIDEO_SHADOWQUALITY:
					gameConfig.shadowQuality = *((int*)selectMenu->GetSelectedValue());
					break;
				case MENU_VIDEO_SHADOWSENABLED:
					gameConfig.shadowsEnabled = *((int*)selectMenu->GetSelectedValue());
					break;
				case MENU_VIDEO_FIRST_PERSON:
					gameConfig.isFirstPerson = *((int*)selectMenu->GetSelectedValue());
					break;
				case MENU_AUDIO_MUSIC_VOLUME:
					gameConfig.musicVolume = *((int*)selectMenu->GetSelectedValue());
					gAudioEngine->Configure(gameConfig);
					break;
				case MENU_AUDIO_FX_VOLUME:
					gameConfig.fxVolume = *((int*)selectMenu->GetSelectedValue());
					gAudioEngine->Configure(gameConfig);
					break;
			}
		} else if(source->IsLeaf()) {
			switch(source->GetID()) {
				case MENU_EXIT:
					GLWindow::Dispose();
					exit(0);
					break;
				case MENU_OPTIONS_BACK:
					activeMenu = activeMenu->GetParent();
					activeMenu->SetSelectedChild(0);
					gInterfaceRendering->SetActiveMenu(activeMenu);
					Global::StoreGameConfig(gameConfig);
					break;
				case MENU_CUSTOM_BACK:
				case MENU_VIDEO_BACK:
				case MENU_AUDIO_BACK:
				case MENU_NEWGAME_BACK:
					activeMenu = activeMenu->GetParent();
					activeMenu->SetSelectedChild(0);
					gInterfaceRendering->SetActiveMenu(activeMenu);
					break;
				case MENU_NEWGAME_START:
					WaitForSingleObject(animMutex, INFINITE);

					if(gGameData != NULL)
						delete gGameData;
					if(gAnimation != NULL)
						delete gAnimation;

					gDeferredRendering->Configure(gameConfig);
					
					gGameData = StageGenerator::GetStage(dynamic_cast<GameMenuCodeInputItem*>(GameMenu::GetMenuById(MENU_NEWGAME_INSERTCODE))->GetCode());


					gGameRendering->GetSkyVertexArray()->InitStarColors(gGameData->GetStarsColor(0), gGameData->GetStarsColor(1));

					gAnimation = new GameLogic(gGameData, gAudioEngine);
					gAnimation->AddGameLogicListener(new MainLogicListener());
					gAnimation->SetPosition(vec2(gGameData->GetStartPoint()));
					gAnimation->SetDirection(gGameData->GetStartDirection());

					InitTextures();

					gAudioEngine->StopTitleScreen();

					gameMode = Global::MODE_BLUESPHERES;
					programContext = Global::CTX_GAME;

					ReleaseMutex(animMutex);

					break;

				case MENU_CUSTOM_STARTGAME:

					WaitForSingleObject(animMutex, INFINITE);


					if(gGameData != NULL)
						delete gGameData;
					if(gAnimation != NULL)
						delete gAnimation;
					
					int * stage = (int*) dynamic_cast<GameMenuSelectItem*>(GameMenu::GetMenuById(MENU_CUSTOM_SELECTSTAGE))->GetSelectedValue();

					gDeferredRendering->Configure(gameConfig);

					gGameData = GameData::LoadData(Global::DATA_DIR + "/" + stageFiles[*stage]);

					gGameRendering->GetSkyVertexArray()->InitStarColors(gGameData->GetStarsColor(0), gGameData->GetStarsColor(1));

					gAnimation = new GameLogic(gGameData, gAudioEngine);
					gAnimation->AddGameLogicListener(new MainLogicListener());
					gAnimation->SetPosition(vec2(gGameData->GetStartPoint()));
					gAnimation->SetDirection(gGameData->GetStartDirection());

					InitTextures();

					gAudioEngine->StopTitleScreen();

					gameMode = Global::MODE_CUSTOMSTAGE;
					programContext = Global::CTX_GAME;

					ReleaseMutex(animMutex);
					break;
			}
		} else {
			activeMenu = (GameMenuParentItem*)source;
			activeMenu->SetSelectedChild(0);
			gInterfaceRendering->SetActiveMenu(activeMenu);
		}
	}
};

class MainWindowListener :  public GLWindowListener {
public:
	void OnKeyDown(int key) {

		if(key == VK_TAB) 
			showGameStats = ! showGameStats;

		if(programContext == CTX_MAIN_MENU) {
			GameMenuItem * selectedMenu = activeMenu->GetChildren()[activeMenu->GetSelectedChild()];
			GameMenuSelectItem * selectedMenuSelect = dynamic_cast<GameMenuSelectItem*>(selectedMenu);
			GameMenuInputItem * selectedInputMenu = dynamic_cast<GameMenuInputItem*>(selectedMenu);
			switch(key) {
				case VK_UP:
					if(selectedInputMenu && selectedInputMenu->IsInputMode())
						break;
					activeMenu->SetSelectedChild(max(activeMenu->GetSelectedChild() - 1, 0));
					break;
				case VK_DOWN:
					if(selectedInputMenu && selectedInputMenu->IsInputMode())
						break;
					activeMenu->SetSelectedChild(min(activeMenu->GetSelectedChild() + 1, activeMenu->GetChildCount() - 1));
					break;
				case VK_LEFT:
					if(selectedMenuSelect)
						selectedMenuSelect->SetSelectedOption(max(0, selectedMenuSelect->GetSelectedOption() - 1));

					if(selectedInputMenu && selectedInputMenu->IsInputMode()) {
						selectedInputMenu->SetSelectedIndex(selectedInputMenu->GetSelectedIndex() -1);
					}
					break;
				case VK_RIGHT:
					if(selectedMenuSelect)
						selectedMenuSelect->SetSelectedOption(min(selectedMenuSelect->GetOptionCount() - 1, selectedMenuSelect->GetSelectedOption() + 1));

					if(selectedInputMenu && selectedInputMenu->IsInputMode()) {
						selectedInputMenu->SetSelectedIndex(selectedInputMenu->GetSelectedIndex() + 1);
					}
					break;
				case VK_RETURN:
					selectedMenu->FireAction(0);
					break;
				default:
					if(selectedInputMenu && selectedInputMenu->IsInputMode())
						selectedInputMenu->SetCurrentCharacter(key);
					break;
			}
		} else if(programContext == CTX_GAME) {
			switch(key) {
				case VK_UP:
					gAnimation->SetRunForward();
					break;
				case VK_LEFT:
					gAnimation->SetRotate(GameLogic::ROTATE_LEFT);
					break;
				case VK_RIGHT:
					gAnimation->SetRotate(GameLogic::ROTATE_RIGHT);
					break;
				case VK_SPACE:
					gAnimation->SetJump();
					break;
			}
		} else if(programContext == Global::CTX_STAGE_CLEAR) {
			if(key == VK_RETURN) {

				WaitForSingleObject(animMutex, INFINITE);
				
				int stage = dynamic_cast<GameMenuCurrentLevel*>(GameMenu::GetMenuById(MENU_NEWGAME_START))->GetCurrentLevel();

				if(gGameData->IsPerfect())
					stage += 10;
				else
					stage += 1;

				__int64 code = StageGenerator::GetStageCode(stage);

				dynamic_cast<GameMenuCurrentLevel*>(GameMenu::GetMenuById(MENU_NEWGAME_START))->SetCurrentLevel(stage);
				dynamic_cast<GameMenuCodeInputItem*>(GameMenu::GetMenuById(MENU_NEWGAME_INSERTCODE))->SetCode(code);

				gAudioEngine->StopStageClear();

				if(gGameData != NULL)
					delete gGameData;

				if(gAnimation != NULL)
					delete gAnimation;

				gDeferredRendering->Configure(gameConfig);
				gGameData = StageGenerator::GetStage(code);
				gAnimation = new GameLogic(gGameData, gAudioEngine);
				gAnimation->AddGameLogicListener(new MainLogicListener());
				gAnimation->SetPosition(vec2(gGameData->GetStartPoint()));
				gAnimation->SetDirection(gGameData->GetStartDirection());

				InitTextures();

				programContext = Global::CTX_GAME;

				ReleaseMutex(animMutex);

			}
		}
	}
	void OnKeyUp(int key) {}
	void OnResize(int width, int height)  {
		viewWidth = width;
		viewHeight = height;
		glViewport(0,0,width,height); // viewport resize
		gDeferredRendering->ResizeViewport(viewWidth, viewHeight);
		gInterfaceRendering->ResizeViewport(viewWidth, viewHeight);
	}
	void OnClose()  {
		GLWindow::Dispose();
		exit(0);
	}
};

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow) {


	#ifdef CONVERT_MODELS
		Global::ConvertModels();
	#endif
	
	Global::InitializeLog();
	gameConfig = Global::LoadGameConfig();
	stageFiles = Global::LoadStageFiles();
	GameData::Initialize();

	viewWidth = gameConfig.displayMode.width;
	viewHeight = gameConfig.displayMode.height;

	GLWindow::Initialize(hInstance, gameConfig);

	Global::CheckGLVersion();

	ShowWindow(GLWindow::hWnd, SW_SHOW);
	SetForegroundWindow(GLWindow::hWnd);
	SetFocus(GLWindow::hWnd);

	glewInit();
	InitGL();
	InitThreading();

	GLWindow::AddWindowListener(new MainWindowListener());


	MSG msg;

	while(1) {
		if(PeekMessage(&msg, GLWindow::hWnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		Render();
	}
}



DWORD WINAPI Animate(LPVOID param) {
	static float FREQUENCY = 500; // hz
	static int MAX_SLEEP = (int)(1000.0f / FREQUENCY);
	static Timer * timer = new Timer();
	static Timer * executionTimer = new Timer();
	static int executionTime = 0;
	static float dt = 0;
	while(1) {

		executionTimer->restart();
		dt = timer->dtSecs();

		WaitForSingleObject(animMutex, INFINITE);

		if(programContext == CTX_GAME && gAnimation != NULL) {
			gAnimation->Animate(dt);
		}

		ReleaseMutex(animMutex);

		executionTime = executionTimer->dtMillis();

		if(showGameStats)
			Global::CollectGameStats(gameStats, executionTime, false, true);


		Sleep(max(MAX_SLEEP - executionTime, 0));
	}
}

// called when window is drawn
void Render() {
	static Timer * timer = new Timer();
	static Timer * executionTimer = new Timer();
	static float dt = 0.0f;
	static float time;
	static MatrixStack * stack = new MatrixStack(4);

	dt = timer->dtSecs();
	executionTimer->restart();

	if(programContext == Global::CTX_DISCLAIMER) {
		glClear(GL_COLOR_BUFFER_BIT);
		gInterfaceRendering->DrawGameDisclaimer(dt, false);
		if(time >= InterfaceRendering::DISCLAIMER_TIME)
			programContext = Global::CTX_INTRO;
	} else if(programContext == Global::CTX_INTRO) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gInterfaceRendering->DrawGameIntro(dt, false);
		if(time >= InterfaceRendering::DISCLAIMER_TIME + InterfaceRendering::INTRO_TIME) {
			programContext = Global::CTX_MAIN_MENU;
			activeMenu = mainMenuRoot;
			activeMenu->SetSelectedChild(0);
			gInterfaceRendering->SetActiveMenu(activeMenu);
			gInterfaceRendering->DrawGameMenu(0, true);
		}
	} else if(programContext == Global::CTX_STAGE_CLEAR) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		int level = dynamic_cast<GameMenuCurrentLevel*>(GameMenu::GetMenuById(MENU_NEWGAME_START))->GetCurrentLevel();
		gInterfaceRendering->DrawStageClear(dt, level, gGameData->IsPerfect(), false);
	} else if(programContext == Global::CTX_MAIN_MENU) {
		glClear(GL_COLOR_BUFFER_BIT);
		gInterfaceRendering->DrawGameMenu(dt, false);
	} else if(programContext == Global::CTX_GAME) {

		// Set up the view
		if (gameConfig.isFirstPerson)
		{
			pView.rotateX = PI / 6;
			pView.cameraPosition.z = 0;
			pView.cameraPosition.x = 0;
			pView.cameraPosition.y = -0.75 - gAnimation->GetSonicHeight();
		}
		else
		{
			pView.rotateX = Global::CAMERA_ROTATE_X;
			pView.cameraPosition.z = -2.5;
			pView.cameraPosition.x = 0;
			pView.cameraPosition.y = -0.5;
		}

		// Game Rendering

		glClear(GL_COLOR_BUFFER_BIT);

		gDeferredRendering->SetLightingParameters(pLighting);
		gDeferredRendering->SetProjectionParameters(pProjection);
		gDeferredRendering->SetTextureParameters(pTexture);
		
		WaitForSingleObject(animMutex, INFINITE);

		if(gameConfig.shadowsEnabled)
			gDeferredRendering->RenderShadowMap(gGameRendering, gAnimation, gGameData);


		if(gameConfig.stereo3D == Global::SWITCH_ON) { // 3D interlaced rendering with stencil buffering

			gDeferredRendering->InterlaceStencil();
			
			glEnable(GL_STENCIL_TEST);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

			// Right eye

			glStencilFunc(GL_NOTEQUAL, 1, 1);

			stack->loadIdentity();
			stack->Stereo(Global::CAMERA_FOVY, viewWidth / viewHeight, Global::CAMERA_NEAR, Global::CAMERA_FAR, Global::CAMERA_STEREO_IOD / 2, Global::CAMERA_STEREO_SCREENZ);

			pView.cameraPosition.x = Global::CAMERA_STEREO_IOD / 2;
			pView.projectionMatrix = stack->current();

			gDeferredRendering->SetViewParamters(pView);

			gDeferredRendering->RenderBuffers(gGameRendering, gAnimation, gGameData);

			gDeferredRendering->RenderAlbedoAndShadows();
		
			if(gameConfig.shadowsEnabled)
				gDeferredRendering->RenderBlurredShadows();
			/*
			if(gameConfig.quality != 0)
				gDeferredRendering->RenderBloomLighting();*/
		
			gDeferredRendering->Render();

			glClear(GL_DEPTH_BUFFER_BIT);

			// Left eye

			glStencilFunc(GL_EQUAL, 1, 1);

			stack->loadIdentity();
			stack->Stereo(Global::CAMERA_FOVY, viewWidth / viewHeight, Global::CAMERA_NEAR, Global::CAMERA_FAR, -Global::CAMERA_STEREO_IOD / 2, Global::CAMERA_STEREO_SCREENZ);

			pView.cameraPosition.x = -Global::CAMERA_STEREO_IOD/2;
			pView.projectionMatrix = stack->current();

			gDeferredRendering->SetViewParamters(pView);

			gDeferredRendering->RenderBuffers(gGameRendering, gAnimation, gGameData);

			gDeferredRendering->RenderAlbedoAndShadows();
		
			if(gameConfig.shadowsEnabled)
				gDeferredRendering->RenderBlurredShadows();

			/*
			if(gameConfig.quality != 0)
				gDeferredRendering->RenderBloomLighting();*/
		
			gDeferredRendering->Render();

			glDisable(GL_STENCIL_TEST);

		} else { // normal rendering
			stack->loadIdentity();
			stack->perspective(Global::CAMERA_FOVY, viewWidth / viewHeight, Global::CAMERA_NEAR, Global::CAMERA_FAR);

			pView.cameraPosition.x = 0;
			pView.projectionMatrix = stack->current();

			gDeferredRendering->SetViewParamters(pView);

			gDeferredRendering->RenderBuffers(gGameRendering, gAnimation, gGameData);

			gDeferredRendering->RenderAlbedoAndShadows();
			
			if(gameConfig.shadowsEnabled)
				gDeferredRendering->RenderBlurredShadows();

			if(gameConfig.quality != 0)
				gDeferredRendering->RenderBloomLighting();
		
			gDeferredRendering->Render();

		}

		ReleaseMutex(animMutex);

		// Game Interface

		glPushAttrib(GL_ENABLE_BIT);
		glEnable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		gInterfaceRendering->DrawGameInterface(gGameData->GetCollectedRings(),
			gGameData->GetObjectCount(GameData::C_BLUESPHERE), dt);

		if (!gameConfig.isFirstPerson)
		{
			gInterfaceRendering->DrawRingSprite(gAnimation->GetRingSpriteTimer());
		}

		glPopAttrib();

		if(gAnimation->GetGameStatus() == GameLogic::STATUS_STARTING || 
			gAnimation->GetGameStatus() == GameLogic::STATUS_GAMEOVER ||
			gAnimation->GetGameStatus() == GameLogic::STATUS_FINISHED) {
				gInterfaceRendering->DrawFading(vec3(1,1,1), gAnimation->GetFadingFactor(), GL_ONE);
		}

		if(gAnimation->GetGameStatus() == GameLogic::STATUS_FINISHED) {

			if(gameMode == Global::MODE_BLUESPHERES) {
				
				if(gGameData->IsClear()) {

					gInterfaceRendering->DrawStageClear(0, 0, false, true);

					programContext = CTX_STAGE_CLEAR;
				} else {
					programContext = Global::CTX_INTRO;
					time = InterfaceRendering::DISCLAIMER_TIME;
					gInterfaceRendering->DrawGameIntro(0, true);
				}
			} else {

				programContext = Global::CTX_INTRO;
				time = InterfaceRendering::DISCLAIMER_TIME;
				gInterfaceRendering->DrawGameIntro(0, true);
			}
		}

	}

	if(showGameStats) {
		glPushAttrib(GL_ENABLE_BIT);
			glEnable(GL_BLEND);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			gInterfaceRendering->DrawGameStats(gameStats);
		glPopAttrib();
	}



	time += dt;
	glFlush();
	glFinish();
	if(showGameStats)
		Global::CollectGameStats(gameStats, executionTimer->dtMillis(), true, false);
	SwapBuffers(GLWindow::hDC);
}

void InitGL() {
	// Init opengl(depth test, blending, lighting and so on...)

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glClearColor(0,0,0,0);
	glDisable(GL_BLEND);

	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
	glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);

	//glPolygonMode(GL_FRONT, GL_LINE);

	mainMenuRoot = (GameMenuParentItem*)BuildGameMenu(new MainMenuActionListener(), gameConfig, stageFiles);
	activeMenu = mainMenuRoot;

	gAudioEngine = new AudioEngine();
	gAudioEngine->Configure(gameConfig);

	gDeferredRendering = new DeferredRendering(viewWidth, viewHeight);
	gDeferredRendering->ResizeViewport(viewWidth, viewHeight);

	gInterfaceRendering = new InterfaceRendering(gAudioEngine);
	gInterfaceRendering->Configure(gameConfig);
	gInterfaceRendering->SetActiveMenu(activeMenu);
	gInterfaceRendering->ResizeViewport(gameConfig.displayMode.width, gameConfig.displayMode.height);

	gGameRendering = new GameRendering();
	gGameRendering->Configure(gameConfig);
	
	pLighting.position = vec4(0, 5, 0, 1);
	pLighting.shadowPosition = vec4(0, 3, 0, 1);
	pLighting.ambient = vec4(0.2, 0.2, 0.2, 1);
	pLighting.diffuse = vec4(1.0, 0.95, 0.8, 1);
	pLighting.specular = vec4(1.0, 0.95, 0.8, 1);
	pLighting.attenuation = vec3(0.05, 1.0, 8.0);
	pLighting.bloomExposure = .4f;

	pProjection.planeCenter = vec4(0.0, -8.0, 0.0, 1.0);
	pProjection.planeNormal = vec4(0.0, 1.0, 0.0, 0.0);
	pProjection.planeTangent = vec4(1.0, 0.0, 0.0, 0.0);
}

void InitThreading() {
	animMutex = CreateMutex(NULL, FALSE, "AnimMutex");
	animThread = CreateThread(NULL, 0, Animate, NULL, 0, NULL);
}

void InitTextures() {
	
	if(texChecker != 0) {
		glDeleteTextures(1, &texChecker);
		texChecker = 0;
	}
	
	if(texNormalMap != 0) {
		glDeleteTextures(1, &texNormalMap);
		texNormalMap = 0;
	}

	if(gGameData->GetFloorRenderingMode() == GameData::RM_TEXTURE)
		texChecker = Global::PngTexture((Global::TEXTURE_DIR + "/" + gGameData->GetTexture()).c_str(), GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	if(gGameData->IsBumpMappingEnabled() == GameData::AS_YES)
		texNormalMap = Global::PngTexture((Global::TEXTURE_DIR + "/" + gGameData->GetBumpMap()).c_str(), GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	if(texStarSphere == 0) {
		texStarSphere = Global::PngTexture((Global::TEXTURE_DIR + "/star1.png").c_str(), GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	}

	pTexture.floorDiffuse = texChecker;
	pTexture.floorNormalMap = texNormalMap;
	pTexture.starSphere = texStarSphere;
}