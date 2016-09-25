#include "AAExample.h"
#include <iostream>
#include <Windows.h>

using namespace BWAPI;
using namespace Filter;

// AA: Do a blackboard, don't use it directly! (or at least don't use it static, use a singleton)
static bool GameOver;
HANDLE ghMutex;

static int enemyPositionX = 50;
static int enemyPositionY = 50;

Unitset minerals;

Unit enemyCommandCenter = NULL;

bool lockMineralsUnitset = false;

void AAExample::onStart()
{
	GameOver = false;

	// Create a mutex with no initial owner
	ghMutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex

	// Hello World!
	Broodwar->sendText("My badass agent is ready for battle 3!");
	
	int startPointX = Broodwar->self()->getStartLocation().x;
	int startPointY = Broodwar->self()->getStartLocation().y;
	
	if (startPointX < 25 && startPointY < 25) {
		enemyPositionX = 1792;
		enemyPositionY = 1872;
	}

	//Broodwar->sendText(std::to_string(Broodwar->self()->getStartLocation().x).c_str());
	//Broodwar->sendText(std::to_string(Broodwar->self()->getStartLocation().y).c_str());

	// Set the command optimization level so that common commands can be grouped
	// and reduce the bot's APM (Actions Per Minute).
	Broodwar->setCommandOptimizationLevel(2);

	// Retrieve you and your enemy's races. enemy() will just return the first enemy. AA: Not a problem with just 2.
	// If you wish to deal with multiple enemies then you must use enemies().
	//if (Broodwar->enemy()) // First make sure there is an enemy AA: Make sure to check EVERYTHING.
	//	Broodwar << "The matchup is " << Broodwar->self()->getRace() << " vs " << Broodwar->enemy()->getRace() << std::endl;

}

// For the hardcore ones, you can use Thread Pool closing it at onEnd.
void AAExample::onEnd(bool isWinner)
{
	// Called when the game ends
	GameOver = true;
	if (isWinner)
	{
		// AA: Do whatever you want, but make sure to not break anything.
		// Log your win here!
	}
	// you don't need it if you end your threads without GameOver.
	Sleep(100); // Enough time to end the threads.
}

void AAExample::onFrame()
{
	// Called once every game frame

	// Display the game frame rate as text in the upper left area of the screen
	//Broodwar->drawTextScreen(200, 0, "FPS: %d", Broodwar->getFPS());
	//Broodwar->drawTextScreen(200, 20, "Average FPS: %f", Broodwar->getAverageFPS());

}

void AAExample::onSendText(std::string text)
{
	// Send the text to the game if it is not being processed.
	Broodwar->sendText("%s", text.c_str());
	// Make sure to use %s and pass the text as a parameter,
	// otherwise you may run into problems when you use the %(percent) character!
}

void AAExample::onReceiveText(BWAPI::Player player, std::string text)
{ // AA: You won't need it because we don't have a team game, but it can change.
	// Parse the received text
	Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
}

void AAExample::onPlayerLeft(BWAPI::Player player)
{
	// Interact verbally with the other players in the game by
	// announcing that the other player has left.
	//Broodwar->sendText("Goodbye %s!", player->getName().c_str());
}

void AAExample::onNukeDetect(BWAPI::Position target)
{
	// AA: If you can make a nuke in 10 minutes, good for you.
	// Check if the target is a valid position
	if (target)
	{
		// if so, print the location of the nuclear strike target
		Broodwar << "Nuclear Launch Detected at " << target << std::endl;
	}
	else
	{
		// Otherwise, ask other players where the nuke is!
		Broodwar->sendText("Where's the nuke?");
	}
	// You can also retrieve all the nuclear missile targets using Broodwar->getNukeDots()!
}

