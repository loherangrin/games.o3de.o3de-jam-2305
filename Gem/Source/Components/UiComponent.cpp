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

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Serialization/SerializeContext.h>

#include <AzFramework/API/ApplicationAPI.h>
#include <AzFramework/Components/CameraBus.h>

#include <LyShine/Bus/UiCanvasBus.h>
#include <LyShine/Bus/UiElementBus.h>
#include <LyShine/Bus/UiFaderBus.h>
#include <LyShine/Bus/UiImageBus.h>
#include <LyShine/Bus/UiInteractableBus.h>
#include <LyShine/Bus/UiTextBus.h>

#include "UiComponent.hpp"

using Loherangrin::Games::O3DEJam2305::UiComponent;


void UiComponent::Reflect(AZ::ReflectContext* io_context)
{
	if(auto serializeContext = azrtti_cast<AZ::SerializeContext*>(io_context))
	{
		serializeContext->Class<UiComponent, AZ::Component>()
			->Version(0)
			->Field("CameraMenu", &UiComponent::m_menuCamera)
			->Field("CameraGame", &UiComponent::m_gameCamera)
			->Field("Lift", &UiComponent::m_liftDuration)
			->Field("Fade", &UiComponent::m_fadeDuration)
			->Field("Collectable", &UiComponent::m_collectableNotificationDuration)
		;

		if(AZ::EditContext* editContext = serializeContext->GetEditContext())
		{
			editContext->Class<UiComponent>("UI", "UI")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

				->ClassElement(AZ::Edit::ClassElements::Group, "Camera")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &UiComponent::m_menuCamera, "Menu", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &UiComponent::m_gameCamera, "Game", "")

				->ClassElement(AZ::Edit::ClassElements::Group, "Timer")
					->Attribute(AZ::Edit::Attributes::AutoExpand, true)

					->DataElement(AZ::Edit::UIHandlers::Default, &UiComponent::m_liftDuration, "Animation - Moving", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &UiComponent::m_fadeDuration, "Animation - Fading", "")
					->DataElement(AZ::Edit::UIHandlers::Default, &UiComponent::m_collectableNotificationDuration, "Notification - Collectable", "")
			;
		}
	}
}

void UiComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& io_provided)
{
	io_provided.push_back(AZ_CRC_CE("GameUiService"));
}

void UiComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& io_incompatible)
{
	io_incompatible.push_back(AZ_CRC_CE("GameUiService"));
}

void UiComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& io_required)
{
	io_required.push_back(AZ_CRC("UiCanvasRefService", 0xb4cb5ef4));
}

void UiComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& io_dependent)
{}

void UiComponent::Activate()
{
	const AZ::EntityId thisEntityId = GetEntityId();

	AZ::EntityId canvasEntityId {};
	EBUS_EVENT_ID_RESULT(canvasEntityId, thisEntityId, UiCanvasRefBus, GetCanvas);

	const bool isCanvasActive = FindAllUiElements(canvasEntityId);
	if(!isCanvasActive)
	{
		UiCanvasAssetRefNotificationBus::Handler::BusConnect(thisEntityId);

		return;
	}

	InitializeAllUiElements();
	ShowMainMenu();
}

void UiComponent::Deactivate()
{
	AZ::TickBus::Handler::BusDisconnect();
	UiCanvasAssetRefNotificationBus::Handler::BusDisconnect();

	TilesNotificationBus::Handler::BusDisconnect();
	SpaceshipNotificationBus::Handler::BusDisconnect();
	ScoreNotificationBus::Handler::BusDisconnect();
	GameNotificationBus::Handler::BusDisconnect();
}

void UiComponent::OnCanvasLoadedIntoEntity([[maybe_unused]] AZ::EntityId i_uiCanvasEntity)
{
	UiCanvasAssetRefNotificationBus::Handler::BusDisconnect();

	const bool found = FindAllUiElements(i_uiCanvasEntity);
	if(!found)
	{
		AZ_Error("UiComponent", false, "Unable to find a valid canvas");
		return;
	}

	InitializeAllUiElements();
	ShowMainMenu();
}

