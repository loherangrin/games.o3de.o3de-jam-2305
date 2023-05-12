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


namespace Loherangrin::Games::O3DEJam2305
{
	class CollectablesNotifications
    {
    public:
        using Points = AZ::u16;

        AZ_RTTI(CollectablesNotifications, "{580578F6-150A-4A30-8A22-C799B336B52A}");
        virtual ~CollectablesNotifications() = default;

		virtual void OnStopDecayCollected(float i_duration){}
		virtual void OnSpaceshipEnergyCollected(float i_energy){}
		virtual void OnTileEnergyCollected(float i_energy){}
        virtual void OnPointsCollected(Points i_points){}
        virtual void OnSpeedCollected(float i_multiplier, float i_duration){}
    };
    
    class CollectablesNotificationBusTraits
        : public AZ::EBusTraits
    {
    public:
		// EBusTraits
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
    };

    using CollectablesNotificationBus = AZ::EBus<CollectablesNotifications, CollectablesNotificationBusTraits>;

} // Loherangrin::Games::O3DEJam2305
