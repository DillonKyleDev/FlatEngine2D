#include "FlatEngine.h"


/*
######################################
######							######
######         Engine           ######
######                          ######
######################################
*/


namespace FlatEngine
{
	//Variables
	int FlatEngine::FocusedGameObjectIndex = -1;
	FlatEngine::SceneManager* FlatEngine::sceneManager = new FlatEngine::SceneManager();
	FlatEngine::Logger* FlatEngine::logger = new FlatEngine::Logger();
	FlatEngine::GameLoop* FlatEngine::gameLoop = new FlatEngine::GameLoop();

	// Colors
	ImU32 White = IM_COL32(255, 255, 255, 255);
	ImU32 Green = IM_COL32(1, 255, 1, 255);

	// FlatEngine
	void FlatEngine::Run(bool& _hasQuit)
	{
		// Keeping track of the frames passed since we started the GameLoop
		if (FlatEngine::GameLoopStarted() && !FlatEngine::GameLoopPaused())
		{
			FlatEngine::GameLoopUpdate();
		}

		FlatEngine::FlatGui::Render(_hasQuit);
	}

	void FlatEngine::SetFocusedGameObjectIndex(int index)
	{
		FlatEngine::FocusedGameObjectIndex = index;
	}

	int FlatEngine::GetFocusedGameObjectIndex()
	{
		return FlatEngine::FocusedGameObjectIndex;
	}


	std::vector<FlatEngine::GameScript*> gameScripts = {};


	// Scene Manager Prettification
	Scene* FlatEngine::GetLoadedScene()
	{
		return FlatEngine::sceneManager->GetLoadedScene();
	}

	void FlatEngine::SaveScene(Scene *scene)
	{
		FlatEngine::sceneManager->SaveScene(scene);
	}

	void FlatEngine::LoadScene(std::string name)
	{
		FlatEngine::sceneManager->LoadScene(name);
	}
	

	// Scene Prettification
	std::vector<GameObject*> FlatEngine::GetSceneObjects()
	{
		return FlatEngine::GetLoadedScene()->GetSceneObjects();
	}

	void FlatEngine::CreateGameObject()
	{
		FlatEngine::GetLoadedScene()->CreateGameObject();
	}

	void FlatEngine::DeleteGameObject(int sceneObjectIndex)
	{
		FlatEngine::GetLoadedScene()->DeleteGameObject(sceneObjectIndex);
	}


	Component* FlatEngine::GetObjectComponent(long objectID, ComponentTypes type)
	{
		return FlatEngine::GetLoadedScene()->GetObjectById(objectID)->GetComponent(type);
	}

	FlatEngine::GameObject* GetObjectById(long objectID)
	{
		return FlatEngine::GetLoadedScene()->GetObjectById(objectID);
	}


	// Logging
	void FlatEngine::LogString(std::string line)
	{
		logger->LogString(line);
	}

	void FlatEngine::LogFloat(float var, std::string line)
	{
		logger->LogFloat(var, line);
	}

	void FlatEngine::LogInt(int var, std::string line)
	{
		logger->LogInt(var, line);
	}

	void FlatEngine::LogVector2(Vector2 vector, std::string line)
	{
		logger->LogVector2(vector, line);
	}

	void FlatEngine::DrawRectangle(Vector2 startingPoint, Vector2 endingPoint, ImVec2 canvas_p0, ImVec2 canvas_sz, ImU32 color, float thickness, ImDrawList* drawList)
	{
		if (startingPoint.x < (canvas_sz.x + canvas_p0.x) && endingPoint.x > canvas_p0.x && startingPoint.y < (canvas_sz.y + canvas_p0.y) && endingPoint.y >(canvas_p0.y))
		{
			if (startingPoint.x < canvas_p0.x)
				startingPoint.x = canvas_p0.x;
			if (endingPoint.x > canvas_p0.x + canvas_sz.x)
				endingPoint.x = canvas_p0.x + canvas_sz.x;
			if (startingPoint.y < canvas_p0.y)
				startingPoint.y = canvas_p0.y;
			if (endingPoint.y > canvas_p0.y + canvas_sz.y)
				endingPoint.y = canvas_p0.y + canvas_sz.y;

			logger->DrawRectangle(startingPoint, endingPoint, color, thickness, drawList);
		}
	}

