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

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/string/string.h>

#include <LyShine/Bus/World/UiCanvasRefBus.h>
#include <LyShine/Bus/UiButtonBus.h>

#include "../EBuses/CollectableBus.hpp"
#include "../EBuses/GameBus.hpp"
#include "../EBuses/ScoreBus.hpp"
#include "../EBuses/SpaceshipBus.hpp"
#include "../EBuses/TileBus.hpp"


namespace Loherangrin::Games::O3DEJam2305
{
	class UiComponent
		: public AZ::Component
		, protected AZ::TickBus::Handler
		, protected UiCanvasAssetRefNotificationBus::Handler
		, protected CollectablesNotificationBus::Handler
		, protected GameNotificationBus::Handler
		, protected ScoreNotificationBus::Handler
		, protected SpaceshipNotificationBus::Handler
		, protected TilesNotificationBus::Handler
	{
	public:
		AZ_COMPONENT(UiComponent, "{B92A6A1F-E706-48AE-903E-2B21912D58E9}");
		static void Reflect(AZ::ReflectContext* io_context);

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& io_provided);
		static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& io_incompatible);
		static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& io_required);
		static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& io_dependent);

	protected:
		// AZ::Component
		void Activate() override;
		void Deactivate() override;

		// UiCanvasRefNotificationBus
		void OnCanvasLoadedIntoEntity(AZ::EntityId i_uiCanvasEntity) override;

		// AZ::TickBus
		void OnTick(float i_deltaTime, AZ::ScriptTimePoint i_time) override;

		// CollectablesNotificationBus
		void OnStopDecayCollected(float i_duration);
		void OnSpaceshipEnergyCollected(float i_energy);
		void OnTileEnergyCollected(float i_energy);
		void OnPointsCollected(Points i_points);
		void OnSpeedCollected(float i_multiplier, float i_duration);

		// GameNotificationBus
		void OnGamePaused() override;
		void OnGameEnded() override;

		// ScoreNotificationBus
		void OnScoreChanged(TotalPoints i_newPoints) override;
		void OnClaimedTilesChanged(TileCount i_newClaimedTiles) override;

		// SpaceshipNotificationBus
		void OnSpaceshipEnergyChanged(float i_normalizedNewEnergy) override;
		void OnEnergySavingModeActivated() override;
		void OnEnergySavingModeDeactivated() override;
		void OnLandingEnded() override;
		void OnTakeOffStarted() override;

		// TilesNotificationBus
		void OnAllTilesCreated();
		void OnTileEnergyChanged(const AZ::EntityId& i_tileEntityId, float i_normalizedNewEnergy) override;
		void OnTileClaimed(const AZ::EntityId& i_tileEntityId) override;
		void OnTileLost(const AZ::EntityId& i_tileEntityId) override;
		void OnTileSelected(const AZ::EntityId& i_tileEntityId);
		void OnTileDeselected(const AZ::EntityId& i_tileEntityId);

	private:
		enum class Animation : AZ::u8
		{
			NONE = 0,
			TAKE_OFF,
			AFTER_TAKE_OFF,
			BEFORE_LAND,
			LAND,
			LOADING,
			AFTER_LOADING,
			COLLECTABLE
		};

		bool FindAllUiElements(const AZ::EntityId& i_canvasId);
		AZ::EntityId FindUiElement(const char* i_elementName) const;

		void InitializeAllUiElements();
		
		void ShowMainMenu();
		void CreateGame();
		void StartGame();
		void ResumeGame();
		void EndGame();
		void DestroyGame();

		void SetCollectableText(const AZStd::string& i_message, bool i_isPositive);

		static void ConnectOnButtonClick(const AZ::EntityId& i_buttonEntityId, const UiButtonInterface::OnClickCallback& i_callback);

		static void ShowUiElement(const AZ::EntityId& i_elementEntityId);
		static void ShowUiElement(const AZ::EntityId& i_elementEntityId, float i_fadeSpeed);

		static void HideUiElement(const AZ::EntityId& i_elementEntityId);
		static void HideUiElement(const AZ::EntityId& i_elementEntityId, float i_fadeSpeed);

		void SwapUiElements(const AZ::EntityId*& io_currentEntityId, const AZ::EntityId& i_newEntityId);

		Animation m_animation { Animation::NONE };

		float m_collectableNotificationDuration { 2.f };
		float m_liftDuration { 2.f };
		float m_fadeDuration { 3.f };
		float m_timer { -1.f };

		AZ::EntityId m_selectedTileEntityId {};

		AZ::EntityId m_menuCamera {};
		AZ::EntityId m_gameCamera {};

		AZ::EntityId m_canvasId {};

		AZ::EntityId m_hudEntityId {};
		AZ::EntityId m_loadingScreenEntityId {};
		AZ::EntityId m_mainMenuEntityId {};
		AZ::EntityId m_pauseMenuEntityId {};
		AZ::EntityId m_endMenuEntityId {};

		// Hud
		AZ::EntityId m_lowEnergyModeEntityId {};
		AZ::EntityId m_energyBarsSeparatorEntityId {};
		AZ::EntityId m_spaceshipLowEnergyEntityId {};
		AZ::EntityId m_spaceshipHighEnergyEntityId {};
		AZ::EntityId m_tileEnergyBarEntityId {};
		AZ::EntityId m_tileLowEnergyEntityId {};
		AZ::EntityId m_tileHighEnergyEntityId {};
		AZ::EntityId m_claimedTilesEntityId {};
		AZ::EntityId m_scoreEntityId {};
		AZ::EntityId m_positiveCollectableEntityId {};
		AZ::EntityId m_negativeCollectableEntityId {};

		// Loading
		AZ::EntityId m_loadingTextEntityId {};

		// Main menu
		AZ::EntityId m_startGameEntityId {};
		AZ::EntityId m_exitGameEntityId {};

		// Pause menu
		AZ::EntityId m_warningInstructionsEntityId {};
		AZ::EntityId m_endInstructionsEntityId {};
		AZ::EntityId m_resumeGameEntityId {};
		AZ::EntityId m_cancelGameEntityId {};
		AZ::EntityId m_endGameEntityId {};

		// End menu
		AZ::EntityId m_gameCompletedEntityId {};
		AZ::EntityId m_gameFailedEntityId {};
		AZ::EntityId m_finalScoreEntityId {};
		AZ::EntityId m_retryGameEntityId {};
		AZ::EntityId m_returnMainMenuEntityId {};

		const AZ::EntityId* m_spaceshipEnergyEntityId { nullptr };
		const AZ::EntityId* m_tileEnergyEntityId { nullptr };

		static constexpr float FADE_SPEED = 0.5f;

		static constexpr const char* UI_HUD = "Hud";
		static constexpr const char* UI_HUD_LOW_ENERGY_MODE_TEXT = "LowEnergyMode";
		static constexpr const char* UI_HUD_ENERGY_BARS_SEPARATOR_IMAGE = "Separator";
		static constexpr const char* UI_HUD_SPACESHIP_LOW_ENERGY_IMAGE = "SpaceshipEnergy_LowValue";
		static constexpr const char* UI_HUD_SPACESHIP_HIGH_ENERGY_IMAGE = "SpaceshipEnergy_HighValue";
		static constexpr const char* UI_HUD_TILE_ENERGY_BAR = "TileEnergy";
		static constexpr const char* UI_HUD_TILE_LOW_ENERGY_IMAGE = "TileEnergy_LowValue";
		static constexpr const char* UI_HUD_TILE_HIGH_ENERGY_IMAGE = "TileEnergy_HighValue";
		static constexpr const char* UI_HUD_CLAIMED_TILES_TEXT = "ClaimedTiles_Value";
		static constexpr const char* UI_HUD_SCORE_TEXT = "Score_Value";
		static constexpr const char* UI_HUD_POSITIVE_COLLECTABLE_TEXT = "Collectable_Positive";
		static constexpr const char* UI_HUD_NEGATIVE_COLLECTABLE_TEXT = "Collectable_Negative";

		static constexpr const char* UI_LOADING = "LoadingScreen";
		static constexpr const char* UI_LOADING_TEXT = "Loading_Text";

		static constexpr const char* UI_MAIN_MENU = "MainMenu";
		static constexpr const char* UI_MAIN_MENU_START_BUTTON = "StartButton";
		static constexpr const char* UI_MAIN_MENU_EXIT_BUTTON = "ExitButton";

		static constexpr const char* UI_PAUSE_MENU = "PauseMenu";
		static constexpr const char* UI_PAUSE_MENU_END_INSTRUCTIONS_TEXT = "Instructions_End";
		static constexpr const char* UI_PAUSE_MENU_WARNING_INSTRUCTIONS_TEXT = "Instructions_Warning";
		static constexpr const char* UI_PAUSE_MENU_RESUME_BUTTON = "ResumeButton";
		static constexpr const char* UI_PAUSE_MENU_CANCEL_BUTTON = "CancelButton";
		static constexpr const char* UI_PAUSE_MENU_END_BUTTON = "EndButton";

		static constexpr const char* UI_END_MENU = "EndMenu";
		static constexpr const char* UI_END_MENU_GAME_COMPLETED = "Background_Completed";
		static constexpr const char* UI_END_MENU_GAME_FAILED = "Background_Failed";
		static constexpr const char* UI_END_MENU_FINAL_SCORE_TEXT = "FinalScore_Value";
		static constexpr const char* UI_END_MENU_RETRY_BUTTON = "RetryButton";
		static constexpr const char* UI_END_MENU_RETURN_BUTTON = "ReturnButton";
	};

} // Loherangrin::Games::O3DEJam2305
