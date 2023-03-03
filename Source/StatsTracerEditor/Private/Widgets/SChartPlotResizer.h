///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Private\Widgets\SChartPlotResizer.h
///
/// Summary:	Declares the chart plot resizer class.
///-------------------------------------------------------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "StatsTracerCore.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_OneParam(FResizeDelegate, float)



class SChartPlotResizer : public SCompoundWidget
{
private:

	bool			isResizing;

	FResizeDelegate	OnResize;

public:

	SLATE_BEGIN_ARGS(SChartPlotResizer) :
		_OnResize()
	{}
	SLATE_EVENT(FResizeDelegate, OnResize);
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs);

	inline bool IsResizing() const { return this->isResizing; }
	inline void StopResizing() { this->isResizing = false; }

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
};