void AAExample::onUnitDiscover(BWAPI::Unit unit)
{
	if (unit->getType().isMineralField()) 
	{
		if (minerals.find(unit) != minerals.end()) 
		{
			minerals.insert(unit);
		}
	}
	else if (unit->getType().isResourceDepot()) 
	{
		Player player = unit->getPlayer();
		if (unit->getPlayer()->isEnemy(player)) {
			enemyCommandCenter = unit;
		}
	}
}

void AAExample::onUnitEvade(BWAPI::Unit unit)
{
}

void AAExample::onUnitShow(BWAPI::Unit unit)
{
}

void AAExample::onUnitHide(BWAPI::Unit unit)
{
}

void AAExample::onUnitCreate(BWAPI::Unit unit)
{
	// For each created unit...
	if (unit->getType().isWorker()) // or  == BWAPI::UnitTypes::Terran_SCV if terran
		CreateThread(NULL, 0, thisShouldBeAClassButImTooLazyToDoIt_Worker, (LPVOID)unit, 0, NULL);
	// You can do a direct comparison like  == BWAPI::UnitTypes::Terran_Command_Center too.
	else if (unit->getType().isResourceDepot())
		CreateThread(NULL, 0, GeneralOrManagerOrGerenteOrSomethingLikeThat, (LPVOID)unit, 0, NULL);
	else if (unit->getType() == BWAPI::UnitTypes::Terran_Barracks)
		CreateThread(NULL, 0, doSomethingWithBarracks, (LPVOID)unit, 0, NULL);
	else if (unit->getType() == BWAPI::UnitTypes::Terran_Marine)
		CreateThread(NULL, 0, doSomethingWithMariners, (LPVOID)unit, 0, NULL);
}

void AAExample::onUnitDestroy(BWAPI::Unit unit)
{
}

void AAExample::onUnitMorph(BWAPI::Unit unit)
{
	// AA: This is a important function for the Zerg player.
}

void AAExample::onUnitRenegade(BWAPI::Unit unit)
{
}

void AAExample::onSaveGame(std::string gameName)
{
}

void AAExample::onUnitComplete(BWAPI::Unit unit)
{
}

DWORD WINAPI AAExample::thisShouldBeAClassButImTooLazyToDoIt_Worker(LPVOID param){

	//Broodwar->sendText(std::to_string(Broodwar->self()->getStartLocation().x).c_str());
	//Broodwar->sendText(std::to_string(Broodwar->self()->getStartLocation().y).c_str());

	BWAPI::Unit unit = static_cast<BWAPI::Unit>(param);
	DWORD dwWaitResult;

	while (true){

		dwWaitResult = WaitForSingleObject(
			ghMutex,    // handle to mutex
			100);  // time-out interval

		//// If end game, or if it exists (remember to always check)
		if (GameOver || unit == NULL || !unit->exists())  {
			ReleaseMutex(ghMutex);
			return 0; // end thread
		} // end thread
		// You can check tons of others things like isStuck, isLockedDown, constructing
		if (!unit->isCompleted() || !unit->isCompleted()){ // You can create it on the onUnitComplete too!
			ReleaseMutex(ghMutex);
			Sleep(500);
			continue;
		}

		if (dwWaitResult == WAIT_OBJECT_0 || dwWaitResult == WAIT_ABANDONED) //RAII
		{
			// if our worker is idle
			if (unit->isIdle())
			{
				// Order workers carrying a resource to return them to the center,
				// otherwise find a mineral patch to harvest.
				if (unit->isCarryingGas() || unit->isCarryingMinerals())
				{
					unit->returnCargo();
				}
				else if (!unit->getPowerUp())  // The worker cannot harvest anything if it
				{                             // is carrying a powerup such as a flag
					// Harvest from the nearest mineral patch or gas refinery
					BWAPI::Unit tempu = unit->getClosestUnit(
						BWAPI::Filter::IsMineralField || BWAPI::Filter::IsRefinery);
					if (tempu != NULL && !unit->gather(tempu))
					{
						Broodwar->sendText(Broodwar->getLastError().getName().c_str());
						// If the call fails, then print the last error message
						// Broodwar << Broodwar->getLastError() << std::endl;
					}
					else if(Broodwar->self()->minerals() == 0 && !lockMineralsUnitset)
					{	
						lockMineralsUnitset = true;

						if (!minerals.empty())
							{
								for (auto mineral = minerals.begin(); mineral != minerals.end(); ++mineral)
								{
									if (*mineral != NULL)
									{
										unit->gather(*mineral);
									}
								}
							}
						lockMineralsUnitset = false;
					}

				} // closure: has no powerup
				
			} // closure: if idle

			if (!ReleaseMutex(ghMutex))
			{
				// Handle error.
			}

			Sleep(10); // Some agents can sleep more than others. 
		}
	}
}

