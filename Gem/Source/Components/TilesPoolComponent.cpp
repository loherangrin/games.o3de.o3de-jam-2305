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
			->Field("Tile", &TilesPoolComponent::m_tilePrefab)
			->Field("Grid", &TilesPoolComponent::m_gridLength)
			->Field("Cell", &TilesPoolComponent::m_cellSize)
		;

		if(AZ::EditContext* editContext = serializeContext->GetEditContext())
		{
			editContext->Class<TilesPoolComponent>("Tiles Pool", "Tiles Pool")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

				->DataElement(AZ::Edit::UIHandlers::Default, &TilesPoolComponent::m_tilePrefab, "Prefab", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "Grid")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &TilesPoolComponent::m_gridLength, "Length", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &TilesPoolComponent::m_cellSize, "Cell", "")
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

void TilesPoolComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& io_required)
{}

void TilesPoolComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& io_dependent)
{}

void TilesPoolComponent::Init()
{
	m_tileSpawnTicket = AzFramework::EntitySpawnTicket { m_tilePrefab };
}

void TilesPoolComponent::Activate()
{
	// TODO - Debug
	CreateAllTiles();
}

void TilesPoolComponent::Deactivate()
{
	DestroyAllTiles();
}

AZ::Vector2 TilesPoolComponent::GetGridSize() const
{
	return AZ::Vector2
	{
		m_gridLength * m_cellSize.GetX(),
		m_gridLength * m_cellSize.GetY()
	};
}

void TilesPoolComponent::CreateAllTiles()
{
	for(AZ::u16 i = 0; i < m_gridLength; ++i)
	{
		for(AZ::u16 j = 0; j < m_gridLength; ++j)
		{
			CreateTile(i, j);
		}
	}
}

void TilesPoolComponent::CreateTile(AZ::u16 i_row, AZ::u16 i_column)
{
	AzFramework::SpawnAllEntitiesOptionalArgs spawnOptions;

	spawnOptions.m_preInsertionCallback = [this, i_row, i_column]([[maybe_unused]] AzFramework::EntitySpawnTicket::Id i_spawnTicketId, AzFramework::SpawnableEntityContainerView i_newEntities)
	{
		if(i_newEntities.empty())
		{
			AZ_Error("TilesPool", false, "Unable to spawn tiles. Please check if a prefab is assigned");
			return;
		}

		AZ::Entity* newEntity = *(i_newEntities.begin() + 1);
		auto newTile = newEntity->FindComponent<TileComponent>();
		newTile->m_id = CalculateTileId(i_row, i_column);

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

		const AZ::Vector2 halfGridSize = (m_cellSize * m_gridLength) / 2.f;
		const AZ::Vector3 tileTranslation
		{
			i_row * m_cellSize.GetX() - halfGridSize.GetX(),
			i_column * m_cellSize.GetY() - halfGridSize.GetY(),
			0.f
		};

		const AZ::Entity* newRootEntity = *(i_newEntities.begin());
		const AZ::EntityId newRootEntityId = newRootEntity->GetId();

		EBUS_EVENT_ID(newRootEntityId, AZ::TransformBus, SetLocalTranslation, tileTranslation);
	};

	auto spawnableSystem = AzFramework::SpawnableEntitiesInterface::Get();
    AZ_Assert(spawnableSystem, "Unable to retrieve the main spawnable system");

	spawnableSystem->SpawnAllEntities(m_tileSpawnTicket, AZStd::move(spawnOptions));
}

void TilesPoolComponent::DestroyAllTiles()
{
	AzFramework::DespawnAllEntitiesOptionalArgs despawnOptions;

	auto spawnableSystem = AzFramework::SpawnableEntitiesInterface::Get();
    AZ_Assert(spawnableSystem, "Unable to retrieve the main spawnable system");

	spawnableSystem->DespawnAllEntities(m_tileSpawnTicket, AZStd::move(despawnOptions));
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
