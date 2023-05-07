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
	class SpaceshipRequests
	{
	public:
		AZ_RTTI(SpaceshipRequests, "{4AF25A16-1664-41F6-8362-F5FA62372D67}");
		virtual ~SpaceshipRequests() = default;

        virtual void SubtractEnergy(float i_amount) = 0;
	};
	
	class SpaceshipRequestBusTraits
		: public AZ::EBusTraits
	{
	public:
		// EBusTraits
		static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
	};

	using SpaceshipRequestBus = AZ::EBus<SpaceshipRequests, SpaceshipRequestBusTraits>;

    // ---

    class SpaceshipNotifications
    {
    public:
        AZ_RTTI(SpaceshipNotifications, "{089C3DD7-1BA9-444D-9CC0-7156848CFDA0}");
        virtual ~SpaceshipNotifications() = default;

		virtual void OnEnergySavingModeActivated(){}
        virtual void OnEnergySavingModeDeactivated(){}

		virtual void OnRechargingStarted(){}
		virtual void OnRechargingEnded(){}
    };
    
    class SpaceshipNotificationBusTraits
        : public AZ::EBusTraits
    {
    public:
		// EBusTraits
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
    };

    using SpaceshipNotificationBus = AZ::EBus<SpaceshipNotifications, SpaceshipNotificationBusTraits>;

} // Loherangrin::Games::O3DEJam2305
