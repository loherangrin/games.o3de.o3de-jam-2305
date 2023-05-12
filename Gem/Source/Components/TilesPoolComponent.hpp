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

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Component/Component.h>

#include <AzFramework/Spawnable/SpawnableEntitiesInterface.h>
#include <AzFramework/Spawnable/Spawnable.h>

#include "../EBuses/GameBus.hpp"
#include "../EBuses/TileBus.hpp"


namespace Loherangrin::Games::O3DEJam2305
{
	class TilesPoolComponent
		: public AZ::Component
		, protected TilesRequestBus::Handler
		, protected GameNotificationBus::Handler
	{
	public:
		AZ_COMPONENT(TilesPoolComponent, "{C2212D14-4B02-4AE2-A5C9-2485D5CBEE54}");
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

		// TilesRequestBus
		AZ::Vector2 GetGridSize() const override;

		// GameNotificationBus
		void OnGameLoading() override;

	private:
		void CreateAllTiles();
		void CreateTile(AZ::u16 i_row, AZ::u16 i_column);

		void DestroyAllTiles();

		TileId CalculateTileId(AZ::u16 i_row, AZ::u16 i_column) const;
		AZStd::vector<TileId> CalculateNeighbors(AZ::u16 i_row, AZ::u16 i_column) const;

		AZ::u16 m_gridLength { 10 };
		AZ::Vector2 m_cellSize { AZ::Vector2::CreateOne() };

		AZ::Data::Asset<AzFramework::Spawnable> m_tilePrefab {};
    	AzFramework::EntitySpawnTicket m_tileSpawnTicket {};
	};

} // Loherangrin::Games::O3DEJam2305
