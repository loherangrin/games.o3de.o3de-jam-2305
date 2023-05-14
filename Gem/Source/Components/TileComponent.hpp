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
#include <AzCore/Component/EntityBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Quaternion.h>

#include "../EBuses/GameBus.hpp"
#include "../EBuses/CollectableBus.hpp"
#include "../EBuses/TileBus.hpp"


namespace Loherangrin::Games::O3DEJam2305
{
	class TilesPoolComponent;

	class TileComponent
		: public AZ::Component
		, protected AZ::EntityBus::MultiHandler
		, protected AZ::TickBus::Handler
		, protected CollectablesNotificationBus::Handler
		, protected GameNotificationBus::Handler
		, protected TileRequestBus::Handler
		, protected TileNotificationBus::MultiHandler
	{
	public:
		AZ_COMPONENT(TileComponent, "{D59C9EF7-BB5E-476F-B692-BFBB94FE3A06}");
		static void Reflect(AZ::ReflectContext* io_context);

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& io_provided);
		static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& io_incompatible);
		static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& io_required);
		static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& io_dependent);

	protected:
		// AZ::Component
		void Activate() override;
		void Deactivate() override;

		// AZ::EntityBus
		void OnEntityActivated(const AZ::EntityId& i_entityId) override;

		// AZ::TickBus
		void OnTick(float i_deltaTime, AZ::ScriptTimePoint i_time) override;

		// TileRequestBus
		void AddEnergy(float i_amount) override;
		void SubtractEnergy(float i_amount) override;

		TileId GetTileId() const override;
		bool IsClaimed() const override;
		bool IsLandingArea() const override;

		void SetSelected(bool i_enabled) override;

		// TileNotificationBus
		void OnTileClaimed() override;
		void OnTileLost() override;

		// CollectablesNotificationBus
		void OnStopDecayCollected(float i_duration) override;
		void OnTileEnergyCollected(float i_energy) override;

		// GameNotificationBus
		void OnGamePaused() override;
		void OnGameResumed() override;
		void OnGameEnded() override;

	private:
		enum class Animation : AZ::u8
		{
			NONE = 0,
			FLIP,
			SHAKE
		};

		void RegisterNeighbor(TileId i_tileId);

		void Decay(float i_deltaTime);

		void Alert();
		void Toggle();

		void PlayAnimation(float i_deltaTime);
		void PlayFlipAnimation(float i_deltaTime);
		void PlayShakeAnimation(float i_deltaTime);

		void StopAnimation();
		void StopShakeAnimation();

		float m_maxEnergy { 10.f };
		float m_energy { 0.f };

		float m_toggleEnergyThreshold { 2.5f };
		float m_alertEnergyThreshold { 3.5f };

		float m_decaySpeed { 1.f };
		float m_noDecayTimer { -1.f };

		bool m_isRecharging { false };
		bool m_isClaimed { false };
		bool m_isLocked { false };
		bool m_isLandingArea { false };

		AZ::u8 m_nClaimedNeighbors { 0 };

		float m_flipSpeed { 2.f };

		float m_maxShakeHeight { 1.f };
		float m_shakeSpeed { 2.f };

		Animation m_animation { Animation::NONE };
		float m_animationParameter { 0.f };

		float m_startHeight { 0.f };
		float m_endHeight { 0.f };

		AZ::Quaternion m_startRotation { AZ::Quaternion::CreateIdentity() };
		AZ::Quaternion m_endRotation { AZ::Quaternion::CreateIdentity() };

		AZ::EntityId m_meshEntityId {};
		AZ::EntityId m_selectionEntityId {};

		static constexpr AZ::u8 MAX_NEIGHBORS = 8;

		friend TilesPoolComponent;
	};

} // Loherangrin::Games::O3DEJam2305
