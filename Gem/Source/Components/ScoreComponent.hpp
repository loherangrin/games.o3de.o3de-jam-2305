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

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>

#include "../EBuses/CollectableBus.hpp"
#include "../EBuses/GameBus.hpp"
#include "../EBuses/ScoreBus.hpp"
#include "../EBuses/TileBus.hpp"


namespace Loherangrin::Games::O3DEJam2305
{
	class ScoreComponent
		: public AZ::Component
		, protected AZ::TickBus::Handler
		, protected CollectablesNotificationBus::Handler
		, protected GameNotificationBus::Handler
		, protected TilesNotificationBus::Handler
	{
	public:
		AZ_COMPONENT(ScoreComponent, "{2F874DB1-040E-415A-837E-AB24C8F299D5}");
		static void Reflect(AZ::ReflectContext* io_context);

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& io_provided);
		static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& io_incompatible);
		static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& io_required);
		static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& io_dependent);

	protected:
		// AZ::Component
		void Activate() override;
		void Deactivate() override;

		// AZ::TickBus
		void OnTick(float i_deltaTime, AZ::ScriptTimePoint i_time) override;

		// CollectablesNotificationBus
		void OnPointsCollected(Points i_points) override;

		// GameNotificationBus
		void OnGameLoading() override;
		void OnGamePaused() override;
		void OnGameResumed() override;
		void OnGameEnded() override;

		// TilesNotificationBus
		void OnTileClaimed(const AZ::EntityId& i_tileEntityId) override;
		void OnTileLost(const AZ::EntityId& i_tileEntityId) override;

	private:
		using Points = CollectablesNotifications::Points;
		using TotalPoints = ScoreNotifications::TotalPoints;

		Points CalculateAllTilePoints() const;
		void NotifyClaimedTiles() const;

		TotalPoints m_totalPoints { 0 };

		Points m_claimedTilePoints { 1 };
		TileCount m_nClaimedTiles { 0 };

		float m_tileTimerPeriod { 5.f };
		float m_timer { -1.f };
	};

} // Loherangrin::Games::O3DEJam2305
