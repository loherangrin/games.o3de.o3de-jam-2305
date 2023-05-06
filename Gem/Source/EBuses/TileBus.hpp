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

#include <AzCore/EBus/EBus.h>


namespace Loherangrin::Games::O3DEJam2305
{
   class TileRequests
	{
	public:
		AZ_RTTI(TileRequests, "{8759B2BE-9228-4D42-80DB-3846877C3C56}");
		virtual ~TileRequests() = default;

		virtual void AddEnergy(float i_amount) = 0;
	};
	
	class TileRequestBusTraits
		: public AZ::EBusTraits
	{
	public:
		// EBusTraits
		static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
		using BusIdType = AZ::EntityId;
	};

	using TileRequestBus = AZ::EBus<TileRequests, TileRequestBusTraits>;

} // Loherangrin::Games::O3DEJam2305
