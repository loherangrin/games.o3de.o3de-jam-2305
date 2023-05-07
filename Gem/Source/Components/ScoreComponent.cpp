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

#include "ScoreComponent.hpp"

using Loherangrin::Games::O3DEJam2305::ScoreComponent;


void ScoreComponent::Reflect(AZ::ReflectContext* io_context)
{
	if(auto serializeContext = azrtti_cast<AZ::SerializeContext*>(io_context))
	{
		serializeContext->Class<ScoreComponent, AZ::Component>()
			->Version(0)
			->Field("TilePoints", &ScoreComponent::m_claimedTilePoints)
			->Field("TileTimer", &ScoreComponent::m_tileTimerPeriod)
		;

		if(AZ::EditContext* editContext = serializeContext->GetEditContext())
		{
			editContext->Class<ScoreComponent>("Score", "Score")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

				->ClassElement(AZ::Edit::ClassElements::Group, "Tiles")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &ScoreComponent::m_claimedTilePoints, "Points", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &ScoreComponent::m_tileTimerPeriod, "Timer", "")
			;
		}
	}
}

void ScoreComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& io_provided)
{
	io_provided.push_back(AZ_CRC_CE("ScoreService"));
}

void ScoreComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& io_incompatible)
{
	io_incompatible.push_back(AZ_CRC_CE("ScoreService"));
}

void ScoreComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& io_required)
{}

void ScoreComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& io_dependent)
{}

void ScoreComponent::Activate()
{
	CollectablesNotificationBus::Handler::BusConnect();
	TilesNotificationBus::Handler::BusConnect();
}

void ScoreComponent::Deactivate()
{
	TilesNotificationBus::Handler::BusConnect();
	CollectablesNotificationBus::Handler::BusDisconnect();
}

void ScoreComponent::OnTick(float i_deltaTime, [[maybe_unused]] AZ::ScriptTimePoint i_time)
{
	m_timer -= i_deltaTime;
	if(m_timer > 0.f)
	{
		return;
	}

	while(m_timer < 0.f)
	{
		m_timer += m_tileTimerPeriod;
	}

	m_totalPoints += CalculateAllTilePoints();
}

void ScoreComponent::OnPointsCollected(Points i_points)
{
	m_totalPoints += i_points;
}

void ScoreComponent::OnTileClaimed([[maybe_unused]] const AZ::EntityId& i_tileEntityId)
{
	++m_nClaimedTiles;

	if(!AZ::TickBus::Handler::BusIsConnected())
	{
		AZ::TickBus::Handler::BusConnect();
		m_timer = m_tileTimerPeriod;
	}
}

void ScoreComponent::OnTileLost([[maybe_unused]] const AZ::EntityId& i_tileEntityId)
{
	--m_nClaimedTiles;

	if(m_nClaimedTiles == 0)
	{
		AZ::TickBus::Handler::BusDisconnect();
	}
}

ScoreComponent::Points ScoreComponent::CalculateAllTilePoints() const
{
	return (m_nClaimedTiles * m_claimedTilePoints);
}
