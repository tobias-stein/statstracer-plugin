///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Private\Widgets\SChartPlotResizer.cpp
///
/// Summary:	Implements the chart plot resizer class.
///-------------------------------------------------------------------------------------------------

#include "SChartPlotResizer.h"
#include "StatsTracerEditorPCH.h"


void SChartPlotResizer::Construct(const FArguments& InArgs)
{
	this->OnResize = InArgs._OnResize;

	isResizing = false;

	ChildSlot
		[
			SNew(SBox)
			.MinDesiredHeight(4.0f)
		];
}

FReply SChartPlotResizer::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		isResizing = true;
		return FReply::Handled().CaptureMouse(SharedThis(this));
	}
	else
	{
		return FReply::Unhandled();
	}
}

FReply SChartPlotResizer::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && isResizing == true)
	{
		isResizing = false;
		return FReply::Handled().ReleaseMouseCapture();
	}
	return FReply::Unhandled();
}

FReply SChartPlotResizer::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (isResizing == true)
	{
		Cursor = EMouseCursor::ResizeUpDown;
		this->OnResize.ExecuteIfBound(MouseEvent.GetCursorDelta().Y);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void SChartPlotResizer::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	Cursor = EMouseCursor::ResizeUpDown;
}

void SChartPlotResizer::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	Cursor = EMouseCursor::Default;
}