bool UiComponent::FindAllUiElements(const AZ::EntityId& i_canvasId)
{
	if(!i_canvasId.IsValid())
	{
		return false;
	}
	else if(i_canvasId == m_canvasId)
	{
		return true;
	}

	m_canvasId = i_canvasId;

	m_hudEntityId = FindUiElement(UI_HUD);
	m_lowEnergyModeEntityId = FindUiElement(UI_HUD_LOW_ENERGY_MODE_TEXT);
	m_energyBarsSeparatorEntityId = FindUiElement(UI_HUD_ENERGY_BARS_SEPARATOR_IMAGE);
	m_spaceshipLowEnergyEntityId = FindUiElement(UI_HUD_SPACESHIP_LOW_ENERGY_IMAGE);
	m_spaceshipHighEnergyEntityId = FindUiElement(UI_HUD_SPACESHIP_HIGH_ENERGY_IMAGE);
	m_tileEnergyBarEntityId = FindUiElement(UI_HUD_TILE_ENERGY_BAR);
	m_tileLowEnergyEntityId = FindUiElement(UI_HUD_TILE_LOW_ENERGY_IMAGE);
	m_tileHighEnergyEntityId = FindUiElement(UI_HUD_TILE_HIGH_ENERGY_IMAGE);
	m_claimedTilesEntityId = FindUiElement(UI_HUD_CLAIMED_TILES_TEXT);
	m_scoreEntityId = FindUiElement(UI_HUD_SCORE_TEXT);
	m_positiveCollectableEntityId = FindUiElement(UI_HUD_POSITIVE_COLLECTABLE_TEXT);
	m_negativeCollectableEntityId = FindUiElement(UI_HUD_NEGATIVE_COLLECTABLE_TEXT);

	m_loadingScreenEntityId = FindUiElement(UI_LOADING);
	m_loadingTextEntityId = FindUiElement(UI_LOADING_TEXT);

	m_mainMenuEntityId = FindUiElement(UI_MAIN_MENU);
	m_startGameEntityId = FindUiElement(UI_MAIN_MENU_START_BUTTON);
	m_exitGameEntityId = FindUiElement(UI_MAIN_MENU_EXIT_BUTTON);

	m_pauseMenuEntityId = FindUiElement(UI_PAUSE_MENU);
	m_endInstructionsEntityId = FindUiElement(UI_PAUSE_MENU_END_INSTRUCTIONS_TEXT);
	m_warningInstructionsEntityId = FindUiElement(UI_PAUSE_MENU_WARNING_INSTRUCTIONS_TEXT);
	m_resumeGameEntityId = FindUiElement(UI_PAUSE_MENU_RESUME_BUTTON);
	m_endGameEntityId = FindUiElement(UI_PAUSE_MENU_END_BUTTON);
	m_cancelGameEntityId = FindUiElement(UI_PAUSE_MENU_CANCEL_BUTTON);

	m_endMenuEntityId = FindUiElement(UI_END_MENU);
	m_gameCompletedEntityId = FindUiElement(UI_END_MENU_GAME_COMPLETED);
	m_gameFailedEntityId = FindUiElement(UI_END_MENU_GAME_FAILED);
	m_finalScoreEntityId = FindUiElement(UI_END_MENU_FINAL_SCORE_TEXT);
	m_retryGameEntityId = FindUiElement(UI_END_MENU_RETRY_BUTTON);
	m_returnMainMenuEntityId = FindUiElement(UI_END_MENU_RETURN_BUTTON);

	return true;
}

AZ::EntityId UiComponent::FindUiElement(const char* i_elementName) const
{
	AZ::EntityId elementId;
	EBUS_EVENT_ID_RESULT(elementId, m_canvasId, UiCanvasBus, FindElementEntityIdByName, AZStd::string { i_elementName });

	return elementId;
}

