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

#include <AzFramework/Physics/Common/PhysicsSimulatedBodyEvents.h>
#include <AzFramework/Physics/RigidBodyBus.h>


namespace Loherangrin::Games::O3DEJam2305
{
	class CollectablesPoolComponent;

	class CollectableComponent
		: public AZ::Component
		, protected Physics::RigidBodyNotificationBus::Handler
	{
	public:
		AZ_COMPONENT(CollectableComponent, "{1192D238-1E11-406C-B4D0-88A60BA5199D}");
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

		// Physics::RigidBodyNotificationBus
		void OnPhysicsEnabled(const AZ::EntityId& i_entityId) override;

	private:
		enum class CollectableType : AZ::u8
		{
			NONE = 0,
			STOP_DECAY,
			SPACESHIP_DAMAGE,
			SPACESHIP_ENERGY,
			TILE_DAMAGE,
			TILE_ENERGY,
			SPEED_UP,
			SPEED_DOWN
		};

		CollectableType m_type { CollectableType::NONE };

		float m_amount { 0.f };
		float m_duration { 0.f };

		AzPhysics::SimulatedBodyEvents::OnTriggerEnter::Handler m_triggerEnterHandler;

		friend CollectablesPoolComponent;
	};

} // // Loherangrin::Games::O3DEJam2305
