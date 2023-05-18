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

#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Serialization/SerializeContext.h>

#include "TileComponent.hpp"
#include "TilesPoolComponent.hpp"

using Loherangrin::Games::O3DEJam2305::TileId;
using Loherangrin::Games::O3DEJam2305::TilesPoolComponent;


void TilesPoolComponent::Reflect(AZ::ReflectContext* io_context)
{
	if(auto serializeContext = azrtti_cast<AZ::SerializeContext*>(io_context))
	{
		serializeContext->Class<TilesPoolComponent, AZ::Component>()
			->Version(0)
			->Field("Grid", &TilesPoolComponent::m_maxGridLength)
			->Field("Seed", &TilesPoolComponent::m_randomSeed)
			->Field("TileSize", &TilesPoolComponent::m_tileCellSize)
			->Field("TilesLanding", &TilesPoolComponent::m_landingTilePrefab)
			->Field("Tiles", &TilesPoolComponent::m_tilePrefabs)
			->Field("BoundarySize", &TilesPoolComponent::m_boundaryCellSize)
			->Field("Boundaries", &TilesPoolComponent::m_boundaryPrefabs)
			->Field("ObstacleCount", &TilesPoolComponent::m_maxObstacles)
			->Field("ObstacleSize", &TilesPoolComponent::m_obstacleCellSize)
			->Field("Obstacles", &TilesPoolComponent::m_obstaclePrefabs)
		;

		if(AZ::EditContext* editContext = serializeContext->GetEditContext())
		{
			editContext->Class<TilesPoolComponent>("Tiles Pool", "Tiles Pool")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

				->DataElement(AZ::Edit::UIHandlers::Default, &TilesPoolComponent::m_maxGridLength, "Grid", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "Random")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &TilesPoolComponent::m_randomSeed, "Seed", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "Tiles")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &TilesPoolComponent::m_tileCellSize, "Cell", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &TilesPoolComponent::m_landingTilePrefab, "Prefab - Landing", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &TilesPoolComponent::m_tilePrefabs, "Prefabs - Others", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "Boundaries")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &TilesPoolComponent::m_boundaryCellSize, "Cell", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &TilesPoolComponent::m_boundaryPrefabs, "Prefabs", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "Obstacles")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &TilesPoolComponent::m_maxObstacles, "Max", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &TilesPoolComponent::m_obstacleCellSize, "Cell", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &TilesPoolComponent::m_obstaclePrefabs, "Prefabs", "")
			;
		}
	}
}

void TilesPoolComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& io_provided)
{
	io_provided.push_back(AZ_CRC_CE("TilesPoolService"));
}

void TilesPoolComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& io_incompatible)
{
	io_incompatible.push_back(AZ_CRC_CE("TilesPoolService"));
}

void TilesPoolComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& io_required)
{}

void TilesPoolComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& io_dependent)
{}

void TilesPoolComponent::Init()
{
	for(auto& prefab : m_boundaryPrefabs)
	{
		m_boundarySpawnTickets.emplace_back(AzFramework::EntitySpawnTicket { prefab });
	}

	for(auto& prefab : m_obstaclePrefabs)
	{
		m_obstacleSpawnTickets.emplace_back(AzFramework::EntitySpawnTicket { prefab });
	}
	
	m_tileSpawnTickets.emplace_back(AzFramework::EntitySpawnTicket { m_landingTilePrefab });
	for(auto& prefab : m_tilePrefabs)
	{
		m_tileSpawnTickets.emplace_back(AzFramework::EntitySpawnTicket { prefab });
	}

	m_randomGenerator.SetSeed(m_randomSeed);
}

void TilesPoolComponent::Activate()
{
	m_gridLength = GRID_LENGTHS_FIRST_ACTIVATION;

	CreateAllBoundaries();
	CreateAllTiles(true);

	GameNotificationBus::Handler::BusConnect();
}

void TilesPoolComponent::Deactivate()
{
	GameNotificationBus::Handler::BusDisconnect();

	DestroyAllObstacles();
	DestroyAllTiles();
	DestroyAllBoundaries();
}

void TilesPoolComponent::OnGameLoading()
{
	DestroyAllObstacles();
	DestroyAllTiles();

	if(m_gridLength != m_maxGridLength)
	{
		DestroyAllBoundaries();

		m_gridLength = m_maxGridLength;
		CreateAllBoundaries();
	}

	const CellIndexesList obstacleIndexes = CreateAllObstacles();
	CreateAllTiles(false, obstacleIndexes);
}

