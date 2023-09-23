#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
#include "Behaviors.h"

using namespace std;

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "Poppy";
	info.Student_FirstName = "Nick";
	info.Student_LastName = "Azarafroz";
	info.Student_Class = "2DAE07";

	ItemInfo invalidItem;
	m_Inventory = { invalidItem, invalidItem, invalidItem, invalidItem, invalidItem };

	m_SearchPoints = { std::make_pair(Elite::Vector2{-100, 100}, false), std::make_pair(Elite::Vector2{-200, 0}, false),
					   std::make_pair(Elite::Vector2{-100, -100}, false),	std::make_pair(Elite::Vector2{-200, -200}, false),
					   std::make_pair(Elite::Vector2{0, -200}, false), std::make_pair(Elite::Vector2{100, -100}, false),
					   std::make_pair(Elite::Vector2{200, -200}, false),	std::make_pair(Elite::Vector2{200, 0}, false),
					   std::make_pair(Elite::Vector2{100, 100}, false), std::make_pair(Elite::Vector2{200, 200}, false),
					   std::make_pair(Elite::Vector2{0, 200}, false),	std::make_pair(Elite::Vector2{-200, 200}, false),
					   std::make_pair(Elite::Vector2{0, 0}, false) };

	//1. Create Blackboard
	Elite::Blackboard* pBlackBoard = CreateBlackboard();

	//2. Create BT
	m_pBehaviorTree = new Elite::BehaviorTree(pBlackBoard,
		new Elite::BehaviorSelector
		({
			new Elite::BehaviorSequence
			({
				new Elite::BehaviorConditional(&BT_Conditions::IHaveUseableMedkit),
				new Elite::BehaviorAction(&BT_Actions::HealMyself)
			}),
			new Elite::BehaviorSequence
			({
				new Elite::BehaviorConditional(&BT_Conditions::IHaveEatableFood),
				new Elite::BehaviorAction(&BT_Actions::EatSomething)
			}),
			new Elite::BehaviorSequence
			({
				new Elite::BehaviorConditional(&BT_Conditions::IHaveAWeapon),
					new Elite::BehaviorSequence
					({
						new Elite::BehaviorConditional(&BT_Conditions::IHaveNoAmmo),
						new Elite::BehaviorAction(&BT_Actions::ThrowEmptyWeaponsAway)
					}),
			}),
			new Elite::BehaviorSequence
			({
				new Elite::BehaviorConditional(&BT_Conditions::IAmInsidePurgeZone),
				new Elite::BehaviorAction(&BT_Actions::LeavePurgeZone)
			}),
			new Elite::BehaviorSequence
			({
				new Elite::BehaviorConditional(&BT_Conditions::IHaveGarbage),
				new Elite::BehaviorAction(&BT_Actions::ThrowGarbageAway)
			}),
			new Elite::BehaviorSequence
			({
				new Elite::BehaviorConditional(&BT_Conditions::ISeeZombies),
					new Elite::BehaviorSequence
					({
						new Elite::BehaviorConditional(&BT_Conditions::IHaveAWeapon),
							new Elite::BehaviorSequence
							({
								new Elite::BehaviorConditional(&BT_Conditions::IHaveAmmo),
								new Elite::BehaviorAction(&BT_Actions::FaceZombie)
							}),
					}),
			}),
			new Elite::BehaviorSequence
			({
				new Elite::BehaviorConditional(&BT_Conditions::IWasBitten),
					new Elite::BehaviorSequence
					({
						new Elite::BehaviorConditional(&BT_Conditions::IHaveAWeapon),
						new Elite::BehaviorAction(&BT_Actions::TurnAround)
					}),
			}),
			new Elite::BehaviorSequence
			({
				new Elite::BehaviorConditional(&BT_Conditions::ISeePurgeZone),
				new Elite::BehaviorAction(&BT_Actions::Wait)
			}),
			new Elite::BehaviorSequence
			({
				new Elite::BehaviorConditional(&BT_Conditions::ISeeZombies),
					new Elite::BehaviorSequence
					({
						new Elite::BehaviorConditional(&BT_Conditions::IDontHaveAWeapon),
						new Elite::BehaviorAction(&BT_Actions::Search)
					}),
			}),
			new Elite::BehaviorSequence
			({
				new Elite::BehaviorConditional(&BT_Conditions::ISeeItems),
					new Elite::BehaviorSequence
					({
						new Elite::BehaviorConditional(&BT_Conditions::IHaveSpaceLeft),
						new Elite::BehaviorAction(&BT_Actions::GoToItem)
					}),
			}),
			new Elite::BehaviorSequence
			({
				new Elite::BehaviorConditional(&BT_Conditions::IamStarving),
					new Elite::BehaviorSequence
					({
						new Elite::BehaviorConditional(&BT_Conditions::MyInventoryIsFull),
						new Elite::BehaviorAction(&BT_Actions::MakeSpace)
					}),
			}),
			new Elite::BehaviorSequence
			({
				new Elite::BehaviorConditional(&BT_Conditions::ISeeHouses),
				new Elite::BehaviorAction(&BT_Actions::GoInsideHouse)
			}),
			new Elite::BehaviorAction(&BT_Actions::Search)
		})
	);
}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
}