DWORD WINAPI AAExample::GeneralOrManagerOrGerenteOrSomethingLikeThat(LPVOID param){

	BWAPI::Unit hq = static_cast<BWAPI::Unit>(param);
	DWORD dwWaitResult;

	while (true){

		dwWaitResult = WaitForSingleObject(
			ghMutex,    // handle to mutex
			100);  // time-out interval

		if (GameOver || hq == NULL || !hq->exists()) {
			ReleaseMutex(ghMutex);
			return 0; // end thread
		}
		// Some things are commom between units, so you can apply a little of OO here.

		if (dwWaitResult == WAIT_OBJECT_0 || dwWaitResult == WAIT_ABANDONED)
		{
			bool canTrain = false;
			int workersCount = Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_SCV);
			int marinersCount = Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Marine);
			if ((workersCount * 2) <= marinersCount) {
				canTrain = hq->train(hq->getType().getRace().getWorker());
			}

			if (hq->isIdle() && !canTrain)
			{
				// If that fails, get error
				Error lastErr = Broodwar->getLastError();

				// Retrieve the supply provider type in the case that we have run out of supplies
				UnitType supplyProviderType = supplyProviderType = BWAPI::UnitTypes::Terran_Barracks;

				static int lastChecked = 0;

				if (lastErr == Errors::Insufficient_Supply) {
					hq->getType().getRace().getSupplyProvider();
				}

				// If we are supply blocked and haven't tried constructing more recently
				if (//lastErr == Errors::Insufficient_Supply &&
					lastChecked + 400 < Broodwar->getFrameCount() &&
					Broodwar->self()->incompleteUnitCount(supplyProviderType) == 0)
				{
					lastChecked = Broodwar->getFrameCount();

					// Retrieve a unit that is capable of constructing the supply needed
					Unit supplyBuilder = hq->getClosestUnit(GetType == supplyProviderType.whatBuilds().first &&
						(IsIdle || IsGatheringMinerals) &&
						IsOwned);
					// If a unit was found
					if (supplyBuilder)
					{
						if (supplyProviderType.isBuilding())
						{
							TilePosition targetBuildLocation = Broodwar->getBuildLocation(supplyProviderType, supplyBuilder->getTilePosition());
							if (targetBuildLocation)
							{
								
								// Order the builder to construct the supply structure
								supplyBuilder->build(supplyProviderType, targetBuildLocation);
							}
						}
						else
						{
							// Train the supply provider (Overlord) if the provider is not a structure
							supplyBuilder->train(supplyProviderType);
						}
					} // closure: supplyBuilder is valid
				} // closure: insufficient supply
			} // closure: failed to train idle unit
			// Release ownership of the mutex object
			if (!ReleaseMutex(ghMutex))
			{
				// Handle error.
			}
		}
		Sleep(20);
	}
}