AZ::Vector2 TilesPoolComponent::GetGridSize() const
{
	return (m_tileCellSize * m_gridLength);
}

void TilesPoolComponent::CreateAllBoundaries()
{
	const AZ::Vector2 halfGridSize = GetGridSize() / 2.f;
	const AZ::Vector2 halfBoundaryCellSize = m_boundaryCellSize / 2.f;

	AZ::Vector3 translation
	{
		-halfGridSize.GetX() - halfBoundaryCellSize.GetX(),
		halfGridSize.GetY() + halfBoundaryCellSize.GetY(),
		0.f
	};

	for(AZ::u8 i = 0; i < 4; ++i)
	{
		AZ::Vector3 offset;
		switch(i)
		{
			case 0:
			{
				offset = { 1.f, 0.f, 0.f };
			}
			break;

			case 1:
			{
				offset = { 0.f, -1.f, 0.f };
			}
			break;

			case 2:
			{
				offset = { -1.f, 0.f, 0.f };
			}
			break;

			case 3:
			{
				offset = { 0.f, 1.f, 0.f };
			}
			break;
		}

		offset *= (i % 2 == 0) ? m_boundaryCellSize.GetX() : m_boundaryCellSize.GetY();
		translation += offset;

		const float scale = (i % 2 == 0)
			? m_tileCellSize.GetX() / m_boundaryCellSize.GetX()
			: m_tileCellSize.GetY() / m_boundaryCellSize.GetY()
		;

		const auto length = static_cast<AZ::u16>(static_cast<float>(m_gridLength) * scale);
		for(AZ::u16 j = 0; j < length; ++j)
		{
			CreateBoundary(translation);
			translation += offset;
		}

		CreateBoundary(translation);
	}
}

void TilesPoolComponent::CreateBoundary(const AZ::Vector3& i_translation)
{
	const AZStd::size_t boundaryType = m_randomGenerator.Getu64Random() % m_boundarySpawnTickets.size();

	AzFramework::SpawnAllEntitiesOptionalArgs spawnOptions;

	spawnOptions.m_completionCallback = [i_translation]([[maybe_unused]] AzFramework::EntitySpawnTicket::Id i_spawnTicketId, AzFramework::SpawnableConstEntityContainerView i_newEntities)
	{
		if(i_newEntities.empty())
		{
			AZ_Error("TilesPool", false, "Unable to spawn boundaries. Please check if prefabs are assigned");
			return;
		}

		const AZ::Entity* newRootEntity = *(i_newEntities.begin());
		const AZ::EntityId newRootEntityId = newRootEntity->GetId();

		EBUS_EVENT_ID(newRootEntityId, AZ::TransformBus, SetLocalTranslation, i_translation);
	};

	auto spawnableSystem = AzFramework::SpawnableEntitiesInterface::Get();
    AZ_Assert(spawnableSystem, "Unable to retrieve the main spawnable system");

	spawnableSystem->SpawnAllEntities(m_boundarySpawnTickets[boundaryType], AZStd::move(spawnOptions));
}

TilesPoolComponent::CellIndexesList TilesPoolComponent::CreateAllObstacles()
{
	const float rowScale = m_tileCellSize.GetY() / m_obstacleCellSize.GetY();
	const float columnScale = m_tileCellSize.GetX() / m_obstacleCellSize.GetX();

	const float invertedRowScale = 1.f / rowScale;
	const float invertedColumnScale = 1.f / columnScale;

	const auto nObstacleRows = static_cast<AZ::u16>(rowScale * m_gridLength);
	const auto nObstacleColumns = static_cast<AZ::u16>(columnScale * m_gridLength);	

	CellIndexesList obstacleCellIndexes {};
	for(AZ::u16 i = 0, nAttempts = 0; i < m_maxObstacles;)
	{
		const AZ::u16 obstacleRow = m_randomGenerator.Getu64Random() % nObstacleRows;
		const AZ::u16 obstacleColumn = m_randomGenerator.Getu64Random() % nObstacleColumns;

		if(obstacleRow == nObstacleRows / 2 && obstacleColumn == nObstacleColumns / 2)
		{
			++nAttempts;
			if(nAttempts > m_maxObstacles)
			{
				return obstacleCellIndexes;
			}

			continue;
		}

		CreateObstacle(obstacleRow, obstacleColumn);

		const auto tileRow = static_cast<AZ::u16>(static_cast<float>(obstacleRow) * invertedRowScale);
		const auto tileColumn = static_cast<AZ::u16>(static_cast<float>(obstacleColumn) * invertedColumnScale);

		for(AZ::u16 j = 0; j < static_cast<AZ::u16>(invertedRowScale); ++j)
		{
			for(AZ::u16 k = 0; k < static_cast<AZ::u16>(invertedColumnScale); ++k)
			{
				obstacleCellIndexes.emplace(AZStd::make_pair(tileRow + j, tileColumn + k));
			}
		}

		nAttempts = 0;
		++i;
	}

	return obstacleCellIndexes;
}

