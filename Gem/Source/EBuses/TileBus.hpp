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

#include <AzCore/EBus/EBus.h>
#include <AzCore/Math/Vector2.h>


namespace Loherangrin::Games::O3DEJam2305
{
	using TileId = AZStd::size_t;
	using TileCount = TileId;

    static constexpr TileId INVALID_TILE_ID = AZStd::numeric_limits<TileId>::max();

	class TileRequests
	{
	public:
		AZ_RTTI(TileRequests, "{8759B2BE-9228-4D42-80DB-3846877C3C56}");
		virtual ~TileRequests() = default;

		virtual void AddEnergy(float i_amount) = 0;
        virtual void SubtractEnergy(float i_amount) = 0;

		virtual TileId GetTileId() const = 0;
        virtual bool IsClaimed() const = 0;
		virtual bool IsLandingArea() const = 0;

		virtual void SetSelected(bool i_enabled) = 0;
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

    class TilesRequests
	{
	public:
		AZ_RTTI(TilesRequests, "{363FA7F7-61EE-456B-AF34-25A87AC51CC4}");
		virtual ~TilesRequests() = default;

		virtual AZ::Vector2 GetGridSize() const = 0;
	};
	
	class TilesRequestBusTraits
		: public AZ::EBusTraits
	{
	public:
		// EBusTraits
		static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
	};

	using TilesRequestBus = AZ::EBus<TilesRequests, TilesRequestBusTraits>;

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

	// ---

    class TilesNotifications
    {
    public:
        AZ_RTTI(TilesNotifications, "{3240463C-C419-4970-A7E5-F82FD1D490A4}");
        virtual ~TilesNotifications() = default;

		virtual void OnTileEnergyChanged(const AZ::EntityId& i_tileEntityId, float i_normalizedNewEnergy){}

        virtual void OnTileClaimed(const AZ::EntityId& i_tileEntityId){}
		virtual void OnTileLost(const AZ::EntityId& i_tileEntityId){}

		virtual void OnTileSelected(const AZ::EntityId& i_tileEntityId){}
		virtual void OnTileDeselected(const AZ::EntityId& i_tileEntityId){}
    };
    
    class TilesNotificationBusTraits
        : public AZ::EBusTraits
    {
    public:
		// EBusTraits
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
    };

    using TilesNotificationBus = AZ::EBus<TilesNotifications, TilesNotificationBusTraits>;

} // Loherangrin::Games::O3DEJam2305
