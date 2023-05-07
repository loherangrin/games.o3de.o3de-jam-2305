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

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

#include "Components/BeamComponent.hpp"
#include "Components/CollectableComponent.hpp"
#include "Components/CollectablesPoolComponent.hpp"
#include "Components/SpaceshipComponent.hpp"
#include "Components/StormComponent.hpp"
#include "Components/StormsPoolComponent.hpp"
#include "Components/TileComponent.hpp"
#include "Components/TilesPoolComponent.hpp"


namespace Loherangrin::Games::O3DEJam2305
{
	class GameModule
		: public AZ::Module
	{
	public:
		AZ_RTTI(GameModule, "{393538A2-E5FF-466A-B7AF-E75907EA9DEE}", AZ::Module);
		AZ_CLASS_ALLOCATOR(GameModule, AZ::SystemAllocator, 0);

		GameModule()
			: AZ::Module()
		{
			m_descriptors.insert(m_descriptors.end(),
			{
				BeamComponent::CreateDescriptor(),
				CollectableComponent::CreateDescriptor(),
				CollectablesPoolComponent::CreateDescriptor(),
				SpaceshipComponent::CreateDescriptor(),
				StormComponent::CreateDescriptor(),
				StormsPoolComponent::CreateDescriptor(),
				TileComponent::CreateDescriptor(),
				TilesPoolComponent::CreateDescriptor()
			});
		}

		AZ::ComponentTypeList GetRequiredSystemComponents() const override
		{
			return AZ::ComponentTypeList {};
		}
	};

} // Loherangrin::Games::O3DEJam2305

AZ_DECLARE_MODULE_CLASS(Gem_Loherangrin_O3DEJam2305, Loherangrin::Games::O3DEJam2305::GameModule)