void UiComponent::InitializeAllUiElements()
{
	ConnectOnButtonClick(m_startGameEntityId, [this]([[maybe_unused]] AZ::EntityId i_buttonEntityId, [[maybe_unused]] AZ::Vector2 i_clickPosition)
	{
		CreateGame();
	});

	ConnectOnButtonClick(m_exitGameEntityId, []([[maybe_unused]] AZ::EntityId i_buttonEntityId, [[maybe_unused]] AZ::Vector2 i_clickPosition)
	{
		EBUS_EVENT(AzFramework::ApplicationRequests::Bus, ExitMainLoop);
	});

	ConnectOnButtonClick(m_resumeGameEntityId, [this]([[maybe_unused]] AZ::EntityId i_buttonEntityId, [[maybe_unused]] AZ::Vector2 i_clickPosition)
	{
		ResumeGame();
	});

	ConnectOnButtonClick(m_endGameEntityId, [this]([[maybe_unused]] AZ::EntityId i_buttonEntityId, [[maybe_unused]] AZ::Vector2 i_clickPosition)
	{
		EndGame();
	});

	ConnectOnButtonClick(m_cancelGameEntityId, [this]([[maybe_unused]] AZ::EntityId i_buttonEntityId, [[maybe_unused]] AZ::Vector2 i_clickPosition)
	{	
		HideUiElement(m_pauseMenuEntityId);

		DestroyGame();
	});

	ConnectOnButtonClick(m_retryGameEntityId, [this]([[maybe_unused]] AZ::EntityId i_buttonEntityId, [[maybe_unused]] AZ::Vector2 i_clickPosition)
	{
		HideUiElement(m_endMenuEntityId);

		StartGame();
	});

	ConnectOnButtonClick(m_returnMainMenuEntityId, [this]([[maybe_unused]] AZ::EntityId i_buttonEntityId, [[maybe_unused]] AZ::Vector2 i_clickPosition)
	{
		HideUiElement(m_endMenuEntityId);

		DestroyGame();
	});

	HideUiElement(m_loadingTextEntityId);
	ShowUiElement(m_loadingScreenEntityId);

	HideUiElement(m_hudEntityId);
	HideUiElement(m_mainMenuEntityId);
	HideUiElement(m_pauseMenuEntityId);
	HideUiElement(m_endMenuEntityId);
}

void UiComponent::ShowMainMenu()
{
	HideUiElement(m_hudEntityId);
	HideUiElement(m_pauseMenuEntityId);

	ShowUiElement(m_loadingScreenEntityId, 0.f);
	ShowUiElement(m_mainMenuEntityId);

	HideUiElement(m_loadingScreenEntityId, FADE_SPEED);
}

void UiComponent::CreateGame()
{
	EBUS_EVENT(GameNotificationBus, OnGameCreated);

	HideUiElement(m_mainMenuEntityId);

	m_timer = m_liftDuration;
	m_animation = Animation::TAKE_OFF;

	AZ::TickBus::Handler::BusConnect();
}

void UiComponent::StartGame()
{
	ShowUiElement(m_loadingScreenEntityId, 0.f);
	ShowUiElement(m_loadingTextEntityId);

	HideUiElement(m_spaceshipLowEnergyEntityId);
	ShowUiElement(m_spaceshipHighEnergyEntityId);
	m_spaceshipEnergyEntityId = &m_spaceshipHighEnergyEntityId;

	ShowUiElement(m_tileLowEnergyEntityId);
	HideUiElement(m_tileHighEnergyEntityId);
	m_tileEnergyEntityId = &m_tileLowEnergyEntityId;

	HideUiElement(m_tileEnergyBarEntityId);
	HideUiElement(m_energyBarsSeparatorEntityId);

	HideUiElement(m_lowEnergyModeEntityId);
	HideUiElement(m_positiveCollectableEntityId);
	HideUiElement(m_negativeCollectableEntityId);

	GameNotificationBus::Handler::BusConnect();

	CollectablesNotificationBus::Handler::BusConnect();
	ScoreNotificationBus::Handler::BusConnect();
	SpaceshipNotificationBus::Handler::BusConnect();
	TilesNotificationBus::Handler::BusConnect();

	EBUS_EVENT_ID(m_gameCamera, Camera::CameraRequestBus, MakeActiveView);

	m_animation = Animation::LOADING;
	m_timer = 0.5f;

	AZ::TickBus::Handler::BusConnect();
}

