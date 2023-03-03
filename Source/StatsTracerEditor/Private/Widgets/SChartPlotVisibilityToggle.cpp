///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Private\Widgets\SChartPlotVisibilityToggle.cpp
///
/// Summary:	Implements the chart plot visibility toggle class.
///-------------------------------------------------------------------------------------------------

#include "SChartPlotVisibilityToggle.h"
#include "StatsTracerEditorPCH.h"



BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SChartPlotVisibilityToggle::Construct(const FArguments& InArgs)
{
	this->isVisible = InArgs._InitialVisible;
	this->OnToggleVisibility = InArgs._OnToggleVisibility;

	SImage::Construct(
		SImage::FArguments()
		.Image(this, &SChartPlotVisibilityToggle::GetBrush)
	);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

/** Get the brush for this widget */
const FSlateBrush* SChartPlotVisibilityToggle::GetBrush() const
{
	if (IsVisible() == true)
	{
		return IsHovered() ? FStatsTracerEditorStyle::Get().GetBrush("VisibleIcon.Highlighted") :
			FStatsTracerEditorStyle::Get().GetBrush("VisibleIcon");
	}
	else
	{
		return IsHovered() ? FStatsTracerEditorStyle::Get().GetBrush("NotVisibleIcon.Highlighted") :
			FStatsTracerEditorStyle::Get().GetBrush("NotVisibleIcon");
	}
}

FReply SChartPlotVisibilityToggle::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	return HandleClick();
}

/** Called when the mouse button is pressed down on this widget */
FReply SChartPlotVisibilityToggle::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	return HandleClick();
}

FReply SChartPlotVisibilityToggle::HandleClick()
{
	this->isVisible = !this->isVisible;
	this->OnToggleVisibility.ExecuteIfBound(this->isVisible);
	return FReply::Handled();
}