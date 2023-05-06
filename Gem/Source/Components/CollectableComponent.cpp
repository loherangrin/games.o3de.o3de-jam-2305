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

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Serialization/SerializeContext.h>

#include <AzFramework/Entity/GameEntityContextBus.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>
#include <AzFramework/Physics/Components/SimulatedBodyComponentBus.h>
#include <AzFramework/Physics/Collision/CollisionEvents.h>

#include "../EBuses/CollectableBus.hpp"
#include "CollectableComponent.hpp"

using Loherangrin::Games::O3DEJam2305::CollectableComponent;


void CollectableComponent::Reflect(AZ::ReflectContext* io_context)
{
	if(auto serializeContext = azrtti_cast<AZ::SerializeContext*>(io_context))
	{
		serializeContext->Class<CollectableComponent, AZ::Component>()
			->Version(0)
			->Field("Type", &CollectableComponent::m_type)
			->Field("Amount", &CollectableComponent::m_amount)
			->Field("Duration", &CollectableComponent::m_duration)
		;

		if(AZ::EditContext* editContext = serializeContext->GetEditContext())
		{
			editContext->Class<CollectableComponent>("Collectable", "Collectable")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

				->DataElement(AZ::Edit::UIHandlers::Default, &CollectableComponent::m_type, "Type", "")
				->DataElement(AZ::Edit::UIHandlers::Default, &CollectableComponent::m_amount, "Amount", "")
				->DataElement(AZ::Edit::UIHandlers::Default, &CollectableComponent::m_duration, "Duration", "")
			;
		}
	}
}

void CollectableComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& io_provided)
{
	io_provided.push_back(AZ_CRC_CE("CollectableService"));
}

void CollectableComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& io_incompatible)
{
	io_incompatible.push_back(AZ_CRC_CE("CollectableService"));
}

void CollectableComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& io_required)
{
	io_required.push_back(AZ_CRC_CE("PhysicsTriggerService"));
	io_required.push_back(AZ_CRC_CE("PhysicsRigidBodyService"));
}

void CollectableComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& io_dependent)
{}

void CollectableComponent::Init()
{
	m_triggerEnterHandler = AzPhysics::SimulatedBodyEvents::OnTriggerEnter::Handler([this]([[maybe_unused]] AzPhysics::SimulatedBodyHandle i_bodyHandle, const AzPhysics::TriggerEvent& i_trigger)
	{
		switch(m_type)
		{
			case CollectableType::STOP_DECAY:
			{
				EBUS_EVENT(CollectablesNotificationBus, OnStopDecayCollected, m_duration);
			}
			break;

			case CollectableType::SPACESHIP_ENERGY:
			{
				EBUS_EVENT(CollectablesNotificationBus, OnSpaceshipEnergyCollected, m_amount);
			}
			break;

			case CollectableType::TILE_ENERGY:
			{
				EBUS_EVENT(CollectablesNotificationBus, OnTileEnergyCollected, m_amount);
			}
			break;
		}

		EBUS_EVENT(AzFramework::GameEntityContextRequestBus, DestroyGameEntityAndDescendants, GetEntityId());
	});
}

void CollectableComponent::Activate()
{
	Physics::RigidBodyNotificationBus::Handler::BusConnect(GetEntityId());
}

void CollectableComponent::Deactivate()
{
	Physics::RigidBodyNotificationBus::Handler::BusDisconnect();

	m_triggerEnterHandler.Disconnect();
}

void CollectableComponent::OnPhysicsEnabled(const AZ::EntityId& i_entityId)
{
	if(i_entityId != GetEntityId())
	{
		return;
	}

	AzPhysics::SimulatedBody* collider { nullptr };
	EBUS_EVENT_ID_RESULT(collider, i_entityId, AzPhysics::SimulatedBodyComponentRequestsBus, GetSimulatedBody);

	AZ_Assert(collider, "Collider cannot be null");

	collider->RegisterOnTriggerEnterHandler(m_triggerEnterHandler);

	Physics::RigidBodyNotificationBus::Handler::BusDisconnect();
}
