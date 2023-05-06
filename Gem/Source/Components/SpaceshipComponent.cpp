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
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Serialization/SerializeContext.h>

#include <AzFramework/Input/Devices/Keyboard/InputDeviceKeyboard.h>
#include <AzFramework/Physics/CharacterBus.h>

#include "SpaceshipComponent.hpp"

using Loherangrin::Games::O3DEJam2305::SpaceshipComponent;


void SpaceshipComponent::Reflect(AZ::ReflectContext* io_context)
{
	if(auto serializeContext = azrtti_cast<AZ::SerializeContext*>(io_context))
	{
		serializeContext->Class<SpaceshipComponent, AZ::Component>()
			->Version(0)
			->Field("Mesh", &SpaceshipComponent::m_meshEntityId)
			->Field("SpeedMove", &SpaceshipComponent::m_moveSpeed)
			->Field("SpeedTurn", &SpaceshipComponent::m_turnSpeed)
			->Field("SpeedLift", &SpaceshipComponent::m_liftSpeed)
			->Field("HeightMin", &SpaceshipComponent::m_minHeight)
			->Field("HeightMax", &SpaceshipComponent::m_maxHeight)
		;

		if(AZ::EditContext* editContext = serializeContext->GetEditContext())
		{
			editContext->Class<SpaceshipComponent>("Spaceship", "Spaceship")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

				->DataElement(AZ::Edit::UIHandlers::Default, &SpaceshipComponent::m_meshEntityId, "Mesh", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "Speed")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &SpaceshipComponent::m_moveSpeed, "Move", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &SpaceshipComponent::m_turnSpeed, "Turn", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &SpaceshipComponent::m_liftSpeed, "Lift", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "Height")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &SpaceshipComponent::m_minHeight, "Min", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &SpaceshipComponent::m_maxHeight, "Max", "")
			;
		}
	}
}

void SpaceshipComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& io_provided)
{
	io_provided.push_back(AZ_CRC_CE("SpaceshipService"));
}

void SpaceshipComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& io_incompatible)
{
	io_incompatible.push_back(AZ_CRC_CE("SpaceshipService"));
}

void SpaceshipComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& io_required)
{
	io_required.push_back(AZ_CRC_CE("PhysicsCharacterControllerService"));
	io_required.push_back(AZ_CRC_CE("TransformService"));
}

void SpaceshipComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& io_dependent)
{}

void SpaceshipComponent::Init()
{
	m_turnSpeed = AZ::DegToRad(m_turnSpeed);
}

void SpaceshipComponent::Activate()
{
	AZ::TickBus::Handler::BusConnect();
	InputChannelEventListener::Connect();
}

void SpaceshipComponent::Deactivate()
{
	InputChannelEventListener::Disconnect();
	AZ::TickBus::Handler::BusDisconnect();
}

void SpaceshipComponent::OnTick(float i_deltaTime, [[maybe_unused]] AZ::ScriptTimePoint i_time)
{
	AZ::Transform thisTransform { AZ::Transform::CreateIdentity() };
	EBUS_EVENT_ID_RESULT(thisTransform, GetEntityId(), AZ::TransformBus, GetWorldTM);

	ApplyRotation(thisTransform, i_deltaTime);
	ApplyHorizontalTranslation(thisTransform);

	ApplyVerticalTranslation(i_deltaTime);
}

bool SpaceshipComponent::OnInputChannelEventFiltered(const AzFramework::InputChannel& i_inputChannel)
{
	const AzFramework::InputChannelId& channelId = i_inputChannel.GetInputChannelId();
	const float channelValue = i_inputChannel.GetValue();

	// MoveBackward
	if(channelId == AzFramework::InputDeviceKeyboard::Key::AlphanumericS)
	{
		if(IsGrounded())
		{
			return false;
		}

		m_moveDirection = -channelValue;
	}
	// MoveForward
	else if(channelId == AzFramework::InputDeviceKeyboard::Key::AlphanumericW)
	{
		if(IsGrounded())
		{
			const AzFramework::InputChannel::State inputState = i_inputChannel.GetState();
			if(inputState != AzFramework::InputChannel::State::Began)
			{
				return false;
			}

			m_liftDirection = 1.f;
		}

		m_moveDirection = channelValue;
	}
	// TurnLeft
	else if(channelId == AzFramework::InputDeviceKeyboard::Key::AlphanumericA)
	{
		m_turnDirection = channelValue;
	}
	// TurnRight
	else if(channelId == AzFramework::InputDeviceKeyboard::Key::AlphanumericD)
	{
		m_turnDirection = -channelValue;
	}
	// TakeOff / Land
	else if(channelId == AzFramework::InputDeviceKeyboard::Key::AlphanumericE)
	{
		const AzFramework::InputChannel::State inputState = i_inputChannel.GetState();
		if(inputState == AzFramework::InputChannel::State::Began)
		{
			m_liftDirection = -1.f;
		}		
	}
	else
	{
		return false;
	}

	return true;
}

void SpaceshipComponent::ApplyHorizontalTranslation(const AZ::Transform& i_transform) const
{
	const AZ::Vector3& forwardAxis = i_transform.GetBasisY();
	const AZ::Vector3 linearVelocity = forwardAxis * (m_moveDirection * m_moveSpeed);

	EBUS_EVENT_ID(GetEntityId(), Physics::CharacterRequestBus, AddVelocityForTick, linearVelocity);
}

void SpaceshipComponent::ApplyVerticalTranslation(float i_deltaTime)
{
	if(AZ::IsClose(m_liftDirection, 0.f, AZ::Constants::FloatEpsilon))
	{
		return;
	}
	
	m_liftParameter += m_liftDirection * m_liftSpeed * i_deltaTime;

	if(m_liftParameter < 0.f)
	{
		m_liftParameter = 0.f;
		m_liftDirection = 0.f;
	}
	else if(m_liftParameter > 1.f)
	{
		m_liftParameter = 1.f;
		m_liftDirection = 0.f;
	}

	const float height = AZ::Lerp(m_minHeight, m_maxHeight, m_liftParameter);

	EBUS_EVENT_ID(m_meshEntityId, AZ::TransformBus, SetLocalZ, height);
}

void SpaceshipComponent::ApplyRotation(const AZ::Transform& i_transform, float i_deltaTime) const
{
	const AZ::Quaternion& rotation = i_transform.GetRotation();

	const float angleOffset = m_turnDirection * m_turnSpeed * i_deltaTime;
	const AZ::Quaternion angularOffset = AZ::Quaternion::CreateRotationZ(angleOffset);

	const AZ::Quaternion newRotation = angularOffset * rotation;

	EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus, SetWorldRotationQuaternion, newRotation);
}

bool SpaceshipComponent::IsGrounded() const
{
	return (m_liftParameter < AZ::Constants::FloatEpsilon);
}
