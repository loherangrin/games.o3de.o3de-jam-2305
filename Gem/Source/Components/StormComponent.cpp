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

#include <AzCore/Component/TransformBus.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Serialization/SerializeContext.h>

#include <AzFramework/Entity/GameEntityContextBus.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>
#include <AzFramework/Physics/Components/SimulatedBodyComponentBus.h>
#include <AzFramework/Physics/Collision/CollisionEvents.h>

#include "../EBuses/TileBus.hpp"
#include "../EBuses/SpaceshipBus.hpp"
#include "StormComponent.hpp"

using Loherangrin::Games::O3DEJam2305::StormComponent;


void StormComponent::Reflect(AZ::ReflectContext* io_context)
{
	if(auto serializeContext = azrtti_cast<AZ::SerializeContext*>(io_context))
	{
		serializeContext->Class<StormComponent, AZ::Component>()
			->Version(0)
			->Field("Mesh", &StormComponent::m_meshEntityId)
			->Field("Strength", &StormComponent::m_strength)
			->Field("Direction", &StormComponent::m_moveDirection)
			->Field("Speed", &StormComponent::m_moveSpeed)
			->Field("Animation", &StormComponent::m_animationSpeed)
			->Field("Duration", &StormComponent::m_duration)
		;

		if(AZ::EditContext* editContext = serializeContext->GetEditContext())
		{
			editContext->Class<StormComponent>("Storm", "Storm")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

				->DataElement(AZ::Edit::UIHandlers::Default, &StormComponent::m_meshEntityId, "Mesh", "")

				->DataElement(AZ::Edit::UIHandlers::Default, &StormComponent::m_strength, "Strength", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "Movement")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)
	
					->DataElement(AZ::Edit::UIHandlers::Default, &StormComponent::m_moveDirection, "Direction", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &StormComponent::m_moveSpeed, "Speed", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "Animation")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &StormComponent::m_animationSpeed, "Speed", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "")

				->DataElement(AZ::Edit::UIHandlers::Default, &StormComponent::m_duration, "Duration", "")
			;
		}
	}
}

void StormComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& io_provided)
{
	io_provided.push_back(AZ_CRC_CE("StormService"));
}

void StormComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& io_incompatible)
{
	io_incompatible.push_back(AZ_CRC_CE("StormService"));
}

void StormComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& io_required)
{
	io_required.push_back(AZ_CRC_CE("PhysicsTriggerService"));
	io_required.push_back(AZ_CRC_CE("PhysicsRigidBodyService"));
}

void StormComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& io_dependent)
{
	io_dependent.push_back(AZ_CRC("TransformService", 0x8ee22c50));
}

void StormComponent::Init()
{
	m_triggerEnterHandler = AzPhysics::SimulatedBodyEvents::OnTriggerEnter::Handler([this]([[maybe_unused]] AzPhysics::SimulatedBodyHandle i_bodyHandle, const AzPhysics::TriggerEvent& i_trigger)
	{
		if(!i_trigger.m_otherBody)
		{
			return;
		}

		const AZ::EntityId otherEntityId = i_trigger.m_otherBody->GetEntityId();
		if(TileRequestBus::HasHandlers(otherEntityId))
		{
			bool isClaimed { false };
			EBUS_EVENT_ID_RESULT(isClaimed, otherEntityId, TileRequestBus, IsClaimed);

			if(isClaimed)
			{
				m_hitTileEntityIds.emplace(otherEntityId);
			}
		}
		else
		{
			m_isSpaceshipHit = true;
		}
	});

	m_triggerExitHandler = AzPhysics::SimulatedBodyEvents::OnTriggerExit::Handler([this]([[maybe_unused]] AzPhysics::SimulatedBodyHandle i_bodyHandle, const AzPhysics::TriggerEvent& i_trigger)
	{
		if(!i_trigger.m_otherBody)
		{
			return;
		}

		const AZ::EntityId otherEntityId = i_trigger.m_otherBody->GetEntityId();
		if(TileRequestBus::HasHandlers(otherEntityId))
		{
			m_hitTileEntityIds.extract(otherEntityId);
		}
		else
		{
			m_isSpaceshipHit = false;
		}
	});

	m_timer = m_duration;
	m_animationSpeed = AZ::DegToRad(m_animationSpeed);
}

void StormComponent::Activate()
{
	Physics::RigidBodyNotificationBus::Handler::BusConnect(GetEntityId());
}

void StormComponent::OnPhysicsEnabled(const AZ::EntityId& i_entityId)
{
	if(i_entityId != GetEntityId())
	{
		return;
	}

	Physics::RigidBodyNotificationBus::Handler::BusDisconnect();

	EBUS_EVENT_ID(i_entityId, AZ::TransformBus, SetOnParentChangedBehavior, AZ::OnParentChangedBehavior::Update);

	AzPhysics::SimulatedBody* collider { nullptr };
	EBUS_EVENT_ID_RESULT(collider, i_entityId, AzPhysics::SimulatedBodyComponentRequestsBus, GetSimulatedBody);

	AZ_Assert(collider, "No collider was found");

	collider->RegisterOnTriggerEnterHandler(m_triggerEnterHandler);
	collider->RegisterOnTriggerExitHandler(m_triggerExitHandler);

	AZ::TickBus::Handler::BusConnect();
}

void StormComponent::Deactivate()
{
	AZ::TickBus::Handler::BusDisconnect();
	Physics::RigidBodyNotificationBus::Handler::BusDisconnect();

	m_triggerEnterHandler.Disconnect();
	m_triggerExitHandler.Disconnect();
}

void StormComponent::OnTick(float i_deltaTime, [[maybe_unused]] AZ::ScriptTimePoint i_time)
{	
	m_timer -= i_deltaTime;

	if(m_timer < 0.f)
	{
		AZ::TickBus::Handler::BusDisconnect();

		EBUS_EVENT(AzFramework::GameEntityContextRequestBus, DestroyGameEntityAndDescendants, GetEntityId());
	}

	ApplyMovement(i_deltaTime);
	PlayAnimation(i_deltaTime);

	ApplyDamages(i_deltaTime);
}

void StormComponent::ApplyMovement(float i_deltaTime)
{
	const AZ::Vector3 linearOffset = m_moveDirection * (m_moveSpeed * i_deltaTime);

	EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus, MoveEntity, linearOffset);
}

void StormComponent::PlayAnimation(float i_deltaTime)
{
	const float angularOffset = m_animationSpeed * i_deltaTime;

	EBUS_EVENT_ID(m_meshEntityId, AZ::TransformBus, RotateAroundLocalZ, angularOffset);
}

void StormComponent::ApplyDamages(float i_deltaTime)
{
	const float damage = m_strength * i_deltaTime;

	if(m_isSpaceshipHit)
	{
		EBUS_EVENT(SpaceshipRequestBus, SubtractEnergy, damage);
	}

	for(const AZ::EntityId& tileEntityId : m_hitTileEntityIds)
	{
		EBUS_EVENT_ID(tileEntityId, TileRequestBus, SubtractEnergy, damage);
	}
}