//Called only once
void Plugin::DllShutdown()
{
	//Called wheb the plugin gets unloaded
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be useful to inspect certain behaviors (Default = false)
	params.LevelFile = "GameLevel.gppl";
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	params.StartingDifficultyStage = 1;
	params.InfiniteStamina = false;
	params.SpawnDebugPistol = false;
	params.SpawnDebugShotgun = false;
	params.SpawnPurgeZonesOnMiddleClick = true;
	params.PrintDebugMessages = true;
	params.ShowDebugItemNames = true;
	params.Seed = 5;
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::Update(float dt)
{
	//Demo Event Code
	//In the end your AI should be able to walk around without external input
	if (m_pInterface->Input_IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	{
		//Update target based on input
		Elite::MouseData mouseData = m_pInterface->Input_GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
		const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
		m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Space))
	{
		m_CanRun = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Left))
	{
		m_AngSpeed -= Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Right))
	{
		m_AngSpeed += Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_G))
	{
		m_GrabItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_U))
	{
		m_UseItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_R))
	{
		m_RemoveItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyUp(Elite::eScancode_Space))
	{
		m_CanRun = false;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Delete))
	{
		m_pInterface->RequestShutdown();
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_KP_Minus))
	{
		if (m_InventorySlot > 0)
			--m_InventorySlot;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_KP_Plus))
	{
		if (m_InventorySlot < 4)
			++m_InventorySlot;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Q))
	{
		ItemInfo info = {};
		m_pInterface->Inventory_GetItem(m_InventorySlot, info);
		std::cout << (int)info.Type << std::endl;
	}
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	auto steering = SteeringPlugin_Output();

	m_pBehaviorTree->Update(dt);
	
	//Use the Interface (IAssignmentInterface) to 'interface' with the AI_Framework
	auto agentInfo = m_pInterface->Agent_GetInfo();

	//Use the navmesh to calculate the next navmesh point
	//auto nextTargetPos = m_pInterface->NavMesh_GetClosestPathPoint(checkpointLocation);

	//OR, Use the mouse target
	auto nextTargetPos = m_pInterface->NavMesh_GetClosestPathPoint(m_Target); //Uncomment this to use mouse position as guidance

	auto vHousesInFOV = GetHousesInFOV();
	auto vEntitiesInFOV = GetEntitiesInFOV(); 

	m_pBehaviorTree->GetBlackboard()->GetData("SteeringOutput", steering);

	m_pBehaviorTree->GetBlackboard()->GetData("HousesLooted", m_HousesLooted);
	m_pBehaviorTree->GetBlackboard()->GetData("HousesICanLoot", m_HousesICanLoot);

	m_pBehaviorTree->GetBlackboard()->GetData("ItemsLooted", m_ItemsLooted);
	m_pBehaviorTree->GetBlackboard()->GetData("ItemsICanLoot", m_ItemsICanLoot);

	m_pBehaviorTree->GetBlackboard()->GetData("EnemiesInFOV", m_EnemiesInFOV);

	m_pBehaviorTree->GetBlackboard()->GetData("PurgeZonesInFOV", m_PurgeZonesInFOV);

	m_pBehaviorTree->GetBlackboard()->GetData("SearchPoints", m_SearchPoints);

	m_pBehaviorTree->GetBlackboard()->GetData("Inventory", m_Inventory);
	m_pBehaviorTree->GetBlackboard()->GetData("TurnAround", m_TurnAround);

	//Houses
	//------------------------------------------------------------------------------------------------------
	m_HousesInFOV = vHousesInFOV;

	for (const auto& house : m_HousesInFOV) 
	{
		// Check if the house is already looted
		if (std::find_if(m_HousesLooted.begin(), m_HousesLooted.end(),
			[&house](const HouseInfo& h) { return h.Center == house.Center; }) != m_HousesLooted.end()) 
		{
			// House is already looted, continue to the next house in FOV
			continue;
		}

		// Check if the house is already in the list of houses that can be looted
		if (std::find_if(m_HousesICanLoot.begin(), m_HousesICanLoot.end(),
			[&house](const HouseInfo& h) { return h.Center == house.Center; }) == m_HousesICanLoot.end()) 
		{
			// House is not in the list, add it to the list of houses that can be looted
			m_HousesICanLoot.push_back(house);
		}
	}
	//------------------------------------------------------------------------------------------------------

	//Items & Enemies
	//------------------------------------------------------------------------------------------------------
	m_EntityInFOV = vEntitiesInFOV;

	m_EnemiesInFOV.clear();
	m_PurgeZonesInFOV.clear();

	for(const auto& entity : m_EntityInFOV)
	{
		if(entity.Type == eEntityType::ITEM)
		{
			if (std::find_if(m_ItemsLooted.begin(), m_ItemsLooted.end(),
				[&entity](const EntityInfo& e) { return e.Location == entity.Location; }) != m_ItemsLooted.end())
			{
				continue;
			}

			if(std::find_if(m_ItemsICanLoot.begin(), m_ItemsICanLoot.end(),
				[&entity](const EntityInfo& e) { return e.Location == entity.Location; }) == m_ItemsICanLoot.end())
			{
				m_ItemsICanLoot.push_back(entity);
			}
		}

		if(entity.Type == eEntityType::ENEMY)
		{
			m_EnemiesInFOV.push_back(entity);
		}

		if (entity.Type == eEntityType::PURGEZONE) 
		{
			m_PurgeZonesInFOV.push_back(entity);
		}
	}
	//--------------------------------------------------------------------------------------------------------

	m_pBehaviorTree->GetBlackboard()->ChangeData("HousesLooted", m_HousesLooted);
	m_pBehaviorTree->GetBlackboard()->ChangeData("HousesICanLoot", m_HousesICanLoot);

	m_pBehaviorTree->GetBlackboard()->ChangeData("ItemsLooted", m_ItemsLooted);
	m_pBehaviorTree->GetBlackboard()->ChangeData("ItemsICanLoot", m_ItemsICanLoot);

	m_pBehaviorTree->GetBlackboard()->ChangeData("EnemiesInFOV", m_EnemiesInFOV);

	m_pBehaviorTree->GetBlackboard()->ChangeData("PurgeZonesInFOV", m_PurgeZonesInFOV);

	m_pBehaviorTree->GetBlackboard()->ChangeData("SearchPoints", m_SearchPoints);

	m_pBehaviorTree->GetBlackboard()->ChangeData("Inventory", m_Inventory);

	m_pBehaviorTree->GetBlackboard()->ChangeData("TurnAround", m_TurnAround);

	//for (auto& e : vEntitiesInFOV)
	//{
	//	if (e.Type == eEntityType::PURGEZONE)
	//	{
	//		PurgeZoneInfo zoneInfo;
	//		m_pInterface->PurgeZone_GetInfo(e, zoneInfo);
	//		//std::cout << "Purge Zone in FOV:" << e.Location.x << ", "<< e.Location.y << "---Radius: "<< zoneInfo.Radius << std::endl;
	//	}
	//}

	////INVENTORY USAGE DEMO
	////********************

	//if (m_GrabItem)
	//{
	//	ItemInfo item;
	//	//Item_Grab > When DebugParams.AutoGrabClosestItem is TRUE, the Item_Grab function returns the closest item in range
	//	//Keep in mind that DebugParams are only used for debugging purposes, by default this flag is FALSE
	//	//Otherwise, use GetEntitiesInFOV() to retrieve a vector of all entities in the FOV (EntityInfo)
	//	//Item_Grab gives you the ItemInfo back, based on the passed EntityHash (retrieved by GetEntitiesInFOV)
	//	if (m_pInterface->Item_Grab({}, item))
	//	{
	//		//Once grabbed, you can add it to a specific inventory slot
	//		//Slot must be empty
	//		m_pInterface->Inventory_AddItem(m_InventorySlot, item);
	//	}
	//}

	//if (m_UseItem)
	//{
	//	//Use an item (make sure there is an item at the given inventory slot)
	//	m_pInterface->Inventory_UseItem(m_InventorySlot);
	//}

	//if (m_RemoveItem)
	//{
	//	//Remove an item from a inventory slot
	//	m_pInterface->Inventory_RemoveItem(m_InventorySlot);
	//}

	//Simple Seek Behaviour (towards Target)
	//steering.LinearVelocity = nextTargetPos - agentInfo.Position; //Desired Velocity
	//steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	//steering.LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed

	//if (Distance(nextTargetPos, agentInfo.Position) < 2.f)
	//{
	//	steering.LinearVelocity = Elite::ZeroVector2;
	//}

	//steering.AngularVelocity = m_AngSpeed; //Rotate your character to inspect the world while walking
	//steering.AutoOrient = true; //Setting AutoOrient to True overrides the AngularVelocity

	//steering.RunMode = m_CanRun; //If RunMode is True > MaxLinSpd is increased for a limited time (till your stamina runs out)

	//SteeringPlugin_Output is works the exact same way a SteeringBehaviour output

//@End (Demo Purposes)
	//m_GrabItem = false; //Reset State
	//m_UseItem = false;
	//m_RemoveItem = false;

	return steering;
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}

Elite::Blackboard* Plugin::CreateBlackboard()
{
	Elite::Blackboard* pBlackboard = new Elite::Blackboard();
	pBlackboard->AddData("ExamInterface", static_cast<IExamInterface*>(m_pInterface));
	pBlackboard->AddData("SteeringOutput", static_cast<SteeringPlugin_Output>(m_SteeringOutput));

	pBlackboard->AddData("HousesICanLoot", m_HousesICanLoot);
	pBlackboard->AddData("HousesLooted", m_HousesLooted);

	pBlackboard->AddData("ItemsICanLoot", m_ItemsICanLoot);
	pBlackboard->AddData("ItemsLooted", m_ItemsLooted);

	pBlackboard->AddData("EnemiesInFOV", m_EnemiesInFOV);

	pBlackboard->AddData("PurgeZonesInFOV", m_PurgeZonesInFOV);

	pBlackboard->AddData("Inventory", m_Inventory);

	pBlackboard->AddData("SearchPoints", m_SearchPoints);

	pBlackboard->AddData("TurnAround", m_TurnAround);
	return pBlackboard;
}
