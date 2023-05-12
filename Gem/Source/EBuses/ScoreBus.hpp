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

#include "TileBus.hpp"


namespace Loherangrin::Games::O3DEJam2305
{
	class ScoreNotifications
    {
    public:
		using TotalPoints = AZ::u64;

        AZ_RTTI(ScoreNotifications, "{7796EF24-1875-4013-8A2E-E87FB75E256C}");
        virtual ~ScoreNotifications() = default;

		virtual void OnScoreChanged(TotalPoints i_newPoints){}
		virtual void OnClaimedTilesChanged(TileCount i_newClaimedTiles){}
    };
    
    class ScoreNotificationBusTraits
        : public AZ::EBusTraits
    {
    public:
		// EBusTraits
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
    };

    using ScoreNotificationBus = AZ::EBus<ScoreNotifications, ScoreNotificationBusTraits>;

} // Loherangrin::Games::O3DEJam2305