void TilesPoolComponent::CreateObstacle(AZ::u16 i_row, AZ::u16 i_column)
{
	const AZStd::size_t obstacleType = m_randomGenerator.Getu64Random() % m_obstacleSpawnTickets.size();

	AzFramework::SpawnAllEntitiesOptionalArgs spawnOptions;

	spawnOptions.m_completionCallback = [this, i_row, i_column]([[maybe_unused]] AzFramework::EntitySpawnTicket::Id i_spawnTicketId, AzFramework::SpawnableConstEntityContainerView i_newEntities)
	{
		if(i_newEntities.empty())
		{
			AZ_Error("TilesPool", false, "Unable to spawn obstacles. Please check if prefabs are assigned");
			return;
		}

		const AZ::Vector3 obstacleTranslation = CalculateCellPosition(i_row, i_column, m_obstacleCellSize);

		const AZ::Entity* newRootEntity = *(i_newEntities.begin());
		const AZ::EntityId newRootEntityId = newRootEntity->GetId();

		EBUS_EVENT_ID(newRootEntityId, AZ::TransformBus, SetLocalTranslation, obstacleTranslation);
	};

	auto spawnableSystem = AzFramework::SpawnableEntitiesInterface::Get();
    AZ_Assert(spawnableSystem, "Unable to retrieve the main spawnable system");

	spawnableSystem->SpawnAllEntities(m_obstacleSpawnTickets[obstacleType], AZStd::move(spawnOptions));
}

void TilesPoolComponent::CreateAllTiles(bool i_forceEmptyTiles, const CellIndexesList& i_ignoredCellIndexes)
{
	const AZ::u16 halfLength = m_gridLength / 2;

	for(AZ::u16 i = 0; i < m_gridLength; ++i)
	{
		const bool isCenterRow = (i == halfLength);
		
		for(AZ::u16 j = 0; j < m_gridLength; ++j)
		{
			if(i_ignoredCellIndexes.contains(AZStd::make_pair(i, j)))
			{
				continue;
			}

			const bool isCenterColumn = (j == halfLength);

			CreateTile(i, j, isCenterRow && isCenterColumn, i_forceEmptyTiles);
		}
	}

	EBUS_EVENT(TilesNotificationBus, OnAllTilesCreated);
}

