///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracer\Private\StatsTracerEditorSettings.cpp.
///
/// Summary:	Implements the statistics tracer editor settings class.
///-------------------------------------------------------------------------------------------------

#include "StatsTracerEditorSettings.h"
#include "StatsTracerPCH.h"

#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"

UStatsTracerEditorSettings::UStatsTracerEditorSettings(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	// general
	this->SampleWindowSize = 1024;
	this->UpdateFrequency = 1;
	this->SessionCapacity = 10;
	this->PhysicalMemoryLimit = 128; // 128 Mbyte

	// visual appearance
	this->ChartShowGrid = true;
	this->ChartShowIndicatorCross = true;
	this->ChartShowMultiStatsSubLabels = true;
	this->ChartXAxisGridSize = 50;
	this->ChartYAxisGridSize = 50;
	this->ChartXAxisTimelineMode = ETimelineMode::Time;
	this->ChartReferencePointerTimelineMode = ETimelineMode::Time;

	// csv settings
	this->CsvOutputDir.Path = FString::Printf(TEXT("%s/%s"), *FDesktopPlatformModule::Get()->GetUserTempPath(), TEXT("StatsTracerPlugin"));

	// 'Tracer' component
	this->GlobalStatsFilter =
	{
		TEXT("bHiddenEd[ \\w\\d]*"),
		TEXT("bCanBe[ \\w\\d]*"),
		TEXT("bReplicates[ \\w\\d]*"),
		TEXT("bUsedInEditor[ \\w\\d]*"),
		TEXT("bUseInEditor[ \\w\\d]*"),
		TEXT("bUseControllerRotation[ \\w\\d]*"),
		TEXT("bReplicate[ \\w\\d]*"),

		TEXT("bNetLoadOnClient"),
		TEXT("bNetAddressable"),
		TEXT("bNetUseOwnerRelevancy"),
		TEXT("bNetTemporary"),
		TEXT("bNetStartup"),

		TEXT("bIsEditorOnly"),
		TEXT("bIsEditorPreviewActor"),
		TEXT("bAllowTickBeforeBeginPlay"),
		TEXT("bInputPriority"),
		TEXT("bAutoDestroyWhenFinished"),
		TEXT("bCollideWhenPlacing"),
		TEXT("bRelevantForNetworkReplays"),
		TEXT("bGenerateOverlapEventsDuringLevelStreaming"),
		TEXT("bAllowReceiveTickEventOnDedicatedServer"),
		TEXT("bEnableAutoLODGeneration"),
		TEXT("bAlwaysCreatePhysicsState"),
		TEXT("bOnlyRelevantToOwner"),
		TEXT("bActorLabelEditable"),
		TEXT("bHiddenInGame"),
		TEXT("bUseAttachParentBound"),
		TEXT("bEditableWhenInherited"),
		TEXT("bCanEverAffectNavigation"),

	};
}

#if WITH_EDITOR
void UStatsTracerEditorSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	this->SampleWindowSize = FMath::RoundUpToPowerOfTwo(this->SampleWindowSize);
}
#endif