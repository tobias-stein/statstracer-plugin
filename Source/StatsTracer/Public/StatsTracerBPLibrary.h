///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracer\Public\StatsTracerBPLibrary.h
///
/// Summary:	Declares the statistics tracer bp library class.
///-------------------------------------------------------------------------------------------------

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "StatsTracerBPLibrary.generated.h"


class UTracer;

UCLASS()
class UStatsTracerBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
		
	/* Creates a new stats tracer for this object. Use the returned tracer object to add stats for tracing. If you create new tracer instances after the OnBeginPlay event you should call the Start method of tracer manually. */
	UFUNCTION(
		BlueprintCallable, 
		Category = "StatsTracer", 
		meta = (
			DisplayName = "Create State Tracer", 
			Keywords = "StatsTracer create stat tracer", 
			DefaultToSelf = "TracedActor",
			HidePin = "TracedActor",
			UnsafeDuringActorConstruction = "true"))
	static UTracer* CreateStatsTracer(
		AActor* TracedActor, 
		const FString& name, 
		const FString& description,
		const bool enableCsvStream = false,
		const bool autostartOnBeginPlay = true);
};