void UiComponent::OnAllTilesCreated()
{
	m_timer = 0.5f;
	m_animation = Animation::AFTER_LOADING;

	AZ::TickBus::Handler::BusConnect();
}

void UiComponent::OnTick(float i_deltaTime, [[maybe_unused]] AZ::ScriptTimePoint i_time)
{
	m_timer -= i_deltaTime;
	if(m_timer > 0.f)
	{
		return;
	}

	switch(m_animation)
	{
		case Animation::LOADING:
		{
			m_animation = Animation::NONE;
			AZ::TickBus::Handler::BusDisconnect();

			EBUS_EVENT(GameNotificationBus, OnGameLoading);
		}
		break;
		
		case Animation::AFTER_LOADING:
		{
			m_animation = Animation::NONE;
			AZ::TickBus::Handler::BusDisconnect();

			ShowUiElement(m_hudEntityId);

			EBUS_EVENT(GameNotificationBus, OnGameStarted);

			HideUiElement(m_loadingTextEntityId);
			HideUiElement(m_loadingScreenEntityId, FADE_SPEED);
		}
		break;

		case Animation::TAKE_OFF:
		{
			m_timer = m_fadeDuration;

			ShowUiElement(m_loadingScreenEntityId, FADE_SPEED);
			m_animation = Animation::AFTER_TAKE_OFF;
		}
		break;

		case Animation::AFTER_TAKE_OFF:
		{
			HideUiElement(m_mainMenuEntityId);

			StartGame();			
		}
		break;

		case Animation::BEFORE_LAND:
		{
			EBUS_EVENT(GameNotificationBus, OnGameDestroyed);

			EBUS_EVENT_ID(m_menuCamera, Camera::CameraRequestBus, MakeActiveView);

			HideUiElement(m_hudEntityId);

			ShowUiElement(m_loadingScreenEntityId, 0.f);
			HideUiElement(m_loadingScreenEntityId, FADE_SPEED);

			m_timer = m_liftDuration;
			m_animation = Animation::LAND;

			return;
		}
		break;

		case Animation::LAND:
		{
			ShowMainMenu();

			m_animation = Animation::NONE;
			AZ::TickBus::Handler::BusDisconnect();
		}
		break;

		case Animation::COLLECTABLE:
		{
			HideUiElement(m_negativeCollectableEntityId);
			HideUiElement(m_positiveCollectableEntityId);

			m_animation = Animation::NONE;
			AZ::TickBus::Handler::BusDisconnect();
		}
		break;

		default:
		{
			m_animation = Animation::NONE;
			AZ::TickBus::Handler::BusDisconnect();
		}
	}
}

void UiComponent::OnGamePaused()
{
	ShowUiElement(m_pauseMenuEntityId);
}

void UiComponent::ResumeGame()
{
	HideUiElement(m_pauseMenuEntityId);

	EBUS_EVENT(GameNotificationBus, OnGameResumed);
}

void UiComponent::OnGameEnded()
{
	HideUiElement(m_hudEntityId);
	HideUiElement(m_pauseMenuEntityId);

	HideUiElement(m_gameCompletedEntityId);
	ShowUiElement(m_gameFailedEntityId);

	ShowUiElement(m_endMenuEntityId);
}

void UiComponent::EndGame()
{
	EBUS_EVENT(GameNotificationBus, OnGameEnded);

	HideUiElement(m_gameFailedEntityId);
	ShowUiElement(m_gameCompletedEntityId);
}

