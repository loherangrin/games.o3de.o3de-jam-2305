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

#include "TileComponent.hpp"

using Loherangrin::Games::O3DEJam2305::TileId;
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
			->Field("ShakeThreshold", &TileComponent::m_alertEnergyThreshold)
			->Field("ShakeSpeed", &TileComponent::m_shakeSpeed)
			->Field("ShakeHeight", &TileComponent::m_maxShakeHeight)
			->Field("ToggleThreshold", &TileComponent::m_toggleEnergyThreshold)
			->Field("Flip", &TileComponent::m_flipSpeed)
			->Field("Select", &TileComponent::m_selectionEntityId)
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

				->ClassElement(AZ::Edit::ClassElements::Group, "Alert")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &TileComponent::m_alertEnergyThreshold, "Energy", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &TileComponent::m_shakeSpeed, "Speed", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &TileComponent::m_maxShakeHeight, "Height", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "Toggle")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &TileComponent::m_toggleEnergyThreshold, "Energy", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &TileComponent::m_flipSpeed, "Speed", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "")

				->DataElement(AZ::Edit::UIHandlers::Default, &TileComponent::m_selectionEntityId, "Selection", "")
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

void TileComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& io_dependent)
{}

void TileComponent::Activate()
{
	CollectablesNotificationBus::Handler::BusConnect();

	AZ::EntityBus::MultiHandler::BusConnect(m_selectionEntityId);
	if(m_isClaimed)
	{
		AZ::EntityBus::MultiHandler::BusConnect(m_meshEntityId);
	}

	GameNotificationBus::Handler::BusConnect();

	const AZ::EntityId thisEntityId = GetEntityId();

	TileRequestBus::Handler::BusConnect(thisEntityId);
}

void TileComponent::Deactivate()
{
	TileRequestBus::Handler::BusDisconnect();
	TileNotificationBus::MultiHandler::BusDisconnect();

	GameNotificationBus::Handler::BusDisconnect();
	AZ::TickBus::Handler::BusDisconnect();
	AZ::EntityBus::MultiHandler::BusDisconnect();

	CollectablesNotificationBus::Handler::BusConnect();
}
	
void TileComponent::OnEntityActivated(const AZ::EntityId& i_entityId)
{
	if(i_entityId == m_meshEntityId)
	{
		const AZ::Quaternion rotation = AZ::Quaternion::CreateRotationX(AZ::Constants::Pi);

		EBUS_EVENT_ID(m_meshEntityId, AZ::TransformBus, SetLocalRotationQuaternion, rotation);

		if(!m_isLocked)
		{
			AZ::TickBus::Handler::BusConnect();
		}
	}
	else if(i_entityId == m_selectionEntityId)
	{
		SetSelected(false);
	}
}

void TileComponent::OnGamePaused()
{
	AZ::TickBus::Handler::BusDisconnect();
}

void TileComponent::OnGameResumed()
{
	if(m_animation != Animation::NONE || m_energy > 0.f)
	{
		AZ::TickBus::Handler::BusConnect();
	}
}

void TileComponent::OnGameEnded()
{
	AZ::TickBus::Handler::BusDisconnect();
}

void TileComponent::OnTick(float i_deltaTime, [[maybe_unused]] AZ::ScriptTimePoint i_time)
{
	if(m_noDecayTimer > 0.f)
	{
		m_noDecayTimer -= i_deltaTime;
	}

	if(m_isRecharging)
	{
		m_isRecharging = false;
	}
	else if(m_noDecayTimer < 0.f)
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
	const float decayMultiplier = 1.f - static_cast<float>(m_nClaimedNeighbors) / static_cast<float>(MAX_NEIGHBORS);

	const float lostEnergy = decayMultiplier * m_decaySpeed * i_deltaTime;
	SubtractEnergy(lostEnergy);
}

void TileComponent::SubtractEnergy(float i_amount)
{
	AddEnergy(-i_amount);
}

