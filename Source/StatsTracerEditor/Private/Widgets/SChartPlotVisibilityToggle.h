///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Private\Widgets\SChartPlotVisibilityToggle.h
///
/// Summary:	Declares the chart plot visibility toggle class.
///-------------------------------------------------------------------------------------------------

#pragma once

#include "CoreMinimal.h"

#include "StatsTracerCore.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_OneParam(FOnToggleVisibility, bool)

/**
 * 
 */
class SChartPlotVisibilityToggle : public SImage
{
private:

	bool isVisible;

	FOnToggleVisibility	OnToggleVisibility;

public:

	SLATE_BEGIN_ARGS(SChartPlotVisibilityToggle) :
		_InitialVisible(),
		_OnToggleVisibility()
	{}
	SLATE_ARGUMENT(bool, InitialVisible);
	SLATE_EVENT(FOnToggleVisibility, OnToggleVisibility);
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	const FSlateBrush* GetBrush() const;

	inline bool IsVisible() const { return this->isVisible; }

	FReply HandleClick();

	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
};
