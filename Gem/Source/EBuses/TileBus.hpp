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
	using TileId = AZStd::size_t;

	class TileRequests
	{
	public:
		AZ_RTTI(TileRequests, "{8759B2BE-9228-4D42-80DB-3846877C3C56}");
		virtual ~TileRequests() = default;

		virtual void AddEnergy(float i_amount) = 0;

		virtual TileId GetTileId() const = 0;
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

	// ---

    class TileNotifications
    {
    public:
        AZ_RTTI(TileNotifications, "{0A9D666C-D962-4DE2-90C5-9B9374C8331C}");
        virtual ~TileNotifications() = default;

		virtual void OnTileClaimed(){}
		virtual void OnTileLost(){}
    };
    
    class TileNotificationBusTraits
        : public AZ::EBusTraits
    {
    public:
		// EBusTraits
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = TileId;
    };

    using TileNotificationBus = AZ::EBus<TileNotifications, TileNotificationBusTraits>;

} // Loherangrin::Games::O3DEJam2305
