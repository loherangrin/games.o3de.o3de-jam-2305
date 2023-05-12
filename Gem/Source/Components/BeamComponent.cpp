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

#include <AtomLyIntegration/CommonFeatures/Mesh/MeshComponentBus.h>

#include <AzCore/Component/TransformBus.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Serialization/SerializeContext.h>

#include <AzFramework/Input/Devices/Keyboard/InputDeviceKeyboard.h>
#include <AzFramework/Physics/Common/PhysicsTypes.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>
#include <AzFramework/Physics/Components/SimulatedBodyComponentBus.h>
#include <AzFramework/Physics/Collision/CollisionEvents.h>

#include "../EBuses/TileBus.hpp"
#include "BeamComponent.hpp"

using Loherangrin::Games::O3DEJam2305::BeamComponent;


void BeamComponent::Reflect(AZ::ReflectContext* io_context)
{
	if(auto serializeContext = azrtti_cast<AZ::SerializeContext*>(io_context))
	{
		serializeContext->Class<BeamComponent, AZ::Component>()
			->Version(0)
			->Field("Transfer", &BeamComponent::m_transferSpeed)
		;

		if(AZ::EditContext* editContext = serializeContext->GetEditContext())
		{
			editContext->Class<BeamComponent>("Beam", "Beam")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

				->DataElement(AZ::Edit::UIHandlers::Default, &BeamComponent::m_transferSpeed, "Transfer", "")
			;
		}
	}
}

void BeamComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& io_provided)
{
	io_provided.push_back(AZ_CRC_CE("BeamService"));
}

void BeamComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& io_incompatible)
{
	io_incompatible.push_back(AZ_CRC_CE("BeamService"));
}

void BeamComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& io_required)
{
	io_required.push_back(AZ_CRC_CE("MeshService"));
	io_required.push_back(AZ_CRC_CE("PhysicsTriggerService"));
	io_required.push_back(AZ_CRC_CE("PhysicsRigidBodyService"));
}

void BeamComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& io_dependent)
{}

void BeamComponent::Init()
{
	m_triggerEnterHandler = AzPhysics::SimulatedBodyEvents::OnTriggerEnter::Handler([this]([[maybe_unused]] AzPhysics::SimulatedBodyHandle i_bodyHandle, const AzPhysics::TriggerEvent& i_trigger)
	{
		if(!i_trigger.m_otherBody)
		{
			return;
		}

		const AZ::EntityId otherEntityId = i_trigger.m_otherBody->GetEntityId();
		SelectTile(otherEntityId);
	});

	m_triggerExitHandler = AzPhysics::SimulatedBodyEvents::OnTriggerExit::Handler([this]([[maybe_unused]] AzPhysics::SimulatedBodyHandle i_bodyHandle, const AzPhysics::TriggerEvent& i_trigger)
	{
		if(!i_trigger.m_otherBody)
		{
			return;
		}

		const AZ::EntityId otherEntityId = i_trigger.m_otherBody->GetEntityId();
		DeselectTile(otherEntityId);
	});
}

void BeamComponent::Activate()
{
	Physics::RigidBodyNotificationBus::Handler::BusConnect(GetEntityId());

	SpaceshipNotificationBus::Handler::BusConnect();
	GameNotificationBus::Handler::BusConnect();

	if(m_isEnabled)
	{
		TurnOn();
	}
	else
	{
		TurnOff();
	}
}

void BeamComponent::Deactivate()
{
	GameNotificationBus::Handler::BusDisconnect();

	InputChannelEventListener::Disconnect();
	AZ::TickBus::Handler::BusDisconnect();

	SpaceshipNotificationBus::Handler::BusDisconnect();

	if(m_isEnabled)
	{
		DisconnectTriggerHandlers();
	}

	Physics::RigidBodyNotificationBus::Handler::BusDisconnect();
}

void BeamComponent::OnGameLoading()
{
	if(m_isEnabled)
	{
		TurnOff();
	}

	m_isLocked = false;
	m_selectedTiles.clear();
}

void BeamComponent::OnGameStarted()
{
	OnGameResumed();
}

void BeamComponent::OnGamePaused()
{
	InputChannelEventListener::Disconnect();
	AZ::TickBus::Handler::BusDisconnect();
}

void BeamComponent::OnGameResumed()
{
	if(m_isEnabled)
	{
		AZ::TickBus::Handler::BusConnect();
	}

	InputChannelEventListener::Connect();
}

void BeamComponent::OnGameEnded()
{
	OnGamePaused();
}