void TilesPoolComponent::CreateTile(AZ::u16 i_row, AZ::u16 i_column, bool i_isStart, bool i_forceEmpty)
{
	const AZStd::size_t tileType = (i_isStart)
		? TILE_TYPES_LANDING_AREA
		: ((i_forceEmpty)
			? TILE_TYPES_EMPTY
			: m_randomGenerator.Getu64Random() % m_tileSpawnTickets.size()
		)
	;

	AzFramework::SpawnAllEntitiesOptionalArgs spawnOptions;

	spawnOptions.m_preInsertionCallback = [this, i_row, i_column, i_isStart, tileType]([[maybe_unused]] AzFramework::EntitySpawnTicket::Id i_spawnTicketId, AzFramework::SpawnableEntityContainerView i_newEntities)
	{
		if(i_newEntities.empty())
		{
			AZ_Error("TilesPool", false, "Unable to spawn tiles. Please check if prefabs are assigned");
			return;
		}

		AZ::Entity* newEntity = *(i_newEntities.begin() + 1);
		auto newTile = newEntity->FindComponent<TileComponent>();
		newTile->m_id = CalculateTileId(i_row, i_column);
		newTile->m_isLandingArea = (tileType == TILE_TYPES_LANDING_AREA);

		if(i_isStart)
		{
			newTile->m_isClaimed = true;
			newTile->m_isLocked = true;

			return;
		}

		const AZStd::vector<TileId> neighborIds = CalculateNeighbors(i_row, i_column);
		for(const TileId neighborId : neighborIds)
		{
			newTile->RegisterNeighbor(neighborId);
		}
	};

	spawnOptions.m_completionCallback = [this, i_row, i_column]([[maybe_unused]] AzFramework::EntitySpawnTicket::Id i_spawnTicketId, AzFramework::SpawnableConstEntityContainerView i_newEntities)
	{
		if(i_newEntities.empty())
		{
			AZ_Error("TilesPool", false, "Unable to spawn tiles. Please check if a prefab is assigned");
			return;
		}

		const AZ::Vector3 tileTranslation = CalculateCellPosition(i_row, i_column, m_tileCellSize);

		const AZ::Entity* newRootEntity = *(i_newEntities.begin());
		const AZ::EntityId newRootEntityId = newRootEntity->GetId();

		EBUS_EVENT_ID(newRootEntityId, AZ::TransformBus, SetLocalTranslation, tileTranslation);
	};

	auto spawnableSystem = AzFramework::SpawnableEntitiesInterface::Get();
    AZ_Assert(spawnableSystem, "Unable to retrieve the main spawnable system");

	spawnableSystem->SpawnAllEntities(m_tileSpawnTickets[tileType], AZStd::move(spawnOptions));
}

void TilesPoolComponent::DestroyAllBoundaries()
{
	DestroyAllEntities(m_boundarySpawnTickets);
}

void TilesPoolComponent::DestroyAllObstacles()
{
	DestroyAllEntities(m_obstacleSpawnTickets);
}

void TilesPoolComponent::DestroyAllTiles()
{
	DestroyAllEntities(m_tileSpawnTickets);
}

void TilesPoolComponent::DestroyAllEntities(AZStd::vector<AzFramework::EntitySpawnTicket>& io_spawnTickets)
{
	auto spawnableSystem = AzFramework::SpawnableEntitiesInterface::Get();
    AZ_Assert(spawnableSystem, "Unable to retrieve the main spawnable system");

	for(auto& spawnTicket : io_spawnTickets)
	{
		spawnableSystem->DespawnAllEntities(spawnTicket);
	}
}

AZ::Vector3 TilesPoolComponent::CalculateCellPosition(AZ::u16 i_row, AZ::u16 i_column, const AZ::Vector2& i_cellSize) const
{
	const AZ::Vector2 gridOffset = (GetGridSize() - i_cellSize) / 2.f;

	return
	{
		i_column * i_cellSize.GetX() - gridOffset.GetX(),
		i_row * i_cellSize.GetY() - gridOffset.GetY(),
		0.f
	};
}

TileId TilesPoolComponent::CalculateTileId(AZ::u16 i_row, AZ::u16 i_column) const
{
	return (i_row * m_gridLength) + i_column;
}

AZStd::vector<TileId> TilesPoolComponent::CalculateNeighbors(AZ::u16 i_row, AZ::u16 i_column) const
{
	AZStd::vector<TileId> neighborIds;

	if(i_row > 0)
	{
		const AZ::u16 previousRow = i_row - 1;

		if(i_column > 0)
		{
			neighborIds.emplace_back(CalculateTileId(previousRow, i_column - 1));
		}

		neighborIds.emplace_back(CalculateTileId(previousRow, i_column));

		if(i_column < m_gridLength - 1)
		{
			neighborIds.emplace_back(CalculateTileId(previousRow, i_column + 1));
		}
	}

	if(i_column > 0)
	{
		neighborIds.emplace_back(CalculateTileId(i_row, i_column - 1));
	}

	if(i_column < m_gridLength - 1)
	{
		neighborIds.emplace_back(CalculateTileId(i_row, i_column + 1));
	}

	if(i_row < m_gridLength - 1)
	{
		const AZ::u16 nextRow = i_row + 1;

		if(i_column > 0)
		{
			neighborIds.emplace_back(CalculateTileId(nextRow, i_column - 1));
		}

		neighborIds.emplace_back(CalculateTileId(nextRow, i_column));

		if(i_column < m_gridLength - 1)
		{
			neighborIds.emplace_back(CalculateTileId(nextRow, i_column + 1));
		}
	}

	return neighborIds;
}
