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
#include <AzCore/Math/Random.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/std/containers/set.h>
#include <AzCore/std/containers/vector.h>

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
		using CellIndex = AZStd::pair<AZ::u16, AZ::u16>;
		using CellIndexesList = AZStd::set<CellIndex>;

		void CreateAllBoundaries();
		void CreateBoundary(const AZ::Vector3& i_translation);

		CellIndexesList CreateAllObstacles();
		void CreateObstacle(AZ::u16 i_row, AZ::u16 i_column);

		void CreateAllTiles(const CellIndexesList& i_ignoredCellIndexes = {});
		void CreateTile(AZ::u16 i_row, AZ::u16 i_column, bool i_isStart);

		void DestroyAllBoundaries();
		void DestroyAllObstacles();
		void DestroyAllTiles();

		TileId CalculateTileId(AZ::u16 i_row, AZ::u16 i_column) const;
		AZStd::vector<TileId> CalculateNeighbors(AZ::u16 i_row, AZ::u16 i_column) const;

		AZ::Vector3 CalculateCellPosition(AZ::u16 i_row, AZ::u16 i_column, const AZ::Vector2& i_cellSize) const;
		static void DestroyAllEntities(AZStd::vector<AzFramework::EntitySpawnTicket>& io_spawnTickets);

		AZ::u16 m_gridLength { 10 };
		AZ::u16 m_maxObstacles { 5 };

		AZ::Vector2 m_boundaryCellSize { AZ::Vector2::CreateOne() };
		AZ::Vector2 m_obstacleCellSize { AZ::Vector2::CreateOne() };
		AZ::Vector2 m_tileCellSize { AZ::Vector2::CreateOne() };

		AZStd::vector<AZ::Data::Asset<AzFramework::Spawnable>> m_boundaryPrefabs {};
    	AZStd::vector<AzFramework::EntitySpawnTicket> m_boundarySpawnTickets {};

		AZStd::vector<AZ::Data::Asset<AzFramework::Spawnable>> m_obstaclePrefabs {};
    	AZStd::vector<AzFramework::EntitySpawnTicket> m_obstacleSpawnTickets {};

		AZ::Data::Asset<AzFramework::Spawnable> m_landingTilePrefab {};
		AZStd::vector<AZ::Data::Asset<AzFramework::Spawnable>> m_tilePrefabs {};
    	AZStd::vector<AzFramework::EntitySpawnTicket> m_tileSpawnTickets {};


		AZ::u64 m_randomSeed { 1234 };
		AZ::SimpleLcgRandom m_randomGenerator {};
	};

} // Loherangrin::Games::O3DEJam2305