void BeamComponent::OnGameDestroyed()
{
	if(m_isEnabled)
	{
		TurnOff();
	}
}

void BeamComponent::OnPhysicsEnabled(const AZ::EntityId& i_entityId)
{
	if(i_entityId != GetEntityId())
	{
		return;
	}

	EBUS_EVENT_ID(i_entityId, AZ::TransformBus, SetOnParentChangedBehavior, AZ::OnParentChangedBehavior::Update);

	Physics::RigidBodyNotificationBus::Handler::BusDisconnect();
}

void BeamComponent::OnTick(float i_deltaTime, [[maybe_unused]] AZ::ScriptTimePoint i_time)
{
	TransferEnergyToTiles(i_deltaTime);
}

bool BeamComponent::OnInputChannelEventFiltered(const AzFramework::InputChannel& i_inputChannel)
{
	const AzFramework::InputChannelId& channelId = i_inputChannel.GetInputChannelId();

	// TurnOn / TurnOff
	if(channelId == AzFramework::InputDeviceKeyboard::Key::EditSpace)
	{
		const AzFramework::InputChannel::State inputState = i_inputChannel.GetState();
		if(inputState != AzFramework::InputChannel::State::Began)
		{
			return false;
		}

		Toggle();
	}
	else
	{
		return false;
	}

	return true;
}

void BeamComponent::Toggle()
{
	if(m_isEnabled)
	{
		TurnOff();
	}
	else
	{
		TurnOn();
	}
}

void BeamComponent::TurnOn()
{
	if(m_isLocked)
	{
		return;
	}

	m_isEnabled = true;

	ConnectTriggerHandlers();

	EBUS_EVENT_ID(GetEntityId(), AZ::Render::MeshComponentRequestBus, SetVisibility, true);
}

void BeamComponent::TurnOff()
{
	m_isEnabled = false;

	AZ::TickBus::Handler::BusDisconnect();
	DisconnectTriggerHandlers();

	m_selectedTiles.clear();

	EBUS_EVENT_ID(GetEntityId(), AZ::Render::MeshComponentRequestBus, SetVisibility, false);
}

void BeamComponent::ConnectTriggerHandlers()
{
	const AZ::EntityId thisEntityId = GetEntityId();

	AzPhysics::SimulatedBody* collider { nullptr };
	EBUS_EVENT_ID_RESULT(collider, thisEntityId, AzPhysics::SimulatedBodyComponentRequestsBus, GetSimulatedBody);

	AZ_Assert(collider, "No collider was found");

	collider->RegisterOnTriggerEnterHandler(m_triggerEnterHandler);
	collider->RegisterOnTriggerExitHandler(m_triggerExitHandler);
}

void BeamComponent::DisconnectTriggerHandlers()
{
	m_triggerEnterHandler.Disconnect();
	m_triggerExitHandler.Disconnect();
}

void BeamComponent::SelectTile(const AZ::EntityId& i_tileEntityId)
{
	m_selectedTiles.emplace(i_tileEntityId);

	if(!AZ::TickBus::Handler::BusIsConnected())
	{
		AZ::TickBus::Handler::BusConnect();
	}
}

void BeamComponent::DeselectTile(const AZ::EntityId& i_tileEntityId)
{
	m_selectedTiles.extract(i_tileEntityId);

	if(m_selectedTiles.empty())
	{
		AZ::TickBus::Handler::BusDisconnect();
	}
}

void BeamComponent::TransferEnergyToTiles(float i_deltaTime)
{
	if(m_selectedTiles.empty())
	{
		return;
	}

	const float sentEnergy = m_transferSpeed * i_deltaTime;
	const float tileEnergy = sentEnergy / static_cast<float>(m_selectedTiles.size());

	for(const AZ::EntityId& tileEntityId : m_selectedTiles)
	{
		EBUS_EVENT_ID(tileEntityId, TileRequestBus, AddEnergy, tileEnergy);
	}

	EBUS_EVENT(SpaceshipRequestBus, SubtractEnergy, sentEnergy);
}

void BeamComponent::OnEnergySavingModeActivated()
{
	m_isLocked = true;

	if(m_isEnabled)
	{
		TurnOff();
	}
}

void BeamComponent::OnEnergySavingModeDeactivated()
{
	m_isLocked = false;
}

void BeamComponent::OnLandingStarted()
{
	if(m_isEnabled)
	{
		TurnOff();

		m_isLocked = true;
	}
}

void BeamComponent::OnTakeOffEnded()
{
	m_isLocked = false;
}
