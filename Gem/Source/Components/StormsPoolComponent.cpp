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

#include "../EBuses/TileBus.hpp"
#include "StormComponent.hpp"
#include "StormsPoolComponent.hpp"

using Loherangrin::Games::O3DEJam2305::StormsPoolComponent;


void StormsPoolComponent::Reflect(AZ::ReflectContext* io_context)
{
	if(auto serializeContext = azrtti_cast<AZ::SerializeContext*>(io_context))
	{
		serializeContext->Class<StormsPoolComponent, AZ::Component>()
			->Version(0)
			->Field("Storm", &StormsPoolComponent::m_stormPrefab)
			->Field("Seed", &StormsPoolComponent::m_randomSeed)
			->Field("DurationMin", &StormsPoolComponent::m_minStormDuration)
			->Field("DurationMax", &StormsPoolComponent::m_maxStormDuration)
			->Field("SpeedMin", &StormsPoolComponent::m_minStormSpeed)
			->Field("SpeedMax", &StormsPoolComponent::m_maxStormSpeed)
			->Field("StrengthMin", &StormsPoolComponent::m_minStormStrength)
			->Field("StrengthMax", &StormsPoolComponent::m_maxStormStrength)
			->Field("Height", &StormsPoolComponent::m_stormHeight)
			->Field("Delay", &StormsPoolComponent::m_spawnDelay)
		;

		if(AZ::EditContext* editContext = serializeContext->GetEditContext())
		{
			editContext->Class<StormsPoolComponent>("Storms Pool", "Storms Pool")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

				->DataElement(AZ::Edit::UIHandlers::Default, &StormsPoolComponent::m_stormPrefab, "Prefab", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "Random")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &StormsPoolComponent::m_randomSeed, "Seed", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "Duration")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &StormsPoolComponent::m_minStormDuration, "Min", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &StormsPoolComponent::m_maxStormDuration, "Max", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "Speed")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &StormsPoolComponent::m_minStormSpeed, "Min", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &StormsPoolComponent::m_maxStormSpeed, "Max", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "Strength")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &StormsPoolComponent::m_minStormStrength, "Min", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &StormsPoolComponent::m_maxStormStrength, "Max", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "")

				->DataElement(AZ::Edit::UIHandlers::Default, &StormsPoolComponent::m_stormHeight, "Height", "")
				->DataElement(AZ::Edit::UIHandlers::Default, &StormsPoolComponent::m_spawnDelay, "Delay", "")
			;
		}
	}
}

void StormsPoolComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& io_provided)
{
	io_provided.push_back(AZ_CRC_CE("StormsPoolService"));
}

void StormsPoolComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& io_incompatible)
{
	io_incompatible.push_back(AZ_CRC_CE("StormsPoolService"));
}

void StormsPoolComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& io_required)
{}

void StormsPoolComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& io_dependent)
{}

void StormsPoolComponent::Init()
{
	m_stormSpawnTicket = AzFramework::EntitySpawnTicket { m_stormPrefab };

	m_randomGenerator.SetSeed(m_randomSeed);
	m_timer = m_spawnDelay;
}

void StormsPoolComponent::Activate()
{
	GameNotificationBus::Handler::BusConnect();
}

void StormsPoolComponent::Deactivate()
{
	GameNotificationBus::Handler::BusDisconnect();
	AZ::TickBus::Handler::BusDisconnect();

	DestroyAllStorms();
}

void StormsPoolComponent::OnGameLoading()
{
	DestroyAllStorms();
}

void StormsPoolComponent::OnGameStarted()
{
	OnGameResumed();
}

void StormsPoolComponent::OnGamePaused()
{
	AZ::TickBus::Handler::BusDisconnect();
}

void StormsPoolComponent::OnGameResumed()
{
	AZ::TickBus::Handler::BusConnect();
}

void StormsPoolComponent::OnGameEnded()
{
	OnGamePaused();
}

void StormsPoolComponent::OnTick(float i_deltaTime, [[maybe_unused]] AZ::ScriptTimePoint i_time)
{	
	m_timer -= i_deltaTime;

	if(m_timer > 0.f)
	{
		return;
	}

	while(m_timer < 0.f)
	{
		m_timer += m_spawnDelay;
	}

	CreateStorm();
}

void StormsPoolComponent::CreateStorm()
{
	AzFramework::SpawnAllEntitiesOptionalArgs spawnOptions;
	
	spawnOptions.m_preInsertionCallback = [this]([[maybe_unused]] AzFramework::EntitySpawnTicket::Id i_spawnTicketId, AzFramework::SpawnableEntityContainerView i_newEntities)
	{
		if(i_newEntities.empty())
		{
			AZ_Error("StormsPool", false, "Unable to spawn tiles. Please check if a prefab is assigned");
			return;
		}

		AZ::Entity* newEntity = *(i_newEntities.begin() + 1);
		auto newStorm = newEntity->FindComponent<StormComponent>();
		newStorm->m_duration = GenerateRandomInRange(m_minStormDuration, m_maxStormDuration);
		newStorm->m_strength = GenerateRandomInRange(m_minStormStrength, m_maxStormStrength);

		newStorm->m_moveDirection = AZ::Vector3
		{
			m_randomGenerator.GetRandomFloat(),
			m_randomGenerator.GetRandomFloat(),
			0.f
		}.GetNormalized();

		newStorm->m_moveSpeed = GenerateRandomInRange(m_minStormSpeed, m_maxStormSpeed);
	};

	spawnOptions.m_completionCallback = [this]([[maybe_unused]] AzFramework::EntitySpawnTicket::Id i_spawnTicketId, AzFramework::SpawnableConstEntityContainerView i_newEntities)
	{
		if(i_newEntities.empty())
		{
			AZ_Error("StormsPool", false, "Unable to spawn tiles. Please check if a prefab is assigned");
			return;
		}

		AZ::Vector2 halfGridSize { AZ::Vector2::CreateZero() };
		EBUS_EVENT_RESULT(halfGridSize, TilesRequestBus, GetGridSize);

		halfGridSize /= 2.f;

		AZ::Vector3 worldTranslation
		{
			GenerateRandomInRange(-halfGridSize.GetX(), halfGridSize.GetX()),
			GenerateRandomInRange(-halfGridSize.GetY(), halfGridSize.GetY()),
			m_stormHeight
		};

		const AZ::Entity* newRootEntity = *(i_newEntities.begin());
		const AZ::EntityId newRootEntityId = newRootEntity->GetId();

		EBUS_EVENT_ID(newRootEntityId, AZ::TransformBus, SetWorldTranslation, worldTranslation);
	};

	auto spawnableSystem = AzFramework::SpawnableEntitiesInterface::Get();
    AZ_Assert(spawnableSystem, "Unable to retrieve the main spawnable system");

	spawnableSystem->SpawnAllEntities(m_stormSpawnTicket, AZStd::move(spawnOptions));
}

void StormsPoolComponent::DestroyAllStorms()
{
	AzFramework::DespawnAllEntitiesOptionalArgs despawnOptions;

	auto spawnableSystem = AzFramework::SpawnableEntitiesInterface::Get();
    AZ_Assert(spawnableSystem, "Unable to retrieve the main spawnable system");

	spawnableSystem->DespawnAllEntities(m_stormSpawnTicket, AZStd::move(despawnOptions));
}

float StormsPoolComponent::GenerateRandomInRange(float i_min, float i_max)
{
	return (i_min + (m_randomGenerator.GetRandomFloat() * (i_max - i_min)));
}
