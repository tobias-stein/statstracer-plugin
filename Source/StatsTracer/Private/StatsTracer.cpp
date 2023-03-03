///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracer\Private\StatsTracer.cpp
///
/// Summary:	Implements the statistics tracer class.
///-------------------------------------------------------------------------------------------------

#include "StatsTracer.h"
#include "StatsTracerPCH.h"

#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "ISettingsContainer.h"

#define LOCTEXT_NAMESPACE "FStatsTracerModule"

void FStatsTracerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	RegisterSettings();
}

void FStatsTracerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
	UnregisterSettings();
}

void FStatsTracerModule::RegisterSettings()
{
	ISettingsModule* settingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (settingsModule != nullptr)
	{
		// Register the settings
		ISettingsSectionPtr settingsSection = settingsModule->RegisterSettings(
			"Project", "Plugins", "StatsTracer",
			FText::FromString(TEXT("StatsTracer")),
			FText::FromString(TEXT("StatsTracer settings")),
			GetMutableDefault<UStatsTracerEditorSettings>()
		);

		if (settingsSection.IsValid() == true)
		{
			settingsSection->OnModified().BindLambda([this]()
			{
				UStatsTracerEditorSettings* settings = GetMutableDefault<UStatsTracerEditorSettings>();
				if (settings != nullptr)
				{
					settings->SaveConfig();
				}

				return true;
			});
		}
	}
}

void FStatsTracerModule::UnregisterSettings()
{
	ISettingsModule* settingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (settingsModule != nullptr)
	{
		settingsModule->UnregisterSettings("Project", "Plugins", "StatsTracer");
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FStatsTracerModule, StatsTracer)