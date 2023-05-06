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

#include <AzFramework/Input/Events/InputChannelEventListener.h>


namespace Loherangrin::Games::O3DEJam2305
{
	class SpaceshipComponent
		: public AZ::Component
		, protected AZ::TickBus::Handler
		, protected AzFramework::InputChannelEventListener
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

	private:
		void ApplyRotation(const AZ::Transform& i_thisTransform, float i_deltaTime) const;
		void ApplyHorizontalTranslation(const AZ::Transform& i_thisTransform) const;
		void ApplyVerticalTranslation(float i_deltaTime);

		bool IsGrounded() const;

		float m_moveDirection { 0.f };
		float m_moveSpeed { 5.f };

		float m_turnDirection { 0.f };
		float m_turnSpeed { 25.f };

		float m_liftParameter { 0.f };
		float m_liftDirection { 0.f };
		float m_liftSpeed { 2.f };
		float m_minHeight { 0.f };
		float m_maxHeight { 2.f };

		AZ::EntityId m_meshEntityId {};
	};

} // Loherangrin::Games::O3DEJam2305
