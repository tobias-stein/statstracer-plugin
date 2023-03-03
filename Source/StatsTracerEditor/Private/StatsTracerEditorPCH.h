///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Private\StatsTracerEditorPCH.h
///
/// Summary:	Declares the statistics tracer editor pch class.
///-------------------------------------------------------------------------------------------------

#pragma once

#include "CoreUObject.h"

#include "Engine.h"
#include "UnrealEd.h"

#include "StatsTracerEditor.h"
#include "StatsTracerCore.h"
#include "StatsTracerEditorCore.h"
#include "StatsTracerEditorStyle.h"


#include "TracerComponentCustomization.h"
#include "StatsTracerEditorCommands.h"
#include "StatsTracerEditorSettings.h"


#include "SlateOptMacros.h"
#include "EditorStyleSet.h"


#include "Widgets/STracerSessionOverview.h"
#include "Widgets/STracerDataOverview.h"
#include "Widgets/SStatsTracerMemoryLoadDisplay.h"
#include "Widgets/STracerDataChart.h"
#include "Widgets/SChartPlotResizer.h"
#include "Widgets/SChartPlotVisibilityToggle.h"

#include "Widgets/SWindow.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Colors/SColorPicker.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "Rendering/DrawElements.h"

#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Notifications/NotificationManager.h"

#include "Fonts/FontMeasure.h"
#include "Fonts/SlateFontInfo.h"