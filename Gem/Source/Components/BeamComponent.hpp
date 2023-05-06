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

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/containers/set.h>

#include <AzFramework/Input/Events/InputChannelEventListener.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBodyEvents.h>
#include <AzFramework/Physics/RigidBodyBus.h>


namespace Loherangrin::Games::O3DEJam2305
{
	class BeamComponent
		: public AZ::Component
		, protected AZ::TickBus::Handler
		, protected AzFramework::InputChannelEventListener
		, protected Physics::RigidBodyNotificationBus::Handler
	{
	public:
		AZ_COMPONENT(BeamComponent, "{FD7684A2-7DA0-482E-A104-B9F1D99DFD35}");
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

		// Physics::RigidBodyNotificationBus
		void OnPhysicsEnabled(const AZ::EntityId& i_entityId) override;

	private:
		void ConnectTriggerHandlers();
		void DisconnectTriggerHandlers();

		void TransferEnergyToTiles(float i_deltaTime);
		void SelectTile(const AZ::EntityId& i_tileEntityId);
		void DeselectTile(const AZ::EntityId& i_tileEntityId);

		void Toggle();
		void TurnOn();
		void TurnOff();

		bool m_isEnabled { false };

		float m_maxEnergy { 10.f };
		float m_energy { 0.f };
		float m_transferSpeed { 1.f };

		AZStd::set<AZ::EntityId> m_selectedTiles {};

		AzPhysics::SimulatedBodyEvents::OnTriggerEnter::Handler m_triggerEnterHandler;
		AzPhysics::SimulatedBodyEvents::OnTriggerExit::Handler m_triggerExitHandler;
	};

} // Loherangrin::Games::O3DEJam2305
