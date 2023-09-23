/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "IExamInterface.h"
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"
#include "EBehaviorTree.h"
#include "../inc/EliteMath/EMath.h"

#include <array>
//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

namespace BT_Actions
{
	Elite::BehaviorState Wander(Elite::Blackboard* pBlackboard)
	{
		float wanderAngle = 0.0f;
		float radius = 4.f;
		float offsetDistance = 6.f;

		IExamInterface* pExamInterface = nullptr;
		SteeringPlugin_Output steeringOutput{};

		if (!pBlackboard->GetData("ExamInterface", pExamInterface) || pExamInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SteeringOutput", steeringOutput))
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo agentInfo = pExamInterface->Agent_GetInfo();

		wanderAngle += float(Elite::ToRadians(float(rand() % 21 - 10)));
		float x = radius * cosf(wanderAngle);
		float y = radius * sinf(wanderAngle);

		Elite::Vector2 pointOnCircle{ x, y };

		Elite::Vector2 centerOfCircle{ (agentInfo.Position + agentInfo.LinearVelocity.GetNormalized() * offsetDistance) };

		steeringOutput.RunMode = false;
		steeringOutput.AutoOrient = true;
		steeringOutput.LinearVelocity = (pointOnCircle + centerOfCircle) - (agentInfo.Position);
		steeringOutput.LinearVelocity.Normalize();
		steeringOutput.LinearVelocity *= agentInfo.MaxLinearSpeed;

		pBlackboard->ChangeData("SteeringOutput", steeringOutput);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState GoInsideHouse(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface = nullptr;
		SteeringPlugin_Output steeringOutput{};
		std::vector<HouseInfo> housesICanLoot{};
		std::vector<HouseInfo> housesLooted{};

		Elite::Vector2 housePos{};
		bool turnAround{};

		if (!pBlackboard->GetData("ExamInterface", pExamInterface) || pExamInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SteeringOutput", steeringOutput))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("HousesICanLoot", housesICanLoot))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("HousesLooted", housesLooted))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("TurnAround", turnAround))
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo agentInfo = pExamInterface->Agent_GetInfo();

		for(const auto& house : housesICanLoot)
		{
			housePos = pExamInterface->NavMesh_GetClosestPathPoint(house.Center);

			pExamInterface->Draw_SolidCircle(house.Center, 2.f, { 0, 0 }, { 0, 1, 0 });
			turnAround = false;

			if (agentInfo.Position.Distance(house.Center) < 2.f)
			{
				housesLooted.push_back(house);
				housesICanLoot.erase(housesICanLoot.begin());
			}
			break;
		}

		if (agentInfo.WasBitten)
		{
			steeringOutput.RunMode = true;
		}

		if (agentInfo.Stamina == 0)
		{
			steeringOutput.RunMode = false;
		}

		steeringOutput.AutoOrient = true;
		steeringOutput.LinearVelocity = (housePos - agentInfo.Position);
		steeringOutput.LinearVelocity.Normalize();
		steeringOutput.LinearVelocity *= agentInfo.MaxLinearSpeed;

		pBlackboard->ChangeData("SteeringOutput", steeringOutput);
		pBlackboard->ChangeData("HousesLooted", housesLooted);
		pBlackboard->ChangeData("HousesICanLoot", housesICanLoot);
		pBlackboard->ChangeData("TurnAround", turnAround);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState GoToItem(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface = nullptr;
		SteeringPlugin_Output steeringOutput{};

		std::vector<EntityInfo> itemsICanLoot{};
		std::vector<EntityInfo> itemsLooted{};

		ItemInfo invalidItem;
		std::array<ItemInfo, 5> inventory{ invalidItem, invalidItem, invalidItem, invalidItem, invalidItem };

		Elite::Vector2 itemPos{};

		if (!pBlackboard->GetData("ExamInterface", pExamInterface) || pExamInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SteeringOutput", steeringOutput))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("ItemsICanLoot", itemsICanLoot))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("ItemsLooted", itemsLooted))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Inventory", inventory))
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo agentInfo = pExamInterface->Agent_GetInfo();

		for (const auto& item : itemsICanLoot)
		{
			ItemInfo itemInfo;
			itemPos = pExamInterface->NavMesh_GetClosestPathPoint(item.Location);

			pExamInterface->Draw_SolidCircle(item.Location, 2.f, { 0, 0 }, { 0, 0, 1 });

			if ((item.Location - agentInfo.Position).Magnitude() <= agentInfo.GrabRange)
			{
				for (size_t i{}; i < inventory.size(); ++i)
				{
					if (inventory[i].ItemHash == invalidItem.ItemHash)
					{
						pExamInterface->Item_Grab(item, itemInfo);
						inventory[i] = itemInfo;
						pExamInterface->Inventory_AddItem(i, itemInfo);
						itemsLooted.push_back(item);
						itemsICanLoot.erase(itemsICanLoot.begin());
						break;
					}
				}
			}
			break;
		}

		if (agentInfo.WasBitten)
		{
			steeringOutput.RunMode = true;
		}

		if (agentInfo.Stamina == 0)
		{
			steeringOutput.RunMode = false;
		}

		steeringOutput.AutoOrient = true;
		steeringOutput.LinearVelocity = (itemPos - agentInfo.Position);
		steeringOutput.LinearVelocity.Normalize();
		steeringOutput.LinearVelocity *= agentInfo.MaxLinearSpeed;

		pBlackboard->ChangeData("SteeringOutput", steeringOutput);
		pBlackboard->ChangeData("ItemsLooted", itemsLooted);
		pBlackboard->ChangeData("ItemsICanLoot", itemsICanLoot);
		pBlackboard->ChangeData("Inventory", inventory);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState EatSomething(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface = nullptr;
		const float maxAgentEnergy = 10.f;

		ItemInfo invalidItem;
		std::array<ItemInfo, 5> inventory{ invalidItem, invalidItem, invalidItem, invalidItem, invalidItem };

		if (!pBlackboard->GetData("ExamInterface", pExamInterface) || pExamInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Inventory", inventory))
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo agentInfo = pExamInterface->Agent_GetInfo();

		for(size_t i{}; i < inventory.size(); ++i)
		{
			if(inventory[i].Type == eItemType::FOOD)
			{
				if (maxAgentEnergy - agentInfo.Energy > static_cast<float>(pExamInterface->Food_GetEnergy(inventory[i])))
				{
					pExamInterface->Inventory_UseItem(i);
					pExamInterface->Inventory_RemoveItem(i);
					inventory[i] = invalidItem;
					break;
				}
			}
		}

		pBlackboard->ChangeData("Inventory", inventory);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState HealMyself(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface = nullptr;
		const float maxAgentHealth = 10.f;

		ItemInfo invalidItem;
		std::array<ItemInfo, 5> inventory{ invalidItem, invalidItem, invalidItem, invalidItem, invalidItem };

		if (!pBlackboard->GetData("ExamInterface", pExamInterface) || pExamInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Inventory", inventory))
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo agentInfo = pExamInterface->Agent_GetInfo();

		for (size_t i{}; i < inventory.size(); ++i)
		{
			if (inventory[i].Type == eItemType::MEDKIT)
			{
				if (maxAgentHealth - agentInfo.Health > static_cast<float>(pExamInterface->Medkit_GetHealth(inventory[i])))
				{
					pExamInterface->Inventory_UseItem(i);
					pExamInterface->Inventory_RemoveItem(i);
					inventory[i] = invalidItem;
					break;
				}
			}
		}

		pBlackboard->ChangeData("Inventory", inventory);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ThrowGarbageAway(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface = nullptr;

		ItemInfo invalidItem;
		std::array<ItemInfo, 5> inventory{ invalidItem, invalidItem, invalidItem, invalidItem, invalidItem };

		if (!pBlackboard->GetData("ExamInterface", pExamInterface) || pExamInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Inventory", inventory))
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo agentInfo = pExamInterface->Agent_GetInfo();

		for (size_t i{}; i < inventory.size(); ++i)
		{
			if (inventory[i].Type == eItemType::GARBAGE)
			{
				pExamInterface->Inventory_RemoveItem(i);
				inventory[i] = invalidItem;
				break;
			}
		}

		pBlackboard->ChangeData("Inventory", inventory);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ThrowEmptyWeaponsAway(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface = nullptr;

		ItemInfo invalidItem;
		std::array<ItemInfo, 5> inventory{ invalidItem, invalidItem, invalidItem, invalidItem, invalidItem };

		if (!pBlackboard->GetData("ExamInterface", pExamInterface) || pExamInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Inventory", inventory))
		{
			return Elite::BehaviorState::Failure;
		}

		for (size_t i{}; i < inventory.size(); ++i)
		{
			if ((inventory[i].Type == eItemType::PISTOL) || inventory[i].Type == eItemType::SHOTGUN)
			{
				if (pExamInterface->Weapon_GetAmmo(inventory[i]) == 0)
				{
					pExamInterface->Inventory_RemoveItem(i);
					inventory[i] = invalidItem;
					break;
				}
			}
		}

		pBlackboard->ChangeData("Inventory", inventory);
		pBlackboard->ChangeData("ExamInterface", pExamInterface);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState FaceZombie(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface = nullptr;
		SteeringPlugin_Output steeringOutput{};

		std::vector<EntityInfo> enemiesInFOV{};
		Elite::Vector2 enemyPos{};

		ItemInfo invalidItem;
		std::array<ItemInfo, 5> inventory{ invalidItem, invalidItem, invalidItem, invalidItem, invalidItem };

		bool turnAround{};

		if (!pBlackboard->GetData("ExamInterface", pExamInterface) || pExamInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SteeringOutput", steeringOutput))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("EnemiesInFOV", enemiesInFOV))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Inventory", inventory))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("TurnAround", turnAround))
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo agentInfo = pExamInterface->Agent_GetInfo();

		for (const auto& enemy : enemiesInFOV)
		{
			enemyPos = pExamInterface->NavMesh_GetClosestPathPoint(enemy.Location);

			pExamInterface->Draw_SolidCircle(enemy.Location, 2.f, { 0, 0 }, { 1, 0, 0 });

			turnAround = false;

			for (size_t i{}; i < inventory.size(); ++i)
			{
				if (inventory[i].Type == eItemType::PISTOL || inventory[i].Type == eItemType::SHOTGUN)
				{
					if ((agentInfo.Position - enemy.Location).Magnitude() < agentInfo.FOV_Range - 13.f)
					{
						pExamInterface->Inventory_UseItem(i);
						break;
					}
				}
			}
		}
		
		Elite::Vector2 enemyDirection{};
		float desiredOrientation{};
		float orientationDifference{};

		enemyDirection = (enemyPos - agentInfo.Position) * -1.f;
		desiredOrientation = atan2f(-enemyDirection.y, -enemyDirection.x);
		orientationDifference = desiredOrientation - agentInfo.Orientation;
		orientationDifference = atan2f(sinf(orientationDifference), cosf(orientationDifference));

		steeringOutput.AutoOrient = false;
		steeringOutput.AngularVelocity = orientationDifference;
		steeringOutput.AngularVelocity *= agentInfo.MaxAngularSpeed * 5.f;

		steeringOutput.LinearVelocity = (enemyPos - agentInfo.Position) * -1.f;
		steeringOutput.LinearVelocity.Normalize();
		steeringOutput.LinearVelocity *= agentInfo.MaxLinearSpeed * 0.65f;

		pBlackboard->ChangeData("SteeringOutput", steeringOutput);
		pBlackboard->ChangeData("Inventory", inventory);
		pBlackboard->ChangeData("EnemiesInFOV", enemiesInFOV);
		pBlackboard->ChangeData("TurnAround", turnAround);

		return Elite::BehaviorState::Success;

	}

	Elite::BehaviorState Search(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface = nullptr;
		SteeringPlugin_Output steeringOutput;

		std::array<std::pair<Elite::Vector2, bool>, 13> searchPoints;
		std::pair<Elite::Vector2, int> currentSearchPoint{};

		if (!pBlackboard->GetData("ExamInterface", pExamInterface) || pExamInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SteeringOutput", steeringOutput))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SearchPoints", searchPoints))
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo agentInfo = pExamInterface->Agent_GetInfo();

		if(searchPoints[12].second == true)
		{
			for (auto& searchPoint : searchPoints)
			{
				searchPoint.second = false;
			}
		}

		for(size_t i{}; searchPoints.size(); ++i)
		{
			if (searchPoints[i].second == false)
			{
				currentSearchPoint.first = searchPoints[i].first;
				currentSearchPoint.second = i;
				break;
			}
			else continue;
		}

		auto goToCurrentSearchPoint = pExamInterface->NavMesh_GetClosestPathPoint(currentSearchPoint.first);

		steeringOutput.AutoOrient = true;

		if(agentInfo.WasBitten)
		{
			steeringOutput.RunMode = true;
		}

		if(agentInfo.Stamina == 0)
		{
			steeringOutput.RunMode = false;
		}

		steeringOutput.LinearVelocity = (goToCurrentSearchPoint - agentInfo.Position);
		steeringOutput.LinearVelocity.Normalize();
		steeringOutput.LinearVelocity *= agentInfo.MaxLinearSpeed;

		pExamInterface->Draw_SolidCircle(currentSearchPoint.first, 4.f, { 0, 0 }, { 0, 1, 0 });

		if (agentInfo.Position.Distance(currentSearchPoint.first) <= 4.f)
		{
			searchPoints[currentSearchPoint.second].second = true;
		}

		pBlackboard->ChangeData("SteeringOutput", steeringOutput);
		pBlackboard->ChangeData("SearchPoints", searchPoints);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState Wait(Elite::Blackboard* pBlackboard)
	{
		SteeringPlugin_Output steeringOutput{};

		if (!pBlackboard->GetData("SteeringOutput", steeringOutput))
		{
			return Elite::BehaviorState::Failure;
		}

		steeringOutput.RunMode = false;
		steeringOutput.AutoOrient = true;
		steeringOutput.LinearVelocity = { 0.f, 0.f };

		pBlackboard->ChangeData("SteeringOutput", steeringOutput);

		return Elite::BehaviorState::Success;

	}

	Elite::BehaviorState LeavePurgeZone(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface = nullptr;
		SteeringPlugin_Output steeringOutput{};
		Elite::Vector2 exitPoint{};
		std::vector<EntityInfo> purgeZonesInFOV{};
		PurgeZoneInfo purgeZone;

		if (!pBlackboard->GetData("ExamInterface", pExamInterface) || pExamInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SteeringOutput", steeringOutput))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("PurgeZonesInFOV", purgeZonesInFOV))
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo agentInfo = pExamInterface->Agent_GetInfo();

		for (const auto& purgeZoneInFOV : purgeZonesInFOV)
		{
			pExamInterface->PurgeZone_GetInfo(purgeZoneInFOV, purgeZone);
			break;
		}

		exitPoint = purgeZone.Center + Elite::Vector2{-100.f, 0.f};

		auto exitPointOfPurgeZone = pExamInterface->NavMesh_GetClosestPathPoint(exitPoint);

		steeringOutput.RunMode = true;
		steeringOutput.AutoOrient = true;

		steeringOutput.LinearVelocity = (exitPointOfPurgeZone - agentInfo.Position);
		steeringOutput.LinearVelocity.Normalize();
		steeringOutput.LinearVelocity *= agentInfo.MaxLinearSpeed;

		pExamInterface->Draw_SolidCircle(exitPointOfPurgeZone, 4.f, { 0, 0 }, { 0, 1, 0 });

		pBlackboard->ChangeData("SteeringOutput", steeringOutput);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState TurnAround(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface = nullptr;
		SteeringPlugin_Output steeringOutput{};

		if (!pBlackboard->GetData("ExamInterface", pExamInterface) || pExamInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SteeringOutput", steeringOutput))
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo agentInfo = pExamInterface->Agent_GetInfo();

		steeringOutput.AutoOrient = false;
		steeringOutput.AngularVelocity = 2.f;

		pBlackboard->ChangeData("SteeringOutput", steeringOutput);

		return Elite::BehaviorState::Success;

	}

	Elite::BehaviorState MakeSpace(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface = nullptr;

		ItemInfo invalidItem;
		std::array<ItemInfo, 5> inventory{ invalidItem, invalidItem, invalidItem, invalidItem, invalidItem };

		if (!pBlackboard->GetData("ExamInterface", pExamInterface) || pExamInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Inventory", inventory))
		{
			return Elite::BehaviorState::Failure;
		}

		for (size_t i{}; i < inventory.size(); ++i)
		{
			if ((inventory[i].Type == eItemType::PISTOL) || inventory[i].Type == eItemType::SHOTGUN)
			{
				pExamInterface->Inventory_RemoveItem(i);
				inventory[i] = invalidItem;
				break;
			}
		}

		pBlackboard->ChangeData("Inventory", inventory);
		pBlackboard->ChangeData("ExamInterface", pExamInterface);

		return Elite::BehaviorState::Success;
	}
}

namespace BT_Conditions
{
	bool ISeeHouses(Elite::Blackboard* pBlackboard)
	{
		std::vector<HouseInfo> housesICanLoot{};

		if (!pBlackboard->GetData("HousesICanLoot", housesICanLoot))
		{
			return false;
		}

		if (!housesICanLoot.empty())
		{
			return true;
		}
		else return false;
	}

	bool ISeeItems(Elite::Blackboard* pBlackboard)
	{
		std::vector<EntityInfo> itemsICanLoot{};

		if (!pBlackboard->GetData("ItemsICanLoot", itemsICanLoot))
		{
			return false;
		}

		if (!itemsICanLoot.empty())
		{
			return true;
		}
		else return false;
	}

	bool ISeeZombies(Elite::Blackboard* pBlackboard)
	{
		std::vector<EntityInfo> enemiesInFOV{};

		if (!pBlackboard->GetData("EnemiesInFOV", enemiesInFOV))
		{
			return false;
		}

		if (!enemiesInFOV.empty())
		{
			return true;
		}
		else return false;
	}

	bool ISeePurgeZone(Elite::Blackboard* pBlackboard)
	{
		std::vector<EntityInfo> purgeZonesInFOV{};

		if (!pBlackboard->GetData("PurgeZonesInFOV", purgeZonesInFOV))
		{
			return false;
		}

		if (!purgeZonesInFOV.empty())
		{
			return true;
		}
		else return false;
	}

	bool IAmInsidePurgeZone(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface = nullptr;
		std::vector<EntityInfo> purgeZonesInFOV{};
		PurgeZoneInfo purgeZone;
		float distanceSquared;

		if (!pBlackboard->GetData("ExamInterface", pExamInterface) || pExamInterface == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("PurgeZonesInFOV", purgeZonesInFOV))
		{
			return false;
		}

		AgentInfo agentInfo = pExamInterface->Agent_GetInfo();


		for(const auto& purgeZoneInFOV : purgeZonesInFOV)
		{
			pExamInterface->PurgeZone_GetInfo(purgeZoneInFOV, purgeZone);
			break;
		}

		if (!purgeZone.ZoneHash) return false;

		distanceSquared = purgeZone.Center.DistanceSquared(agentInfo.Position);

		if(distanceSquared < (purgeZone.Radius * purgeZone.Radius))
		{
			return true;
		}

		return false;
	}

	bool IHaveEatableFood(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface = nullptr;
		const float maxAgentEnergy = 10.f;

		ItemInfo invalidItem;
		std::array<ItemInfo, 5> inventory{ invalidItem, invalidItem, invalidItem, invalidItem, invalidItem };

		if (!pBlackboard->GetData("ExamInterface", pExamInterface) || pExamInterface == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("Inventory", inventory))
		{
			return false;
		}

		AgentInfo agentInfo = pExamInterface->Agent_GetInfo();

		for (size_t i{}; i < inventory.size(); ++i)
		{
			if (inventory[i].Type == eItemType::FOOD)
			{
				if(maxAgentEnergy - agentInfo.Energy > static_cast<float>(pExamInterface->Food_GetEnergy(inventory[i])))
				{
					return true;
				}
			}
		}

		return false;
	}

	bool IHaveUseableMedkit(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface = nullptr;
		const float maxAgentHealth = 10.f;

		ItemInfo invalidItem;
		std::array<ItemInfo, 5> inventory{ invalidItem, invalidItem, invalidItem, invalidItem, invalidItem };

		if (!pBlackboard->GetData("ExamInterface", pExamInterface) || pExamInterface == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("Inventory", inventory))
		{
			return false;
		}

		AgentInfo agentInfo = pExamInterface->Agent_GetInfo();

		for (size_t i{}; i < inventory.size(); ++i)
		{
			if (inventory[i].Type == eItemType::MEDKIT)
			{
				if (maxAgentHealth - agentInfo.Health > static_cast<float>(pExamInterface->Medkit_GetHealth(inventory[i])))
				{
					return true;
				}
			}
		}

		return false;
	}

	bool IHaveGarbage(Elite::Blackboard* pBlackboard)
	{
		ItemInfo invalidItem;
		std::array<ItemInfo, 5> inventory{ invalidItem, invalidItem, invalidItem, invalidItem, invalidItem };

		if (!pBlackboard->GetData("Inventory", inventory))
		{
			return false;
		}

		for (size_t i{}; i < inventory.size(); ++i)
		{
			if(inventory[i].Type == eItemType::GARBAGE)
			{
				return true;
			}
		}

		return false;
	}

	bool IHaveAWeapon(Elite::Blackboard* pBlackboard)
	{
		ItemInfo invalidItem;
		std::array<ItemInfo, 5> inventory{ invalidItem, invalidItem, invalidItem, invalidItem, invalidItem };

		if (!pBlackboard->GetData("Inventory", inventory))
		{
			return false;
		}

		for (size_t i{}; i < inventory.size(); ++i)
		{
			if ((inventory[i].Type == eItemType::PISTOL) || inventory[i].Type == eItemType::SHOTGUN)
			{
				return true;
			}
		}

		return false;
	}

	bool IDontHaveAWeapon(Elite::Blackboard* pBlackboard)
	{
		return !IHaveAWeapon(pBlackboard);
	}

	bool IHaveAmmo(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface = nullptr;

		ItemInfo invalidItem;
		std::array<ItemInfo, 5> inventory{ invalidItem, invalidItem, invalidItem, invalidItem, invalidItem };

		if (!pBlackboard->GetData("Inventory", inventory))
		{
			return false;
		}

		if (!pBlackboard->GetData("ExamInterface", pExamInterface) || pExamInterface == nullptr)
		{
			return false;
		}

		for (size_t i{}; i < inventory.size(); ++i)
		{
			if ((inventory[i].Type == eItemType::PISTOL) || inventory[i].Type == eItemType::SHOTGUN)
			{
				if(pExamInterface->Weapon_GetAmmo(inventory[i]) == 0)
				{
					return false;
				}
			}
		}

		return true;
	}

	bool IHaveNoAmmo(Elite::Blackboard* pBlackboard)
	{
		return !IHaveAmmo(pBlackboard);
	}

	bool MyInventoryIsFull(Elite::Blackboard* pBlackboard)
	{
		ItemInfo invalidItem;
		std::array<ItemInfo, 5> inventory{ invalidItem, invalidItem, invalidItem, invalidItem, invalidItem };

		if (!pBlackboard->GetData("Inventory", inventory))
		{
			return false;
		}

		for (size_t i{}; i < inventory.size(); ++i)
		{
			if (inventory[i].ItemHash == invalidItem.ItemHash)
			{
				return false;
			}
		}

		return true;
	}

	bool IHaveSpaceLeft(Elite::Blackboard* pBlackboard)
	{
		return !MyInventoryIsFull(pBlackboard);
	}

	bool IWasBitten(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface = nullptr;
		bool turnAround{};

		if (!pBlackboard->GetData("ExamInterface", pExamInterface) || pExamInterface == nullptr)
		{
			return false;
		}


		if (!pBlackboard->GetData("TurnAround", turnAround))
		{
			return false;
		}

		AgentInfo agentInfo = pExamInterface->Agent_GetInfo();

		if(agentInfo.WasBitten)
		{
			turnAround = true;
		}

		pBlackboard->ChangeData("TurnAround", turnAround);

		return turnAround;
	}

	bool IamStarving(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pExamInterface = nullptr;

		if (!pBlackboard->GetData("ExamInterface", pExamInterface) || pExamInterface == nullptr)
		{
			return false;
		}

		AgentInfo agentInfo = pExamInterface->Agent_GetInfo();

		if(agentInfo.Energy < 3.f)
		{
			return true;
		}

		return false;
	}
}

#endif