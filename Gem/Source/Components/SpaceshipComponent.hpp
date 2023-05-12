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

#include <AzFramework/Input/Events/InputChannelEventListener.h>
#include <AzFramework/Physics/Collision/CollisionGroups.h>

#include "../EBuses/CollectableBus.hpp"
#include "../EBuses/GameBus.hpp"
#include "../EBuses/SpaceshipBus.hpp"
#include "../EBuses/TileBus.hpp"


namespace Loherangrin::Games::O3DEJam2305
{
	class SpaceshipComponent
		: public AZ::Component
		, protected AZ::TickBus::Handler
		, protected AzFramework::InputChannelEventListener
		, protected CollectablesNotificationBus::Handler
		, protected GameNotificationBus::Handler
		, protected SpaceshipRequestBus::Handler
		, protected TileNotificationBus::Handler
	{
	public:
		AZ_COMPONENT(SpaceshipComponent, "{0738CB1F-FB7B-4A7D-8DED-A7A71E7D150C}");
		static void Reflect(AZ::ReflectContext* io_context);

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& io_provided);
		static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& io_incompatible);
		static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& io_required);
		static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& io_dependent);

	protected:
		// AZ::Component
		void Init() override;
		void Activate() override;
		void Deactivate() override;

		// AZ::TickBus
		void OnTick(float i_deltaTime, AZ::ScriptTimePoint i_time) override;

		// AzFramework::InputChannelEventListener
		bool OnInputChannelEventFiltered(const AzFramework::InputChannel& i_inputChannel) override;

		// SpaceshipRequestBus
		void SubtractEnergy(float i_energy) override;

		// CollectablesNotificationBus
		void OnSpaceshipEnergyCollected(float i_energy) override;
		void OnSpeedCollected(float i_multiplier, float i_duration) override;

		// GameBus
		void OnGameCreated() override;
		void OnGameLoading() override;
		void OnGameStarted() override;
		void OnGamePaused() override;
		void OnGameResumed() override;
		void OnGameEnded() override;
		void OnGameDestroyed() override;

		// TileNotificationBus
		void OnTileLost() override;

	private:
		void ApplyRotation(const AZ::Transform& i_thisTransform, float i_deltaTime) const;
		bool ApplyHorizontalTranslation(const AZ::Transform& i_thisTransform) const;
		void ApplyVerticalTranslation(float i_deltaTime);

		TileId GetTileIdIfClaimed() const;
		bool IsGrounded() const;

		void TakeOff();
		void Land(TileId i_tileId);

		void AddEnergy(float i_amount);
		bool IsLowEnergy() const;

		void ConsumeEnergy(float i_deltaTime);
		void RechargeEnergy(float i_deltaTime);

		void ResetSpeedMultiplierOnTimerEnd(float i_deltaTime);

		void ResetInput();
		void ResetPosition();
		void ResetState();

		float m_moveDirection { 0.f };
		float m_moveSpeed { 5.f };

		float m_turnDirection { 0.f };
		float m_turnSpeed { 25.f };

		float m_liftParameter { 0.f };
		float m_liftDirection { 0.f };
		float m_liftSpeed { 2.f };
		float m_minHeight { 0.f };
		float m_maxHeight { 2.f };

		float m_maxEnergy { 10.f };
		float m_energy { 0.f };
		float m_consumptionRate { 0.5f };
		float m_rechargeRate { 1.f };

		float m_lowEnergyThreshold { 2.f };
		float m_lowEnergySpeedMultiplier { 0.5f };

		float m_speedMultiplier { 1.f };
		float m_speedTimer { -1.f };

		AZ::EntityId m_meshEntityId {};
		AzPhysics::CollisionGroup m_tileCollisionGroup {};

		static constexpr const char* COLLISION_GROUPS_ALL_TILES = "AllTiles_Query";
	};

} // Loherangrin::Games::O3DEJam2305
