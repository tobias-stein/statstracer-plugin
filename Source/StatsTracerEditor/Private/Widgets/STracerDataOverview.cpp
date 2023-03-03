///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Private\Widgets\STracerDataOverview.cpp.
///
/// Summary:	Implements the tracer data overview class.
///-------------------------------------------------------------------------------------------------

#include "STracerDataOverview.h"
#include "StatsTracerEditorPCH.h"




BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void STracerDataOverview::Construct(const FArguments& InArgs)
{
	this->Parent = InArgs._Parent;
	this->TracerData = InArgs._TracerData;

	this->DescriptionExpanded = false;
	this->StateExpanded = false;

	if (this->TracerData.IsValid() == false)
		return;


	TracerDataItems.Reset();
	for (auto& KVP : this->TracerData.Pin()->GetRepositoryData())
		TracerDataItems.Add(TTracerDataListItem(new FTracerDataListItem(KVP.Key, &KVP.Value)));


	ChildSlot
	.Padding(FMargin(8.0f))
	[
		SNew(SVerticalBox)

		// Tracer Description and Session-Info
		+SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.AutoHeight()
		[
			SNew(SExpandableArea)
			.Padding(0)
			.InitiallyCollapsed(!this->StateExpanded)
			.OnAreaExpansionChanged_Lambda([this](bool state) { this->StateExpanded = state; })
			.HeaderContent() 
			[
				SNew(STextBlock)
				.Font(FStatsTracerEditorStyle::Get().GetFontStyle("LargeBoldFont"))
				.Text(FText::FromString(this->TracerData.Pin()->GetRepositoryName()))
			]
			.BodyContent() 
			[
				SNew(STextBlock)
				.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalItalicFont"))
				.Text_Lambda([&]()
				{
					auto pinnedTracerData = this->TracerData.Pin();
					return pinnedTracerData->GetRepositoryDescription().IsEmpty() == false ? pinnedTracerData->GetRepositoryDescription() : FText::FromString("No description available.");
				})
			]
			.Padding(8.0f)
		]
		+SVerticalBox::Slot()
		.Padding(0.0f, 4.0f, 0.0f, 0.0f)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.AutoHeight()
		[
			SNew(SExpandableArea)
			.Padding(0)
			.InitiallyCollapsed(!this->DescriptionExpanded)
			.OnAreaExpansionChanged_Lambda([this](bool state) { this->DescriptionExpanded = state; })
			.HeaderContent() [SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont")).Text(FText::FromString("Session"))]
			.BodyContent() 
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot() 
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()[SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont")).Text(FText::FromString("Session-ID:"))]
						+SVerticalBox::Slot()[SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont")).Text(this, &STracerDataOverview::GetSessionId)]
					]
					+SHorizontalBox::Slot() 
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()[SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont")).Text(FText::FromString("Session-State:"))]
						+SVerticalBox::Slot()[SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont")).Text(this, &STracerDataOverview::GetSessionState)]
					]
					+SHorizontalBox::Slot() 
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()[SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont")).Text(FText::FromString("Session-Start:"))]
						+SVerticalBox::Slot()[SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont")).Text(this, &STracerDataOverview::GetSessionStart)]
					]
					+SHorizontalBox::Slot() 
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()[SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont")).Text(FText::FromString("Session-End:"))]
						+SVerticalBox::Slot()[SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont")).Text(this, &STracerDataOverview::GetSessionEnd)]
					]
				]
			]
			.Padding(8.0f)
		]

		// Controls
		+SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.AutoHeight()
		.MaxHeight(36.0)
		.Padding(0.0f, 4.0f, 0.0f, 4.0f)
		[
			SNew(SBorder)
			.Padding(FMargin(4.0f))
			[
				SNew(SHorizontalBox)

				+SHorizontalBox::Slot()
				.MaxWidth(32.0f)
				[
					SNew(SButton)
					.IsEnabled_Lambda([this]()
					{
						if (this->TracerData.IsValid() == false)
							return false;

						return this->TracerData.Pin()->HasTracedActor();
					})
					.OnClicked_Lambda([this]()
					{
						this->TracerData.Pin()->SelectTracedActor();
						return FReply::Handled();
					})
					.ForegroundColor(FSlateColor::UseForeground())
					[
						TSharedRef<SWidget>(SNew(SImage)
						.Image_Lambda([this]()
						{	
							return FStatsTracerEditorStyle::Get().GetBrush("TargetIcon");
						}))
					]
					.ToolTipText_Lambda([this]()
					{
						if(this->TracerData.IsValid() == false || this->TracerData.Pin()->HasTracedActor() == false)
							return FText::FromString("Lost traced object. Was it renamed or deleted?");

						return FText::FromString("Focus traced object");
					})
				]

				+SHorizontalBox::Slot()
				.MaxWidth(32.0f)
				[
					SNew(SButton)
					.IsEnabled_Lambda([this]()
					{
						if (this->TracerData.IsValid() == false)
							return false;

						return this->TracerData.Pin()->IsActive();
					})
					.OnClicked_Lambda([this]()
					{
						if (this->TracerData.IsValid() == false)
							return FReply::Handled();

						auto pinnedTracerData = this->TracerData.Pin();

						switch (pinnedTracerData->GetRepositoryState())
						{
							case StatsTracer::FTracerDataRepository::State::TRACING:
								pinnedTracerData->Pause();
								break;

							case StatsTracer::FTracerDataRepository::State::PAUSED:
								pinnedTracerData->Resume();
								break;
						}

						return FReply::Handled();
					})
					.ForegroundColor(FSlateColor::UseForeground())
					[
						TSharedRef<SWidget>(SNew(SImage)
						.ToolTipText_Lambda([this]() 
						{
							if (this->TracerData.Pin()->GetRepositoryState() == StatsTracer::FTracerDataRepository::State::PAUSED)
								return FText::FromString("Resume tracer");

							return FText::FromString("Pause tracer");		
						})
						.Image_Lambda([this]()
						{	
							if (this->TracerData.Pin()->GetRepositoryState() == StatsTracer::FTracerDataRepository::State::PAUSED)
								return FStatsTracerEditorStyle::Get().GetBrush("PlayIcon");

							return FStatsTracerEditorStyle::Get().GetBrush("PauseIcon");
						}))
					]	
				]

				+SHorizontalBox::Slot()
				.MaxWidth(32.0f)
				[
					SNew(SButton)
					.IsEnabled_Lambda([this]()
					{
						if (this->TracerData.IsValid() == false)
							return false;

						return this->TracerData.Pin()->IsActive();
					})
					.OnClicked_Lambda([this]()
					{
						if (this->TracerData.IsValid() == false)
							return FReply::Handled();

						this->TracerData.Pin()->Stop();
						return FReply::Handled();
					})
					.ForegroundColor(FSlateColor::UseForeground())
					[
						TSharedRef<SWidget>(SNew(SImage)
						.ToolTipText(FText::FromString("Stop tracer"))
						.Image(FStatsTracerEditorStyle::Get().GetBrush("StopIcon")))
					]
				]

				+SHorizontalBox::Slot()
				.MaxWidth(32.0f)
				[
					SNew(SButton)
					.IsEnabled_Lambda([this]()
					{
						if (this->TracerData.IsValid() == false)
							return false;

						return !this->TracerData.Pin()->IsActive();
					})
					.OnClicked_Lambda([this]()
					{
						if (this->TracerData.IsValid() == false)
							return FReply::Handled();
			
						auto pinnedTracerData = this->TracerData.Pin();

						// remove itself from session
						auto sessionHandle = pinnedTracerData->GetSession();
						if(sessionHandle.IsValid() == true)
							sessionHandle.Pin()->DeleteTracerRepository(pinnedTracerData->GetRepositoryId());

						// close tab
						if (this->ThisTab.IsValid() == true)
							STracerSessionOverview::CloseDataOverviewTab(this->ThisTab.Get());

						if (this->Parent.IsValid() == true)
							((STracerSessionOverview*)this->Parent.Pin().Get())->RefreshSessionTracerList();

						if (StatsTracer::TDRM != nullptr)
							StatsTracer::TDRM->UpdatePhysicalMemoryUsage();
						
						return FReply::Handled();
					})
					.ForegroundColor(FSlateColor::UseForeground())
					[
						TSharedRef<SWidget>(SNew(SImage)
						.ToolTipText(FText::FromString("Remove tracer from session"))
						.Image(FStatsTracerEditorStyle::Get().GetBrush("DeleteIcon")))
					]
				]
			]
		]

		// List view
		+SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SBorder)
			.Visibility_Lambda([&]()
			{
				if (this->TracerData.IsValid() && this->TracerData.Pin()->GetRepositoryData().Num() > 0)
					return EVisibility::Visible;
				
				return EVisibility::Hidden;
			})
			[
				SAssignNew(TracerDataListview, STracerDataListView)
				.ListItemsSource(&TracerDataItems)
				.OnGenerateRow(this, &STracerDataOverview::OnGenerateRowForTracerDataListView, this)
				.HeaderRow(
					SNew(SHeaderRow)
					+SHeaderRow::Column("Charts")
					.HeaderContent()
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.MaxHeight(24.0f)
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							.MaxWidth(24.0f)
							[
								SNew(SImage)
								.Image(FStatsTracerEditorStyle::Get().GetBrush("DataGraphIcon"))
							]
							+SHorizontalBox::Slot()
							.HAlign(HAlign_Left)
							.VAlign(VAlign_Center)
							.MaxWidth(50.0f)
							[
								SNew(STextBlock)
								.Font(FStatsTracerEditorStyle::Get().GetFontStyle("BigBoldFont"))
								.Text(FText::FromString(" Charts"))
							]

							+SHorizontalBox::Slot()
							.HAlign(HAlign_Left)
							.VAlign(VAlign_Center)
							.MaxWidth(24.0f)
							.Padding(4.0f, 0.0f, 0.0f, 0.0f)
							[
								SNew(SVerticalBox)
								+SVerticalBox::Slot()
								.Padding(0.0f, 1.0f)
								[
									SNew(SButton)
									.ToolTipText(FText::FromString("Collapse Charts"))
									.OnClicked_Lambda([&]()
									{
										for (auto it : this->TracerDataItems)
											it->IsExpanded = false;

										if (this->TracerDataListview.IsValid() == true)
											this->TracerDataListview->RebuildList();
									
										return FReply::Handled();
									})
									.ForegroundColor(FSlateColor::UseForeground())
									[
										TSharedRef<SWidget>(SNew(SImage)
										.Image(FStatsTracerEditorStyle::Get().GetBrush("FolderClosedIcon")))
									]
								]
							]

							+SHorizontalBox::Slot()
							.HAlign(HAlign_Left)
							.VAlign(VAlign_Center)
							.MaxWidth(24.0f)
							[
								SNew(SVerticalBox)
								+SVerticalBox::Slot()
								.Padding(0.0f, 1.0f)
								[
									SNew(SButton)
									.ToolTipText(FText::FromString("Expand Charts"))
									.OnClicked_Lambda([&]()
									{
										for (auto it : this->TracerDataItems)
											it->IsExpanded = true;

										if (this->TracerDataListview.IsValid() == true)
											this->TracerDataListview->RebuildList();
									
										return FReply::Handled();
									})
									.ForegroundColor(FSlateColor::UseForeground())
									[
										TSharedRef<SWidget>(SNew(SImage)
										.Image(FStatsTracerEditorStyle::Get().GetBrush("FolderOpenIcon")))
									]
								]
							]
						]
					]
					.FillWidth(1.0f)
				)
				.SelectionMode(ESelectionMode::None)
			]
		]

		// OPTIONAL NO DATA MESSAGE.
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(FStatsTracerEditorStyle::Get().GetFontStyle("LargeBoldFont"))
			.Text(FText::FromString("No data available."))
			.Visibility_Lambda([&]()
			{ 
				if (this->TracerData.IsValid() && this->TracerData.Pin()->GetRepositoryData().Num() > 0)
					return EVisibility::Hidden;
				 
				return EVisibility::Visible;
			})
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FText STracerDataOverview::GetSessionId() const
{
	if (this->TracerData.IsValid() == false || this->TracerData.Pin()->GetSession().IsValid() == false)
		return FText::FromString("Invalid Session!");

	return FText::FromString(FString::Printf(TEXT("%s (%u)"), *this->TracerData.Pin()->GetSession().Pin()->GetAllias(), this->TracerData.Pin()->GetSession().Pin()->GetSessionId()));
}

FText STracerDataOverview::GetSessionStart() const
{
	if (this->TracerData.IsValid() == false || this->TracerData.Pin()->GetSession().IsValid() == false)
		return FText::FromString("Invalid Session!");

	return FText::FromString(this->TracerData.Pin()->GetSession().Pin()->GetSessionStart().ToString());
}

FText STracerDataOverview::GetSessionEnd() const
{
	if (this->TracerData.IsValid() == false || this->TracerData.Pin()->GetSession().IsValid() == false)
		return FText::FromString("Invalid Session!");

	auto sessionHandle = this->TracerData.Pin()->GetSession().Pin();

	if(sessionHandle->GetSessionState() < StatsTracer::FTracerSession::State::STOPPED)
		return FText::FromString("-");

	return FText::FromString(sessionHandle->GetSessionEnd().ToString());
}

FText STracerDataOverview::GetSessionState() const
{
	if (this->TracerData.IsValid() == false || this->TracerData.Pin()->GetSession().IsValid() == false)
		return FText::FromString("Invalid Session!");

	StatsTracer::FTracerSession::State sessionState = this->TracerData.Pin()->GetSession().Pin()->GetSessionState();

	switch (sessionState)
	{
		case StatsTracer::FTracerSession::State::TRACING: return FText::FromString("TRACING");
		case StatsTracer::FTracerSession::State::PAUSED: return FText::FromString("PAUSED");
		case StatsTracer::FTracerSession::State::STOPPED: return FText::FromString("STOPPED");
		case StatsTracer::FTracerSession::State::COMPLETE: return FText::FromString("COMPLETE");
	}

	return FText::FromString("UNKNOWN");
}

TSharedRef<ITableRow> STracerDataOverview::OnGenerateRowForTracerDataListView(TTracerDataListItem InItem, const TSharedRef<STableViewBase>& OwnerTable, STracerDataOverview* InParentWidget)
{
	class STracerDataListItemWidget : public SMultiColumnTableRow<TTracerDataListItem>
	{
		void UpdateChartAreaWidth()
		{
			if (Chart.IsValid() == true)
			{	
				Chart->SetChartZoom(this->ZoomFactor);
				Chart->SetChartAreaWidth(this->GetCachedGeometry().GetLocalSize().X - 10.0f);
			}
		}

	private:

		STracerDataListView*						Listview;
		TTracerDataListItem							Item;

		TSharedPtr<SChartPlotResizer>				Resizier;
		TSharedPtr<STracerDataChart>				Chart;


		FGeometry									ThisGeometry;

		float										PendingListRefreshDelay;

		const float									MinItemHeight { 256.0f };
		const float									MaxItemHeight { 1024.0f };

		float										RequiredItemHeight;
		float										UserScaledItemHeight;
		
		const float									MIN_ZOOM_FACTOR { 1.0f };
		const float									MAX_ZOOM_FACTOR { 5.0f };

		float										ZoomFactor;

		TWeakPtr<StatsTracer::FTracerSession>		SessionHandle;

	public:

		SLATE_BEGIN_ARGS(STracerDataListItemWidget) :
			_UpdateFrequency(),
			_SessionHandle()
		{}

		SLATE_ARGUMENT(int32, UpdateFrequency);
		SLATE_ARGUMENT(TWeakPtr<StatsTracer::FTracerSession>, SessionHandle);
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TTracerDataListItem InListItem)
		{
			this->SessionHandle = InArgs._SessionHandle;

			Listview = static_cast<STracerDataListView*>(&InOwnerTable.Get());
			Item = InListItem;	

			UserScaledItemHeight = 0.0f;
			RequiredItemHeight = MinItemHeight;

			ZoomFactor = 1.0f;

			SMultiColumnTableRow<TTracerDataListItem>::Construct(FSuperRowType::FArguments(), InOwnerTable);
		}

		virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override
		{
			// hack: This creates a little bit of a delay until the triggered expandable area has resized,
			// right after that we need to call RequestListRefresh to make the scrollbar work
			if (this->PendingListRefreshDelay >= 0.0f)
			{
				this->PendingListRefreshDelay -= InDeltaTime;

				if (this->PendingListRefreshDelay <= 0.0f)
				{
					this->PendingListRefreshDelay = -1.0f;

					if(Listview != nullptr)
						Listview->RequestListRefresh();
				}
			}


			if (AllottedGeometry.GetLocalSize().X != ThisGeometry.GetLocalSize().X)
			{
				// host listitem width changed -> update chart area 
				UpdateChartAreaWidth();
			}

			// save last allotted geometry
			ThisGeometry = AllottedGeometry;
		}

		TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName)
		{
			return SNew(SBorder)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					[
						SNew(SExpandableArea)
						.Padding(0)
						.InitiallyCollapsed(!Item->IsExpanded)
						.OnAreaExpansionChanged_Lambda([this](bool state) 
						{ 
							if (PendingListRefreshDelay < 0.05f)
							{
								PendingListRefreshDelay = 0.1f;
								Item->IsExpanded = state;
							}
						})
						.HeaderContent()
						[
							SNew(STextBlock)
							.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont"))
							.Text_Lambda([this]()
							{
								if (Item->DataGroupName.IsEmpty() == true)
									return FText::FromString(TEXT("Default"));

								return FText::FromString(Item->DataGroupName);
							})
						]
						.BodyContent()
						[
							SNew(SBox)
							.MinDesiredHeight_Lambda([this]() 	
							{
								RequiredItemHeight = UserScaledItemHeight + FMath::Max<float>(MinItemHeight, Chart->GetMinDesiredHeight());
								RequiredItemHeight = FMath::Min<float>(MaxItemHeight, RequiredItemHeight);

								return RequiredItemHeight;
							})
							[
								SNew(SVerticalBox)
							
								// controls
								+SVerticalBox::Slot()
								.HAlign(HAlign_Right)
								.VAlign(VAlign_Center)
								.AutoHeight()
								.MaxHeight(24.0f)
								.Padding(0.0f, 4.0f)
								[
									SNew(SHorizontalBox)									
									+SHorizontalBox::Slot()
									.HAlign(HAlign_Fill)
									[
										SNew(SBox)
										.MinDesiredWidth(250.0f)
										[
											SNew(SHorizontalBox)
											+SHorizontalBox::Slot()
											.AutoWidth()
											[
												SNew(SImage)
												.ToolTipText(FText::FromString("Zoom Chart View"))
												.Image(FStatsTracerEditorStyle::Get().GetBrush("ZoomIcon"))
											]

											+SHorizontalBox::Slot()
											.HAlign(HAlign_Fill)
											.FillWidth(1.0f)
											[
												SNew(SSlider)
												.Value(0.0f)
												.Orientation(EOrientation::Orient_Horizontal)
												.OnValueChanged_Lambda([&](float NewValue)
												{
													this->ZoomFactor = FMath::Clamp<float>(1.0f + (NewValue * (MAX_ZOOM_FACTOR - 1.0f)), MIN_ZOOM_FACTOR, MAX_ZOOM_FACTOR);
													UpdateChartAreaWidth();
												})
											]

											+SHorizontalBox::Slot()
											.AutoWidth()
											.Padding(0.0f, 0.0f, 2.0f, 0.0f)
											[
												SNew(STextBlock)
												.Font(FStatsTracerEditorStyle::Get().GetFontStyle("BigBoldFont"))
												.Text_Lambda([this]()
												{
													return FText::FromString(FString::Printf(TEXT(" %.2fx"), this->ZoomFactor));
												})
											]
										]
									]
								]

								// Chart
								+SVerticalBox::Slot()
								.FillHeight(1.0f)
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									SAssignNew(Chart, STracerDataChart)
									.DataSourceArray(Item->DataSourceArray)
									.ChartAreaWidth(Listview->AsWidget()->GetCachedGeometry().GetLocalSize().X - 12.0f)
									.ZoomFactor(this->ZoomFactor)
									.SessionHandle(this->SessionHandle)
								]

								// Item Height Resizier
								+SVerticalBox::Slot()
								.AutoHeight()
								.MaxHeight(6.0f)
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									SAssignNew(Resizier, SChartPlotResizer)
									.OnResize_Lambda([&](float deltaSize)
									{
										UserScaledItemHeight = FMath::Max<float>(0.0f, UserScaledItemHeight + deltaSize);
										if (Listview != nullptr)
											Listview->RequestListRefresh();
									})
								]
							]
						]
					]
				];
		}
	};

	return
		SNew(STracerDataListItemWidget, OwnerTable, InItem)
		.SessionHandle(InParentWidget->GetSessionHandle());
}