DWORD WINAPI AAExample::doSomethingWithBarracks(LPVOID param){

	BWAPI::Unit hq = static_cast<BWAPI::Unit>(param);
	DWORD dwWaitResult;

	while (true){

		dwWaitResult = WaitForSingleObject(
			ghMutex,    // handle to mutex
			100);  // time-out interval

		if (GameOver || hq == NULL || !hq->exists()) {
			ReleaseMutex(ghMutex);
			return 0; // end thread
		}
		// Some things are commom between units, so you can apply a little of OO here.

		if (dwWaitResult == WAIT_OBJECT_0 || dwWaitResult == WAIT_ABANDONED)
		{
			if (hq->isIdle() && !hq->train(BWAPI::UnitTypes::Terran_Marine))
			{
				// If that fails, get error
				Error lastErr = Broodwar->getLastError();

				// Retrieve the supply provider type in the case that we have run out of supplies
				UnitType supplyProviderType = hq->getType().getRace().getSupplyProvider();

				static int lastChecked = 0;

				// If we are supply blocked and haven't tried constructing more recently
				if (lastErr == Errors::Insufficient_Supply &&
					lastChecked + 400 < Broodwar->getFrameCount() &&
					Broodwar->self()->incompleteUnitCount(supplyProviderType) == 0)
				{
					lastChecked = Broodwar->getFrameCount();

					// Retrieve a unit that is capable of constructing the supply needed
					Unit supplyBuilder = hq->getClosestUnit(GetType == supplyProviderType.whatBuilds().first &&
						(IsIdle || IsGatheringMinerals) &&
						IsOwned);
					// If a unit was found
					if (supplyBuilder)
					{
						if (supplyProviderType.isBuilding())
						{
							TilePosition targetBuildLocation = Broodwar->getBuildLocation(supplyProviderType, supplyBuilder->getTilePosition());
							if (targetBuildLocation)
							{
								// Order the builder to construct the supply structure
								supplyBuilder->build(supplyProviderType, targetBuildLocation);
							}
						}
						else
						{
							// Train the supply provider (Overlord) if the provider is not a structure
							supplyBuilder->train(supplyProviderType);
						}
					} // closure: supplyBuilder is valid
				} // closure: insufficient supply
			} // closure: failed to train idle unit
			// Release ownership of the mutex object
			if (!ReleaseMutex(ghMutex))
			{
				// Handle error.
			}
		}
		Sleep(20);
	}
}

DWORD WINAPI AAExample::doSomethingWithMariners(LPVOID param){

	BWAPI::Unit unit = static_cast<BWAPI::Unit>(param);
	DWORD dwWaitResult;
	Broodwar->sendText("Marine");
	Broodwar->sendText(std::to_string(unit->getPosition().x).c_str());
	Broodwar->sendText(std::to_string(unit->getPosition().y).c_str());

	while (true){

		dwWaitResult = WaitForSingleObject(
			ghMutex,    // handle to mutex
			100);  // time-out interval

		//// If end game, or if it exists (remember to always check)
		if (GameOver || unit == NULL || !unit->exists())  {
			ReleaseMutex(ghMutex);
			return 0; // end thread
		} // end thread
		// You can check tons of others things like isStuck, isLockedDown, constructing
		if (!unit->isCompleted() || !unit->isCompleted()){ // You can create it on the onUnitComplete too!
			ReleaseMutex(ghMutex);
			Sleep(500);
			continue;
		}

		if (dwWaitResult == WAIT_OBJECT_0 || dwWaitResult == WAIT_ABANDONED) //RAII
		{
			int marinersCount = Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Marine);
			int mineralsCount = Broodwar->self()->minerals();
			// if our worker is idle
			if ((unit->isIdle() && marinersCount >= 10 && !unit->isPatrolling())) 
			{

				unit->patrol(Position(enemyPositionX, enemyPositionY));

			} // closure: if idle
			else if (unit->isPatrolling() && enemyCommandCenter != NULL)
			{
				unit->attack(enemyCommandCenter->getPosition());
			}
			if (!ReleaseMutex(ghMutex))
			{
				// Handle error.
			}

			Sleep(10); // Some agents can sleep more than others. 
		}
	}
}
