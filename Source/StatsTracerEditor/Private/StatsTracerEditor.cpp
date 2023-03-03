///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Private\StatsTracerEditor.cpp
///
/// Summary:	Implements the statistics tracer editor class.
///-------------------------------------------------------------------------------------------------

#include "StatsTracerEditor.h"
#include "StatsTracerEditorPCH.h"

#include "LevelEditor.h"

#include "PropertyEditorDelegates.h"
#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FStatsTracerEditorModule"

void FStatsTracerEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FStatsTracerEditorStyle::Initialize();
	FStatsTracerEditorStyle::ReloadTextures();

	FStatsTracerEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FStatsTracerEditorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FStatsTracerEditorModule::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	

	// Add extension to Menu 'Debug'
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension(
			"Debug", 
			EExtensionHook::After, 
			PluginCommands, 
			FMenuExtensionDelegate::CreateRaw(this, &FStatsTracerEditorModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	// Add extension to top toolbar, at the end of 'Settings'
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension(
			"Settings", 
			EExtensionHook::After, 
			PluginCommands, 
			FToolBarExtensionDelegate::CreateRaw(this, &FStatsTracerEditorModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(StatsTracerEditorMajorTabName, FOnSpawnTab::CreateRaw(this, &FStatsTracerEditorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FStatsTracerEditorTabTitle", "StatsTracer - Session Overview"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);


	// Register TracerComponent details customization
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout("TracerComponent", FOnGetDetailCustomizationInstance::CreateStatic(&TracerComponentCustomization::MakeInstance));
}

void FStatsTracerEditorModule::ShutdownModule()
{

	// Unregister TracerComponent details customization
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomClassLayout("TracerComponent");

	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FStatsTracerEditorStyle::Shutdown();

	FStatsTracerEditorCommands::Unregister();
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(StatsTracerEditorMajorTabName);
}


TSharedRef<SDockTab> FStatsTracerEditorModule::OnSpawnPluginTab(const class FSpawnTabArgs& Args)
{
	TSharedRef<SDockTab> StatsTracerEditorTab = 	
		SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[		
			SNew(STracerSessionOverview)
		];

	return StatsTracerEditorTab;
}

void FStatsTracerEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->InvokeTab(StatsTracerEditorMajorTabName);
}

void FStatsTracerEditorModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FStatsTracerEditorCommands::Get().OpenPluginWindow);
}

void FStatsTracerEditorModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FStatsTracerEditorCommands::Get().OpenPluginWindow);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FStatsTracerEditorModule, StatsTracerEditor)