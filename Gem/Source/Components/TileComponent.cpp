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

#include "TileComponent.hpp"

using Loherangrin::Games::O3DEJam2305::TileComponent;


void TileComponent::Reflect(AZ::ReflectContext* io_context)
{
	if(auto serializeContext = azrtti_cast<AZ::SerializeContext*>(io_context))
	{
		serializeContext->Class<TileComponent, AZ::Component>()
			->Version(0)
			->Field("Mesh", &TileComponent::m_meshEntityId)
			->Field("Energy", &TileComponent::m_maxEnergy)
			->Field("Decay", &TileComponent::m_decaySpeed)
			->Field("Flip", &TileComponent::m_flipSpeed)
			->Field("ShakeSpeed", &TileComponent::m_shakeSpeed)
			->Field("ShakeHeight", &TileComponent::m_maxShakeHeight)
		;

		if(AZ::EditContext* editContext = serializeContext->GetEditContext())
		{
			editContext->Class<TileComponent>("Tile", "Tile")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

				->DataElement(AZ::Edit::UIHandlers::Default, &TileComponent::m_meshEntityId, "Mesh", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "Energy")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &TileComponent::m_maxEnergy, "Max", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &TileComponent::m_decaySpeed, "Decay", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "Animation")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &TileComponent::m_shakeSpeed, "Alert", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &TileComponent::m_flipSpeed, "Toggle", "")
			;
		}
	}
}

void TileComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& io_provided)
{
	io_provided.push_back(AZ_CRC_CE("TileService"));
}

void TileComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& io_incompatible)
{
	io_incompatible.push_back(AZ_CRC_CE("TileService"));
}

void TileComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& io_required)
{
	io_required.push_back(AZ_CRC_CE("TransformService"));
}

void TileComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& io_dependent)
{}

void TileComponent::Activate()
{
	if(m_isClaimed)
	{
		AZ::TickBus::Handler::BusConnect();
	}

	const AZ::EntityId thisEntityId = GetEntityId();

	TileRequestBus::Handler::BusConnect(thisEntityId);
}

void TileComponent::Deactivate()
{
	TileRequestBus::Handler::BusDisconnect();
	AZ::TickBus::Handler::BusDisconnect();
}

void TileComponent::OnTick(float i_deltaTime, [[maybe_unused]] AZ::ScriptTimePoint i_time)
{
	if(m_isRecharging)
	{
		m_isRecharging = false;
	}
	else
	{
		Decay(i_deltaTime);
	}

	PlayAnimation(i_deltaTime);

	if(m_animation == Animation::NONE && m_energy < AZ::Constants::FloatEpsilon)
	{
		AZ::TickBus::Handler::BusDisconnect();
	}
}

void TileComponent::Decay(float i_deltaTime)
{
	const float lostEnergy = m_decaySpeed * i_deltaTime;
	SubtractEnergy(lostEnergy);
}

void TileComponent::SubtractEnergy(float i_amount)
{
	AddEnergy(-i_amount);
}

void TileComponent::AddEnergy(float i_amount)
{
	m_energy = AZStd::clamp(m_energy + i_amount, 0.f, m_maxEnergy);

	const bool isAdded = (i_amount > 0.f);
	if(isAdded)
	{
		m_isRecharging = true;
		if(!m_isClaimed && m_energy > THRESHOLDS_TOGGLE)
		{
			Toggle();
		}
		else if(m_isClaimed && m_energy > THRESHOLDS_ALERT)
		{
			StopAnimation();
		}
	}
	else
	{
		if(m_isClaimed)
		{
			if(m_energy < THRESHOLDS_TOGGLE)
			{
				Toggle();
			}
			else if(m_energy < THRESHOLDS_ALERT)
			{
				Alert();
			}
		}
	}
}

void TileComponent::Alert()
{
	if(m_animation == Animation::SHAKE)
	{
		return;
	}

	StopAnimation();

	m_animation = Animation::SHAKE;
	m_animationParameter = 0.5f;

	m_startHeight = -m_maxShakeHeight;
	m_endHeight = m_maxShakeHeight;

	if(!AZ::TickBus::Handler::BusIsConnected())
	{
		AZ::TickBus::Handler::BusConnect();
	}
}

void TileComponent::Toggle()
{
	if(m_animation == Animation::FLIP)
	{
		return;
	}

	StopAnimation();

	m_animation = Animation::FLIP;
	m_animationParameter = 0.f;

	const float startAngle = (m_isClaimed) ? AZ::Constants::Pi : 0.f;
	const float endAngle = (m_isClaimed) ? 0.f : AZ::Constants::Pi;

	m_startRotation =  AZ::Quaternion::CreateRotationX(startAngle);
	m_endRotation = AZ::Quaternion::CreateRotationX(endAngle);

	m_isClaimed = !m_isClaimed;

	if(!AZ::TickBus::Handler::BusIsConnected())
	{
		AZ::TickBus::Handler::BusConnect();
	}
}

void TileComponent::PlayAnimation(float i_deltaTime)
{
	switch(m_animation)
	{
		case Animation::FLIP:
		{
			PlayFlipAnimation(i_deltaTime);
		}
		break;

		case Animation::SHAKE:
		{
			PlayShakeAnimation(i_deltaTime);
		}
		break;
	}
}

void TileComponent::PlayFlipAnimation(float i_deltaTime)
{
	m_animationParameter += m_flipSpeed * i_deltaTime;

	if(m_animationParameter > 1.f)
	{
		m_animationParameter = 1.f;
		m_animation = Animation::NONE;
	}

	const AZ::Quaternion rotation = m_startRotation.Slerp(m_endRotation, m_animationParameter);
	
	EBUS_EVENT_ID(m_meshEntityId, AZ::TransformBus, SetLocalRotationQuaternion, rotation);
}

void TileComponent::PlayShakeAnimation(float i_deltaTime)
{
	m_animationParameter += m_shakeSpeed * i_deltaTime;

	if(m_animationParameter > 1.f)
	{
		m_animationParameter = 0.f;

		AZStd::swap(m_startHeight, m_endHeight);
	}

	const float height = AZ::Lerp(m_startHeight, m_endHeight, m_animationParameter);

	EBUS_EVENT_ID(m_meshEntityId, AZ::TransformBus, SetLocalZ, height);
}

void TileComponent::StopAnimation()
{
	switch(m_animation)
	{
		case Animation::NONE:
		{
			return;
		}
		break;
	
		case Animation::SHAKE:
		{
			StopShakeAnimation();
		}
		break;
	}

	m_animation = Animation::NONE;
}

void TileComponent::StopShakeAnimation()
{
	const float initialHeight = AZ::Lerp(m_startHeight, m_endHeight, 0.5f);

	EBUS_EVENT_ID(m_meshEntityId, AZ::TransformBus, SetLocalZ, initialHeight);
}