void TileComponent::AddEnergy(float i_amount)
{
	if(m_isLocked)
	{
		return;
	}

	m_energy = AZStd::clamp(m_energy + i_amount, 0.f, m_maxEnergy);

	const bool isAdded = (i_amount > 0.f);
	if(isAdded)
	{
		m_isRecharging = true;
		if(!m_isClaimed && m_energy > m_toggleEnergyThreshold)
		{
			Toggle();
		}
		else if(m_isClaimed && m_animation == Animation::SHAKE && m_energy > m_alertEnergyThreshold)
		{
			StopShakeAnimation();
		}
	}
	else
	{
		if(m_isClaimed)
		{
			if(m_energy < m_toggleEnergyThreshold)
			{
				Toggle();
			}
			else if(m_energy < m_alertEnergyThreshold)
			{
				Alert();
			}
		}
	}

	EBUS_EVENT(TilesNotificationBus, OnTileEnergyChanged, GetEntityId(), m_energy / m_maxEnergy);
}

void TileComponent::Alert()
{
	if(m_animation != Animation::NONE)
	{
		return;
	}

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
		m_animationParameter = 1.f - m_animationParameter;	
	}
	else
	{
		StopAnimation();

		m_animation = Animation::FLIP;
		m_animationParameter = 0.f;
	}

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

	const bool isEnd = (m_animationParameter > 1.f);
	if(isEnd)
	{
		m_animationParameter = 1.f;
		m_animation = Animation::NONE;
	}

	const AZ::Quaternion rotation = m_startRotation.Slerp(m_endRotation, m_animationParameter);
	
	EBUS_EVENT_ID(m_meshEntityId, AZ::TransformBus, SetLocalRotationQuaternion, rotation);

	if(isEnd)
	{
		if(m_isClaimed)
		{
			EBUS_EVENT_ID(m_id, TileNotificationBus, OnTileClaimed);
			EBUS_EVENT(TilesNotificationBus, OnTileClaimed, GetEntityId());
		}
		else
		{
			EBUS_EVENT_ID(m_id, TileNotificationBus, OnTileLost);
			EBUS_EVENT(TilesNotificationBus, OnTileLost, GetEntityId());
		}
	}
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

void TileComponent::RegisterNeighbor(TileId i_tileId)
{
	if(TileNotificationBus::MultiHandler::BusIsConnectedId(i_tileId))
	{
		return;
	}

	TileNotificationBus::MultiHandler::BusConnect(i_tileId);
}

TileId TileComponent::GetTileId() const
{
	return m_id;
}

bool TileComponent::IsClaimed() const
{
	return m_isClaimed;
}

bool TileComponent::IsLandingArea() const
{
	return m_isLandingArea;
}

void TileComponent::SetSelected(bool i_enabled)
{
	bool isSelected { false };
	EBUS_EVENT_ID_RESULT(isSelected, m_selectionEntityId, AZ::Render::MeshComponentRequestBus, GetVisibility);

	if(isSelected == i_enabled)
	{
		return;
	}
	
	EBUS_EVENT_ID(m_selectionEntityId, AZ::Render::MeshComponentRequestBus, SetVisibility, i_enabled);

	if(i_enabled)
	{
		EBUS_EVENT(TilesNotificationBus, OnTileSelected, GetEntityId());
	}
	else
	{
		EBUS_EVENT(TilesNotificationBus, OnTileDeselected, GetEntityId());
	}
}

void TileComponent::OnTileClaimed()
{
	if(m_nClaimedNeighbors >= MAX_NEIGHBORS)
	{
		return;
	}

	++m_nClaimedNeighbors;
}

void TileComponent::OnTileLost()
{
	if(m_nClaimedNeighbors == 0)
	{
		return;
	}

	--m_nClaimedNeighbors;
}

void TileComponent::OnStopDecayCollected(float i_duration)
{
	if(!m_isClaimed)
	{
		return;
	}

	m_noDecayTimer = i_duration;
}

void TileComponent::OnTileEnergyCollected(float i_energy)
{
	if(!m_isClaimed)
	{
		return;
	}

	AddEnergy(i_energy);
}
