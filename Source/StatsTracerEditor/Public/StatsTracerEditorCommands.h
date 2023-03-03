///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Public\StatsTracerEditorCommands.h
///
/// Summary:	Declares the statistics tracer editor commands class.
///-------------------------------------------------------------------------------------------------

#pragma once

//#include "CoreMinimal.h"
#include "Engine.h"
#include "Framework/Commands/Commands.h"
#include "StatsTracerEditorStyle.h"

class FStatsTracerEditorCommands : public TCommands<FStatsTracerEditorCommands>
{
public:

	FStatsTracerEditorCommands()
		: TCommands<FStatsTracerEditorCommands>(TEXT("StatsTracerEditor"), NSLOCTEXT("Contexts", "StatsTracerEditor", "StatsTracerEditor Plugin"), NAME_None, FStatsTracerEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};