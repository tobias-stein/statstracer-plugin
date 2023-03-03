///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Private\Widgets\SStatsTracerMemoryLoadDisplay.cpp
///
/// Summary:	Implements the statistics tracer memory load display class.
///-------------------------------------------------------------------------------------------------

#include "SStatsTracerMemoryLoadDisplay.h"
#include "StatsTracerEditorPCH.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SStatsTracerMemoryLoadDisplay::Construct(const FArguments& InArgs)
{
	/**

	+------------------------------------------------------+-------+
	| <Title>                                              |       |
	+------------------------------------------------------+-------+
	|+----------------------------------------------------+| +---+ |
	||===========                                         || | x | |
	|+----------------------------------------------------+| +---+ |
	+------------------+------------------+----------------+-------+
	|0 MB              |   <current MB>   |      <Limit MB>|       |
	+------------------+------------------+----------------+-------+

	Vert
	[
		Horz
		[
			Vert
			[
			]

			Vert
			[

			]
		]
	]
	*/

	this->OnCleanMemoryDelegate = InArgs._OnCleanMemoryDelegate;

	ChildSlot
	[
		SNew(SBorder)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(FMargin(4.0f, 4.0f))
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					[
						SNew(STextBlock)
						.Font(FStatsTracerEditorStyle::Get().GetFontStyle("BigBoldFont"))
						.Text(FText::FromString("StatsTracer Memory Consumption"))
					]

					+SVerticalBox::Slot()
					[
						SNew(SBox)
						.MinDesiredHeight(32.0f)
						[
							SNew(SProgressBar)
							.Percent_Lambda([]()
							{
								return StatsTracer::TDRM->GetTotalLimitRatio();
							})
							.FillColorAndOpacity_Lambda([]()
							{
								float t =  StatsTracer::TDRM->GetTotalLimitRatio();

								return FSlateColor(FColor::MakeRedToGreenColorFromScalar(1.0f - (t*t)).ReinterpretAsLinear());
							})
						]
					]

					+SVerticalBox::Slot()
					.MaxHeight(16.0f)
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Top)
						[
							SNew(STextBlock)
							.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont"))
							.Text(FText::FromString("0 Mbyte"))
						]

						+SHorizontalBox::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Top)
						[
							SNew(SBox)
							.MinDesiredHeight(32.0f)
							[
								SNew(STextBlock)
								.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont"))
								.Text_Lambda([]()
								{
									return FText::FromString(FString::Printf(TEXT("%.2f Mbyte"), ((float)StatsTracer::TDRM->GetTotalPhysicalMemory() / 1048576.0f))); // convert to megebyte
								})
							]
						]

						+SHorizontalBox::Slot()
						.HAlign(HAlign_Right)
						.VAlign(VAlign_Top)
						[
							SNew(STextBlock)
							.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont"))
							.Text_Lambda([]()
							{
								return FText::FromString(FString::Printf(TEXT("%d Mbyte"), UStatsTracerEditorSettings::GetInstance()->PhysicalMemoryLimit));
							})
						]
					]
				]

				+SHorizontalBox::Slot()
				.MaxWidth(32.0f)
				.AutoWidth()
				.Padding(2.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					[
						SNew(STextBlock)
					]

					+SVerticalBox::Slot()
					[
						SNew(SButton)
						.ToolTipText(FText::FromString("Clear all sessions data."))
						.IsEnabled_Lambda([]()
						{
#if WITH_EDITOR
						return GEditor->GetPIEWorldContext() == nullptr;
#else
						return true;
#endif

						})
						.OnClicked_Lambda([&]()
						{					
							if (StatsTracer::TDRM->GetSessionCount() > 0 && FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString("Do You really want to clear all session data?")) == EAppReturnType::Yes)
							{
								StatsTracer::TDRM->ClearAllSessions();
								this->OnCleanMemoryDelegate.ExecuteIfBound();
							}
							
							return FReply::Handled();
						})
						.ForegroundColor(FSlateColor::UseForeground())
						[
							TSharedRef<SWidget>(SNew(SImage)
							.Image(FStatsTracerEditorStyle::Get().GetBrush("PurgeIcon")))
						]
					]

					+SVerticalBox::Slot()
					[
						SNew(STextBlock)
					]
				]
			]
		]
	];

}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
