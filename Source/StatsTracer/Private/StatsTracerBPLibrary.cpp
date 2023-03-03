///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracer\Private\StatsTracerBPLibrary.cpp
///
/// Summary:	Implements the statistics tracer bp library class.
///-------------------------------------------------------------------------------------------------

#include "StatsTracerBPLibrary.h"
#include "StatsTracerPCH.h"

UStatsTracerBPLibrary::UStatsTracerBPLibrary(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer)
{}


UTracer* UStatsTracerBPLibrary::CreateStatsTracer(
	AActor* TracedActor,
	const FString& name, 
	const FString& description, 
	const bool enableCsvStream,
	const bool autostartOnBeginPlay)
{
	// Begin critical section ...
	static FCriticalSection MUTEX;
	FScopeLock LOCK_GUARD(&MUTEX);

	// instanciate new tracer object
	auto tracerObject = NewObject<UTracer>();

	// initialize it
	tracerObject->Initialize(
		name,
		description, 
		TracedActor,
		enableCsvStream,
		autostartOnBeginPlay);

	return tracerObject;
	// ... End critical section
}
