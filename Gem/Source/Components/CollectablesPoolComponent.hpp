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

#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Math/Random.h>
#include <AzCore/std/containers/map.h>

#include <AzFramework/Spawnable/SpawnableEntitiesInterface.h>
#include <AzFramework/Spawnable/Spawnable.h>

#include "../EBuses/GameBus.hpp"
#include "../EBuses/TileBus.hpp"
#include "CollectableComponent.hpp"


namespace Loherangrin::Games::O3DEJam2305
{
	class CollectablesPoolComponent
		: public AZ::Component
		, protected GameNotificationBus::Handler
		, protected TilesNotificationBus::Handler
	{
	public:
		AZ_COMPONENT(CollectablesPoolComponent, "{06734ACD-DE93-4A48-887C-3C8AC30F7E3B}");
		static void Reflect(AZ::ReflectContext* io_context);

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& io_provided);
		static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& io_incompatible);
		static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& io_required);
		static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& io_dependent);

	protected:
		// AZ::Component
		void Init() override;
		void Activate() override;
		void Deactivate() override;

		// GameNotificationBus
		void OnGameLoading() override;
		void OnGameStarted() override;
		void OnGameEnded() override;

		// TilesNotificationBus
		void OnTileClaimed(const AZ::EntityId& i_tileEntityId) override;

	private:
		using CollectableType = CollectableComponent::CollectableType;

		void TryCreateCollectable(const AZ::EntityId& i_tileEntityId);
		void DestroyAllCollectables();

		float GenerateRandomInRange(float i_min, float i_max);

		AZ::u64 m_collectableSeed { 1234 };
		float m_collectableProbability { 0.25f };
		float m_collectableHeight { 1.5f };

		float m_minCollectableExpiration { 3.f };
		float m_maxCollectableExpiration { 20.f };

		AZ::Data::Asset<AzFramework::Spawnable> m_stopDecayPrefab {};
		AZ::Data::Asset<AzFramework::Spawnable> m_spaceshipDamagePrefab {};
		AZ::Data::Asset<AzFramework::Spawnable> m_spaceshipEnergyPrefab {};
		AZ::Data::Asset<AzFramework::Spawnable> m_tileDamagePrefab {};
		AZ::Data::Asset<AzFramework::Spawnable> m_tileEnergyPrefab {};
		AZ::Data::Asset<AzFramework::Spawnable> m_smallPointsPrefab {};
		AZ::Data::Asset<AzFramework::Spawnable> m_mediumPointsPrefab {};
		AZ::Data::Asset<AzFramework::Spawnable> m_largePointsPrefab {};
		AZ::Data::Asset<AzFramework::Spawnable> m_speedUpPrefab {};
		AZ::Data::Asset<AzFramework::Spawnable> m_speedDownPrefab {};

    	AZStd::unordered_map<CollectableType, AzFramework::EntitySpawnTicket> m_collectableSpawnTickets {};

		AZ::SimpleLcgRandom m_randomGenerator {};
	};

} // Loherangrin::Games::O3DEJam2305