void UiComponent::DestroyGame()
{
	m_timer = m_fadeDuration;
	m_animation = Animation::BEFORE_LAND;

	ShowUiElement(m_loadingScreenEntityId, FADE_SPEED);

	AZ::TickBus::Handler::BusConnect();
}

void UiComponent::OnScoreChanged(TotalPoints i_newPoints)
{
	const AZStd::string score = AZStd::to_string(i_newPoints);

	EBUS_EVENT_ID(m_scoreEntityId, UiTextBus, SetText, score);
	EBUS_EVENT_ID(m_finalScoreEntityId, UiTextBus, SetText, score);
}

void UiComponent::OnClaimedTilesChanged(TileCount i_newClaimedTiles)
{
	EBUS_EVENT_ID(m_claimedTilesEntityId, UiTextBus, SetText, AZStd::to_string(i_newClaimedTiles));
}

void UiComponent::OnLandingEnded()
{
	HideUiElement(m_warningInstructionsEntityId);
	ShowUiElement(m_endInstructionsEntityId);

	EBUS_EVENT_ID(m_endGameEntityId, UiInteractableBus, SetIsHandlingEvents, true);
}

void UiComponent::OnTakeOffStarted()
{
	HideUiElement(m_endInstructionsEntityId);
	ShowUiElement(m_warningInstructionsEntityId);

	EBUS_EVENT_ID(m_endGameEntityId, UiInteractableBus, SetIsHandlingEvents, false);
}

void UiComponent::OnEnergySavingModeActivated()
{
	ShowUiElement(m_lowEnergyModeEntityId);
	SwapUiElements(m_spaceshipEnergyEntityId, m_spaceshipLowEnergyEntityId);
}

void UiComponent::OnEnergySavingModeDeactivated()
{
	HideUiElement(m_lowEnergyModeEntityId);
	SwapUiElements(m_spaceshipEnergyEntityId, m_spaceshipHighEnergyEntityId);
}

void UiComponent::OnSpaceshipEnergyChanged(float i_normalizedNewEnergy)
{
	EBUS_EVENT_ID(*m_spaceshipEnergyEntityId, UiImageBus, SetFillAmount, i_normalizedNewEnergy);
}

void UiComponent::OnTileEnergyChanged(const AZ::EntityId& i_tileEntityId, float i_normalizedNewEnergy)
{
	if(i_tileEntityId != m_selectedTileEntityId)
	{
		return ;
	}

	EBUS_EVENT_ID(*m_tileEnergyEntityId, UiImageBus, SetFillAmount, i_normalizedNewEnergy);
}

void UiComponent::OnTileClaimed(const AZ::EntityId& i_tileEntityId)
{
	if(i_tileEntityId != m_selectedTileEntityId)
	{
		return ;
	}

	SwapUiElements(m_tileEnergyEntityId, m_tileHighEnergyEntityId);
}

void UiComponent::OnTileLost(const AZ::EntityId& i_tileEntityId)
{
	if(i_tileEntityId != m_selectedTileEntityId)
	{
		return ;
	}

	SwapUiElements(m_tileEnergyEntityId, m_tileLowEnergyEntityId);
}
	
void UiComponent::OnTileSelected(const AZ::EntityId& i_tileEntityId)
{
	if(!m_selectedTileEntityId.IsValid())
	{
		ShowUiElement(m_tileEnergyBarEntityId);
		ShowUiElement(m_energyBarsSeparatorEntityId);
	}

	bool isClaimed { false };
	EBUS_EVENT_ID_RESULT(isClaimed, i_tileEntityId, TileRequestBus, IsClaimed);

	if(isClaimed && m_tileEnergyEntityId != &m_tileHighEnergyEntityId)
	{
		SwapUiElements(m_tileEnergyEntityId, m_tileHighEnergyEntityId);
	}
	else if(!isClaimed && m_tileEnergyEntityId != &m_tileLowEnergyEntityId)
	{
		SwapUiElements(m_tileEnergyEntityId, m_tileLowEnergyEntityId);
	}

	m_selectedTileEntityId = i_tileEntityId;
}

