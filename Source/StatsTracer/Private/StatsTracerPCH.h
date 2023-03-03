///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
/// 
/// File:	StatsTracer\Private\StatsTracerPCH.h
///
/// Summary:	Declares the statistics tracer pch class.
///-------------------------------------------------------------------------------------------------

#pragma once


namespace StatsTracer {

#if PLATFORM_WINDOWS
	static const FString NEWLINE = FString(TEXT("\r\n"));
#elif PLATFORM_LINUX
	static const FString NEWLINE = FString(TEXT("\n"));
#elif PLATFORM_MAC
	static const FString NEWLINE = FString(TEXT("\r"));
#else
	static const FString NEWLINE = FString(TEXT("\r\n"));
#endif
}

#include "CoreUObject.h"

#include "CoreMinimal.h"
#include "Engine.h"
#include "UnrealEd.h"

#include "StatsTracerProfiler.h"

#include "CSVStream.h"

#include "StatsTracer.h"
#include "StatsTracerCore.h"

#include "Tracer.h"
#include "TracerComponent.h"
#include "StatsTracerSessionRunner.h"

#include "StatsTracerBPLibrary.h"

#include "StatsTracerEditorSettings.h"