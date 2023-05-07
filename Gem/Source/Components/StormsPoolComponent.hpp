/* Submission to O3DE Jam - May 5-7, 2023
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
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Random.h>

#include <AzFramework/Spawnable/SpawnableEntitiesInterface.h>
#include <AzFramework/Spawnable/Spawnable.h>


namespace Loherangrin::Games::O3DEJam2305
{
	class StormsPoolComponent
		: public AZ::Component
		, protected AZ::TickBus::Handler
	{
	public:
		AZ_COMPONENT(StormsPoolComponent, "{C66C7EBA-D5DF-4331-9B67-38123276A580}");
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

		// AZ::TickBus
		void OnTick(float i_deltaTime, AZ::ScriptTimePoint i_time) override;

	private:
		void CreateStorm();
		void DestroyAllStorms();

		float GenerateRandomInRange(float i_min, float i_max);

		float m_spawnDelay { 15.f };
		float m_timer { -1.f };

		float m_stormHeight { 1.f };

		float m_minStormDuration { 5.f };
		float m_maxStormDuration { 15.f };

		float m_minStormSpeed { 1.f };
		float m_maxStormSpeed { 4.f };

		float m_minStormStrength { 5.f };
		float m_maxStormStrength { 10.f };

		AZ::Data::Asset<AzFramework::Spawnable> m_stormPrefab {};
		AzFramework::EntitySpawnTicket m_stormSpawnTicket {};

		AZ::u64 m_randomSeed { 1234 };
		AZ::SimpleLcgRandom m_randomGenerator {};
	};

 } // Loherangrin::Games::O3DEJam2305