void UiComponent::OnTileDeselected(const AZ::EntityId& i_tileEntityId)
{
	if(i_tileEntityId != m_selectedTileEntityId)
	{
		return;
	}

	HideUiElement(m_tileEnergyBarEntityId);
	HideUiElement(m_energyBarsSeparatorEntityId);

	m_selectedTileEntityId.SetInvalid();
}

void UiComponent::OnStopDecayCollected(float i_duration)
{
	SetCollectableText(AZStd::string::format("Block tiles for %.f sec", i_duration), true);
}

void UiComponent::OnSpaceshipEnergyCollected(float i_energy)
{
	const bool isDamage = (i_energy < 0.f);
	SetCollectableText(AZStd::string::format("%s%.f energy to spaceship", (isDamage) ? "-" : "+", AZStd::abs(i_energy)), !isDamage);
}

void UiComponent::OnTileEnergyCollected(float i_energy)
{
	const bool isDamage = (i_energy < 0.f);
	SetCollectableText(AZStd::string::format("%s%.f energy to all tiles", (isDamage) ? "-" : "+", AZStd::abs(i_energy)), !isDamage);
}

void UiComponent::OnPointsCollected(Points i_points)
{
	SetCollectableText(AZStd::string::format("+%u points", i_points), true);
}

void UiComponent::OnSpeedCollected(float i_multiplier, float i_duration)
{
	const bool isBoost = (i_multiplier > 1.f);
	SetCollectableText(AZStd::string::format("x%.1f speed for %.f sec", i_multiplier, i_duration), isBoost);
}

void UiComponent::SetCollectableText(const AZStd::string& i_message, bool i_isPositive)
{
	if(m_animation == Animation::COLLECTABLE)
	{
		if(i_isPositive)
		{
			HideUiElement(m_negativeCollectableEntityId);
		}
		else
		{
			HideUiElement(m_positiveCollectableEntityId);
		}
	}

	const AZ::EntityId& collectableEntityId = (i_isPositive) ? m_positiveCollectableEntityId : m_negativeCollectableEntityId;
	ShowUiElement(collectableEntityId);

	EBUS_EVENT_ID(collectableEntityId, UiTextBus, SetText, i_message);

	m_animation = Animation::COLLECTABLE;
	m_timer = m_collectableNotificationDuration;

	AZ::TickBus::Handler::BusConnect();
}

void UiComponent::ConnectOnButtonClick(const AZ::EntityId& i_buttonEntityId, const UiButtonInterface::OnClickCallback& i_callback)
{
	EBUS_EVENT_ID(i_buttonEntityId, UiButtonBus, SetOnClickCallback, i_callback);
}

void UiComponent::ShowUiElement(const AZ::EntityId& i_elementEntityId)
{
	EBUS_EVENT_ID(i_elementEntityId, UiElementBus, SetIsEnabled, true);
}

void UiComponent::ShowUiElement(const AZ::EntityId& i_elementEntityId, float i_fadeSpeed)
{
	EBUS_EVENT_ID(i_elementEntityId, UiFaderBus, Fade, 1.f, i_fadeSpeed);
}

void UiComponent::HideUiElement(const AZ::EntityId& i_elementEntityId)
{
	EBUS_EVENT_ID(i_elementEntityId, UiElementBus, SetIsEnabled, false);
}

void UiComponent::HideUiElement(const AZ::EntityId& i_elementEntityId, float i_fadeSpeed)
{
	EBUS_EVENT_ID(i_elementEntityId, UiFaderBus, Fade, 0.f, i_fadeSpeed);
}

void UiComponent::SwapUiElements(const AZ::EntityId*& io_currentEntityId, const AZ::EntityId& i_newEntityId)
{
	HideUiElement(*io_currentEntityId);

	io_currentEntityId = &i_newEntityId;
	ShowUiElement(i_newEntityId);
}
