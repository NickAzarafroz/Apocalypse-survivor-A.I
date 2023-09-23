#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"
#include "EBlackboard.h" // Create BlackBoard
#include "EBehaviorTree.h" // Create BehaviorTree

#include <array> // Inventory

class IBaseInterface;
class IExamInterface;

class Plugin : public IExamPlugin
{
public:
	Plugin() {};
	virtual ~Plugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void Update(float dt) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;

	void Render(float dt) const override;

private:
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;

	SteeringPlugin_Output m_SteeringOutput;

	std::vector<HouseInfo> GetHousesInFOV() const;
	std::vector<EntityInfo> GetEntitiesInFOV() const;

	Elite::Vector2 m_Target = {};

	bool m_CanRun = false; //Demo purpose
	bool m_GrabItem = false; //Demo purpose
	bool m_UseItem = false; //Demo purpose
	bool m_RemoveItem = false; //Demo purpose
	float m_AngSpeed = 0.f; //Demo purpose

	UINT m_InventorySlot = 0;

	//--------------------------------------------------------

	Elite::Blackboard* CreateBlackboard();
	Elite::BehaviorTree* m_pBehaviorTree = nullptr;

	std::vector<HouseInfo> m_HousesInFOV;
	std::vector<HouseInfo> m_HousesICanLoot;
	std::vector<HouseInfo> m_HousesLooted;

	std::vector<EntityInfo> m_EntityInFOV;
	std::vector<EntityInfo> m_ItemsICanLoot;
	std::vector<EntityInfo> m_ItemsLooted;

	std::vector<EntityInfo> m_EnemiesInFOV;

	std::vector<EntityInfo> m_PurgeZonesInFOV;

	std::array<std::pair<Elite::Vector2, bool>, 13> m_SearchPoints;

	std::array<ItemInfo, 5> m_Inventory;

	bool m_TurnAround;
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}