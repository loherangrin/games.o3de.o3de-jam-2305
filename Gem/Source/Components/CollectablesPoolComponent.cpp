/* Submission to O3DE Jam - May 5-14, 2023
 * Copyright 2023 Matteo Grasso
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Serialization/SerializeContext.h>

#include "CollectablesPoolComponent.hpp"

using Loherangrin::Games::O3DEJam2305::CollectablesPoolComponent;


void CollectablesPoolComponent::Reflect(AZ::ReflectContext* io_context)
{
	if(auto serializeContext = azrtti_cast<AZ::SerializeContext*>(io_context))
	{
		serializeContext->Class<CollectablesPoolComponent, AZ::Component>()
			->Version(0)
			->Field("Seed", &CollectablesPoolComponent::m_collectableSeed)
			->Field("Probability", &CollectablesPoolComponent::m_collectableProbability)
			->Field("Decay", &CollectablesPoolComponent::m_stopDecayPrefab)
			->Field("DamageSpaceship", &CollectablesPoolComponent::m_spaceshipDamagePrefab)
			->Field("DamageTile", &CollectablesPoolComponent::m_tileDamagePrefab)
			->Field("EnergySpaceship", &CollectablesPoolComponent::m_spaceshipEnergyPrefab)
			->Field("EnergyTile", &CollectablesPoolComponent::m_tileEnergyPrefab)
			->Field("PointsSmall", &CollectablesPoolComponent::m_smallPointsPrefab)
			->Field("PointsMedium", &CollectablesPoolComponent::m_mediumPointsPrefab)
			->Field("PointsLarge", &CollectablesPoolComponent::m_largePointsPrefab)
			->Field("SpeedUp", &CollectablesPoolComponent::m_speedUpPrefab)
			->Field("SpeedDown", &CollectablesPoolComponent::m_speedDownPrefab)
			->Field("Height", &CollectablesPoolComponent::m_collectableHeight)
		;

		if(AZ::EditContext* editContext = serializeContext->GetEditContext())
		{
			editContext->Class<CollectablesPoolComponent>("Collectables Pool", "Collectables Pool")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

				->ClassElement(AZ::Edit::ClassElements::Group, "Random")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &CollectablesPoolComponent::m_collectableSeed, "Seed", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &CollectablesPoolComponent::m_collectableProbability, "Probability", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "Prefabs")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)
					
					->DataElement(AZ::Edit::UIHandlers::Default, &CollectablesPoolComponent::m_stopDecayPrefab, "Stop Decay", "")

					->DataElement(AZ::Edit::UIHandlers::Default, &CollectablesPoolComponent::m_spaceshipDamagePrefab, "Damage - Spaceship", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &CollectablesPoolComponent::m_tileDamagePrefab, "Damage - Tile", "")

					->DataElement(AZ::Edit::UIHandlers::Default, &CollectablesPoolComponent::m_spaceshipEnergyPrefab, "Energy - Spaceship", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &CollectablesPoolComponent::m_tileEnergyPrefab, "Energy - Tile", "")

					->DataElement(AZ::Edit::UIHandlers::Default, &CollectablesPoolComponent::m_smallPointsPrefab, "Points - Small", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &CollectablesPoolComponent::m_mediumPointsPrefab, "Points - Medium", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &CollectablesPoolComponent::m_largePointsPrefab, "Points - Large", "")

					->DataElement(AZ::Edit::UIHandlers::Default, &CollectablesPoolComponent::m_speedUpPrefab, "Speed - Up", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &CollectablesPoolComponent::m_speedDownPrefab, "Speed - Down", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "")

					->DataElement(AZ::Edit::UIHandlers::Default, &CollectablesPoolComponent::m_collectableHeight, "Height", "")
			;
		}
	}
}

void CollectablesPoolComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& io_provided)
{
	io_provided.push_back(AZ_CRC_CE("CollectablesPoolService"));
}

void CollectablesPoolComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& io_incompatible)
{
	io_incompatible.push_back(AZ_CRC_CE("CollectablesPoolService"));
}

void CollectablesPoolComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& io_required)
{}

void CollectablesPoolComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& io_dependent)
{}

void CollectablesPoolComponent::Init()
{
	m_collectableSpawnTickets[CollectableType::STOP_DECAY] = AzFramework::EntitySpawnTicket { m_stopDecayPrefab };

	m_collectableSpawnTickets[CollectableType::SPACESHIP_DAMAGE] = AzFramework::EntitySpawnTicket { m_spaceshipDamagePrefab };
	m_collectableSpawnTickets[CollectableType::TILE_DAMAGE] = AzFramework::EntitySpawnTicket { m_tileDamagePrefab };

	m_collectableSpawnTickets[CollectableType::SPACESHIP_ENERGY] = AzFramework::EntitySpawnTicket { m_spaceshipEnergyPrefab };
	m_collectableSpawnTickets[CollectableType::TILE_ENERGY] = AzFramework::EntitySpawnTicket { m_tileEnergyPrefab };

	m_collectableSpawnTickets[CollectableType::SMALL_POINTS] = AzFramework::EntitySpawnTicket { m_smallPointsPrefab };
	m_collectableSpawnTickets[CollectableType::MEDIUM_POINTS] = AzFramework::EntitySpawnTicket { m_mediumPointsPrefab };
	m_collectableSpawnTickets[CollectableType::LARGE_POINTS] = AzFramework::EntitySpawnTicket { m_largePointsPrefab };

	m_collectableSpawnTickets[CollectableType::SPEED_UP] = AzFramework::EntitySpawnTicket { m_speedUpPrefab };
	m_collectableSpawnTickets[CollectableType::SPEED_DOWN] = AzFramework::EntitySpawnTicket { m_speedDownPrefab };

	m_randomGenerator.SetSeed(m_collectableSeed);
}

void CollectablesPoolComponent::Activate()
{
	GameNotificationBus::Handler::BusConnect();
}

void CollectablesPoolComponent::Deactivate()
{
	TilesNotificationBus::Handler::BusDisconnect();
	GameNotificationBus::Handler::BusDisconnect();

	DestroyAllCollectables();
}

void CollectablesPoolComponent::OnGameLoading()
{
	DestroyAllCollectables();
}

void CollectablesPoolComponent::OnGameStarted()
{
	TilesNotificationBus::Handler::BusConnect();
}

void CollectablesPoolComponent::OnGameEnded()
{
	TilesNotificationBus::Handler::BusDisconnect();
}

void CollectablesPoolComponent::OnTileClaimed(const AZ::EntityId& i_tileEntityId)
{
	TryCreateCollectable(i_tileEntityId);
}

void CollectablesPoolComponent::TryCreateCollectable(const AZ::EntityId& i_tileEntityId)
{
	const bool hasCollectable = (m_randomGenerator.GetRandomFloat() < m_collectableProbability);
	if(!hasCollectable)
	{
		return;
	}

	auto collectableType = static_cast<CollectableType>((m_randomGenerator.Getu64Random() % m_collectableSpawnTickets.size()) + 1);

	AzFramework::SpawnAllEntitiesOptionalArgs spawnOptions;
	spawnOptions.m_completionCallback = [this, i_tileEntityId](AzFramework::EntitySpawnTicket::Id i_spawnTicketId, AzFramework::SpawnableConstEntityContainerView i_newEntities)
	{
		if(i_newEntities.empty())
		{
			AZ_Error("TilesPool", false, "Unable to spawn tiles. Please check if a prefab is assigned");
			return;
		}

		AZ::Vector3 worldTranslation { AZ::Vector3::CreateZero() };
		EBUS_EVENT_ID_RESULT(worldTranslation, i_tileEntityId, AZ::TransformBus, GetWorldTranslation);

		worldTranslation += AZ::Vector3 { 0.f, 0.f, m_collectableHeight };

		const AZ::Entity* newRootEntity = *(i_newEntities.begin());
		const AZ::EntityId newRootEntityId = newRootEntity->GetId();

		EBUS_EVENT_ID(newRootEntityId, AZ::TransformBus, SetWorldTranslation, worldTranslation);
	};

	auto spawnableSystem = AzFramework::SpawnableEntitiesInterface::Get();
    AZ_Assert(spawnableSystem, "Unable to retrieve the main spawnable system");

	spawnableSystem->SpawnAllEntities(m_collectableSpawnTickets[collectableType], AZStd::move(spawnOptions));
}

void CollectablesPoolComponent::DestroyAllCollectables()
{
	auto spawnableSystem = AzFramework::SpawnableEntitiesInterface::Get();
    AZ_Assert(spawnableSystem, "Unable to retrieve the main spawnable system");

	for(auto& it : m_collectableSpawnTickets)
	{
		spawnableSystem->DespawnAllEntities(it.second);
	}
}
