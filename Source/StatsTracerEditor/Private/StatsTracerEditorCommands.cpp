///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Private\StatsTracerEditorCommands.cpp
///
/// Summary:	Implements the statistics tracer editor commands class.
///-------------------------------------------------------------------------------------------------

#include "StatsTracerEditorCommands.h"
#include "StatsTracerEditorPCH.h"


#define LOCTEXT_NAMESPACE "FStatsTracerEditorModule"

void FStatsTracerEditorCommands::RegisterCommands()
{
	UI_COMMAND(
		OpenPluginWindow, 
		"StatsTracer", 
		"Open StatsTracer tab", 
		EUserInterfaceActionType::Button, 
		FInputGesture());
}

#undef LOCTEXT_NAMESPACE
