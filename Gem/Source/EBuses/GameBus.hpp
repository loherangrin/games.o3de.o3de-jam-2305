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
    class GameNotifications
    {
    public:
        AZ_RTTI(GameNotifications, "{9159794B-48C9-470D-BFB5-F70575A30067}");
        virtual ~GameNotifications() = default;

		virtual void OnGameCreated(){}
		virtual void OnGameLoading(){}
		virtual void OnGameStarted(){}
		virtual void OnGamePaused(){}
		virtual void OnGameResumed(){}
        virtual void OnGameEnded(){}
		virtual void OnGameDestroyed(){}
    };
    
    class GameNotificationBusTraits
        : public AZ::EBusTraits
    {
    public:
		// EBusTraits
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
    };

    using GameNotificationBus = AZ::EBus<GameNotifications, GameNotificationBusTraits>;

} // Loherangrin::Games::O3DEJam2305
