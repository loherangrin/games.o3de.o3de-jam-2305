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
#include <AzCore/Math/Vector3.h>

#include <AzFramework/Physics/Common/PhysicsSimulatedBodyEvents.h>
#include <AzFramework/Physics/RigidBodyBus.h>


namespace Loherangrin::Games::O3DEJam2305
{
	class StormComponent
		: public AZ::Component
		, protected AZ::TickBus::Handler
		, protected Physics::RigidBodyNotificationBus::Handler
	{
	public:
		AZ_COMPONENT(StormComponent, "{9486C200-FB68-4508-9173-154832C41611}");
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

		// Physics::RigidBodyNotificationBus
		void OnPhysicsEnabled(const AZ::EntityId& i_entityId) override;

	private:
		void ApplyMovement(float i_deltaTime);
		void PlayAnimation(float i_deltaTime);

		void ApplyDamages(float i_deltaTime);

		float m_strength { 0.f };

		AZ::Vector3 m_moveDirection { AZ::Vector3::CreateZero() };
		float m_moveSpeed { 1.f };

		float m_animationSpeed { 180.f };

		float m_duration { 0.f };
		float m_timer { -1.f };

		bool m_isSpaceshipHit { false };
		AZStd::set<AZ::EntityId> m_hitTileEntityIds {};

		AZ::EntityId m_meshEntityId {};

		AzPhysics::SimulatedBodyEvents::OnTriggerEnter::Handler m_triggerEnterHandler;
		AzPhysics::SimulatedBodyEvents::OnTriggerEnter::Handler m_triggerExitHandler;
	};

} // Loherangrin::Games::O3DEJam2305
