///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Private\Widgets\SStatsTracerMemoryLoadDisplay.h
///
/// Summary:	Declares the statistics tracer memory load display class.
///-------------------------------------------------------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"


DECLARE_DELEGATE(FCleanMemoryDelegate)

/**
 * 
 */
class SStatsTracerMemoryLoadDisplay : public SCompoundWidget
{
private:

	FCleanMemoryDelegate	OnCleanMemoryDelegate;

public:

	SLATE_BEGIN_ARGS(SStatsTracerMemoryLoadDisplay) :
		_OnCleanMemoryDelegate()
	{}
	SLATE_EVENT(FCleanMemoryDelegate, OnCleanMemoryDelegate);
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
};