	void FlatEngine::DrawRectangle(ImVec2 startingPoint, ImVec2 endingPoint, ImVec2 canvas_p0, ImVec2 canvas_sz, ImU32 color, float thickness, ImDrawList* drawList)
	{
		if (startingPoint.x < (canvas_sz.x + canvas_p0.x) && endingPoint.x > canvas_p0.x && startingPoint.y < (canvas_sz.y + canvas_p0.y) && endingPoint.y > (canvas_p0.y))
		{
			if (startingPoint.x < canvas_p0.x)
				startingPoint.x = canvas_p0.x;
			if (endingPoint.x > canvas_p0.x + canvas_sz.x)
				endingPoint.x = canvas_p0.x + canvas_sz.x;
			if (startingPoint.y < canvas_p0.y)
				startingPoint.y = canvas_p0.y;
			if (endingPoint.y > canvas_p0.y + canvas_sz.y)
				endingPoint.y = canvas_p0.y + canvas_sz.y;

			logger->DrawRectangle(startingPoint, endingPoint, color, thickness, drawList);
		}
	}

	void FlatEngine::DrawLine(ImVec2 startingPoint, ImVec2 endingPoint, ImU32 color, float thickness, ImDrawList* drawList)
	{
		logger->DrawLine(startingPoint, endingPoint, color, thickness, drawList);
	}

	void FlatEngine::DrawLine(Vector2 startingPoint, Vector2 endingPoint, ImU32 color, float thickness, ImDrawList* drawList)
	{
		logger->DrawLine(startingPoint, endingPoint, color, thickness, drawList);
	}

	void FlatEngine::DrawPoint(ImVec2 point, ImU32 color, ImDrawList* drawList)
	{
		logger->DrawPoint(point, color, drawList);
	}

	void FlatEngine::DrawPoint(Vector2 point, ImU32 color, ImDrawList* drawList)
	{
		logger->DrawPoint(point, color, drawList);
	}


	// Game Loop prettification
	void FlatEngine::StartGameLoop()
	{
		gameLoop->Start();
	}

	void FlatEngine::GameLoopUpdate()
	{
		gameLoop->Update();
	}

	void FlatEngine::PauseGameLoop()
	{
		gameLoop->Pause();
	}

	void FlatEngine::StopGameLoop()
	{
		gameLoop->Stop();
	}

	int FlatEngine::GetEllapsedGameTime()
	{
		return gameLoop->TimeEllapsed();
	}

	bool FlatEngine::GameLoopStarted()
	{
		return gameLoop->IsStarted();
	}

	bool FlatEngine::GameLoopPaused()
	{
		return gameLoop->IsPaused();
	}

	float FlatEngine::GetAverageFps()
	{
		return gameLoop->GetAverageFps();
	}

	float FlatEngine::GetDeltaTime()
	{
		return gameLoop->GetDeltaTime();
	}

	//ImVec4 objectA(top, right, bottom, left), ImVec4 objectB(top, right, bottom, left)
	bool FlatEngine::AreColliding(ImVec4 ObjectA, ImVec4 ObjectB)
	{
		float A_TopEdge = ObjectA.x;
		float A_RightEdge = ObjectA.y;
		float A_BottomEdge = ObjectA.z;
		float A_LeftEdge = ObjectA.w;

		float B_TopEdge = ObjectB.x;
		float B_RightEdge = ObjectB.y;
		float B_BottomEdge = ObjectB.z;
		float B_LeftEdge = ObjectB.w;

		return (A_LeftEdge < B_RightEdge && A_RightEdge > B_LeftEdge && A_TopEdge > B_BottomEdge && A_BottomEdge < B_TopEdge);
	}
}