///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Private\Widgets\STracerSessionOverview.cpp
///
/// Summary:	Implements the tracer session overview class.
///-------------------------------------------------------------------------------------------------

#include "STracerSessionOverview.h"
#include "StatsTracerEditorPCH.h"


STracerSessionOverview::TOpenTabArray STracerSessionOverview::OpenTabArray = STracerSessionOverview::TOpenTabArray();

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void STracerSessionOverview::Construct(const FArguments& InArgs)
{
	if (StatsTracer::TDRM == nullptr)
	{
		ChildSlot
		.Padding(FMargin(8.0f))
		[
			SNew(STextBlock)
			.Text(FText::FromString("TracerDataRepositoryManager handle is invalid!"))
		];

		return;
	}

	// register a timer that will refresh the tracer list every few milliseconds
	RegisterActiveTimer(5.0f, FWidgetActiveTimerDelegate::CreateLambda([&](double InCurrentTime, float InDeltaTime)
	{
		RefreshSessionItemArray();
		if (this->SelectedSessionItem.IsValid() == false && this->SessionItemArray.Num() > 0)
			OnSessionSelectionChanged(this->SessionItemArray[0], ESelectInfo::Direct);

		RefreshSessionTracerList();

		return EActiveTimerReturnType::Continue;
	}));


	// get a list of all stored sessions
	RefreshSessionItemArray();
	
	// select first session by default
	if (this->SessionItemArray.Num() > 0)
		OnSessionSelectionChanged(this->SessionItemArray[0], ESelectInfo::Direct);

	ChildSlot
	.Padding(FMargin(8.0f))
	[
		SNew(SVerticalBox)
		
		// Display current memory consumption of StatsTracer
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SStatsTracerMemoryLoadDisplay)
			.OnCleanMemoryDelegate_Lambda([&]()
			{
				// clear selected item
				OnSessionSelectionChanged(nullptr, ESelectInfo::Direct);

				// get all remaining open tab instances
				TArray<FString> temp;
				STracerSessionOverview::OpenTabArray.GetKeys(temp);

				for (int i = 0; i < temp.Num(); ++i)
				{	
					auto TabInstance = STracerSessionOverview::OpenTabArray[temp[i]];
					if(TabInstance.IsValid() == true)
						STracerSessionOverview::CloseDataOverviewTab(TabInstance.Pin().Get());
				}
				temp.Empty();
				STracerSessionOverview::OpenTabArray.Reset();
				this->SessionItemArray.Reset();
			})
		]

		+SVerticalBox::Slot()
		.Padding(0.0f, 8.0f, 0.0f, 0.0f)
		.AutoHeight()
		[	
			SNew(SBorder)
			.Padding(FMargin(4.0f))
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.MaxHeight(32.0)
				[

					// Header with controls and session selection + info
					SNew(SHorizontalBox)	 
					+SHorizontalBox::Slot()
					[
						SAssignNew(this->SessionSelection, SComboBox<TSessionComboboxItem>)
						.OptionsSource(&this->SessionItemArray)
						.InitiallySelectedItem(SelectedSessionItem)
						.OnSelectionChanged(this, &STracerSessionOverview::OnSessionSelectionChanged)
						.OnGenerateWidget_Lambda([](TSessionComboboxItem InOption)
						{
							if(InOption.IsValid() == false || InOption->SessionHandle.IsValid() == false)
							{
								return SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("BigBoldFont"))
									.Text(FText::FromString(FString::Printf(TEXT("INVALID SESSION"))));
							}

							return SNew(STextBlock)
								.Font(FStatsTracerEditorStyle::Get().GetFontStyle("BigBoldFont"))
								.Text(FText::FromString(FString::Printf(TEXT("%s"), *InOption->SessionHandle.Pin()->GetAllias())));
						})
						.OnComboBoxOpening_Lambda([&]() 
						{ 
							RefreshSessionItemArray();
						})
						[
							SNew(STextBlock)
							.Font(FStatsTracerEditorStyle::Get().GetFontStyle("BigBoldFont"))
							.Text_Lambda([&]() 
							{ 
								if (this->SessionItemArray.Num() == 0)
									return FText::FromString("No Session Available.");

								if (this->SelectedSessionItem.IsValid() == false || this->SelectedSessionItem->SessionHandle.IsValid() == false)
									OnSessionSelectionChanged(this->SessionItemArray[0], ESelectInfo::Direct);

								if (this->SelectedSessionItem.IsValid() == false || this->SelectedSessionItem->SessionHandle.IsValid() == false)
									return FText::FromString("INVALID SESSION");

								return FText::FromString(this->SelectedSessionItem->SessionHandle.Pin()->GetAllias()); 
							})
						]
					]

					+SHorizontalBox::Slot()
					.MaxWidth(32.0f)
					[
						SNew(SButton)
						.IsEnabled_Lambda([this]()
						{
							if (this->SelectedSessionItem.IsValid() == false || this->SelectedSessionItem->SessionHandle.IsValid() == false)
								return false;	

							return this->SelectedSessionItem->SessionHandle.Pin()->IsActiveSession();
						})
						.OnClicked_Lambda([this]()
						{
							if (this->SelectedSessionItem.IsValid() == false || this->SelectedSessionItem->SessionHandle.IsValid() == false)
								return FReply::Handled();

							auto pinnedSessionHandle = this->SelectedSessionItem->SessionHandle.Pin();
							switch (pinnedSessionHandle->GetSessionState())
							{
								case StatsTracer::FTracerSession::State::TRACING:
									pinnedSessionHandle->PauseSession();
									break;

								case StatsTracer::FTracerSession::State::PAUSED:
									pinnedSessionHandle->ResumeSession();
									break;
							}

							return FReply::Handled();
						})
						.ForegroundColor(FSlateColor::UseForeground())
						[
							TSharedRef<SWidget>(SNew(SImage)
							.Image_Lambda([this]()
							{	
								if (this->SelectedSessionItem.IsValid() == true && this->SelectedSessionItem->SessionHandle.IsValid() == true && this->SelectedSessionItem->SessionHandle.Pin()->GetSessionState() == StatsTracer::FTracerSession::State::PAUSED)
									return FStatsTracerEditorStyle::Get().GetBrush("PlayIcon");
						
								return FStatsTracerEditorStyle::Get().GetBrush("PauseIcon");
							})
							.ToolTipText_Lambda([this]()
							{
								if (this->SelectedSessionItem.IsValid() == true && this->SelectedSessionItem->SessionHandle.IsValid() == true && this->SelectedSessionItem->SessionHandle.Pin()->GetSessionState() == StatsTracer::FTracerSession::State::PAUSED)
									return FText::FromString("Resume session");

								return FText::FromString("Pause session");
							}))
						]			
					]


					+SHorizontalBox::Slot()
					.MaxWidth(32.0f)
					[
						SNew(SButton)
						.IsEnabled_Lambda([this]()
						{
							if (this->SelectedSessionItem.IsValid() == false || this->SelectedSessionItem->SessionHandle.IsValid() == false)
								return false;	

							return this->SelectedSessionItem->SessionHandle.Pin()->IsActiveSession();
						})
						.OnClicked_Lambda([this]()
						{
							if (this->SelectedSessionItem.IsValid() == false || this->SelectedSessionItem->SessionHandle.IsValid() == false)
								return FReply::Handled();

							this->SelectedSessionItem->SessionHandle.Pin()->StopSession();

							return FReply::Handled();
						})
						.ForegroundColor(FSlateColor::UseForeground())
						[
							TSharedRef<SWidget>(SNew(SImage)
							.ToolTipText(FText::FromString("Stop session"))
							.Image(FStatsTracerEditorStyle::Get().GetBrush("StopIcon")))
					
						]
					]

					+SHorizontalBox::Slot()
					.MaxWidth(32.0f)
					[
						SNew(SButton)
						.IsEnabled_Lambda([this]()
						{
							if (this->SelectedSessionItem.IsValid() == false || this->SelectedSessionItem->SessionHandle.IsValid() == false)
								return false;	

							return (this->SelectedSessionItem->SessionHandle.Pin()->GetSessionState() == StatsTracer::FTracerSession::COMPLETE);
						})
						.OnClicked_Lambda([this]()
						{
							if (this->SelectedSessionItem.IsValid() == false || this->SelectedSessionItem->SessionHandle.IsValid() == false)
								return FReply::Handled();

							// close all open tab instances for this session
							for (auto it : this->SessionTracers)
							{
								if (it.IsValid() == true && it->OpenTabInstance.IsValid() == true)
									STracerSessionOverview::CloseDataOverviewTab(it->OpenTabInstance.Pin().Get());
							}

							// remove session
							StatsTracer::TDRM->RemoveSession(this->SelectedSessionItem->SessionHandle.Pin()->GetSessionId());
					
							// remove from item array
							this->SessionItemArray.Remove(this->SelectedSessionItem);

							// refresh item array
							RefreshSessionItemArray();

							if (this->SessionItemArray.Num() > 0)
								OnSessionSelectionChanged(this->SessionItemArray[0], ESelectInfo::Direct);
							else
								OnSessionSelectionChanged(nullptr, ESelectInfo::Direct);

							return FReply::Handled();
						})
						.ForegroundColor(FSlateColor::UseForeground())
						[
							TSharedRef<SWidget>(SNew(SImage)
							.ToolTipText(FText::FromString("Remove session"))
							.Image(FStatsTracerEditorStyle::Get().GetBrush("DeleteIcon")))
						]
					]
				]

				// SESSION PROPERTIES
				+SVerticalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.AutoHeight()
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.Padding(0.0f, 8.0f)
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot() 
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()[SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont")).Text(FText::FromString("Session-ID:"))]
							+SVerticalBox::Slot()
							[
								SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont")).Text_Lambda([&]()
								{
									if (this->SelectedSessionItem.IsValid() == false || this->SelectedSessionItem->SessionHandle.IsValid() == false)
										return FText::FromString("-");

									auto pinnedSessionHandle = this->SelectedSessionItem->SessionHandle.Pin();
									return FText::FromString(FString::Printf(TEXT("%s (%u)"), *pinnedSessionHandle->GetAllias(), pinnedSessionHandle->GetSessionId()));
								})
							]
						]
						+SHorizontalBox::Slot() 
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()[SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont")).Text(FText::FromString("Session-State:"))]
							+SVerticalBox::Slot()
							[
								SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont")).Text_Lambda([&]()
								{
									if (this->SelectedSessionItem.IsValid() == false || this->SelectedSessionItem->SessionHandle.IsValid() == false)
										return FText::FromString("-");

									switch (this->SelectedSessionItem->SessionHandle.Pin()->GetSessionState())
									{
										case StatsTracer::FTracerSession::State::TRACING: return FText::FromString("TRACING");
										case StatsTracer::FTracerSession::State::PAUSED: return FText::FromString("PAUSED");
										case StatsTracer::FTracerSession::State::STOPPED: return FText::FromString("STOPPED");
										case StatsTracer::FTracerSession::State::COMPLETE: return FText::FromString("COMPLETE");
									}

									return FText::FromString("UNKNOWN");
								})
							]
						]

						+SHorizontalBox::Slot() 
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()[SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont")).Text(FText::FromString("Session-Start:"))]
							+SVerticalBox::Slot()
							[
								SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont")).Text_Lambda([&]()
								{
									if (this->SelectedSessionItem.IsValid() == false || this->SelectedSessionItem->SessionHandle.IsValid() == false)
										return FText::FromString("-");

									return FText::FromString(this->SelectedSessionItem->SessionHandle.Pin()->GetSessionStart().ToString());
								})
							]
						]

						+SHorizontalBox::Slot() 
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()[SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont")).Text(FText::FromString("Session-End:"))]
							+SVerticalBox::Slot()
							[
								SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont")).Text_Lambda([&]()
								{
									if (this->SelectedSessionItem.IsValid() == false || this->SelectedSessionItem->SessionHandle.IsValid() == false || (this->SelectedSessionItem->SessionHandle.Pin()->GetSessionState() < StatsTracer::FTracerSession::State::STOPPED))
										return FText::FromString("-");

									return FText::FromString(this->SelectedSessionItem->SessionHandle.Pin()->GetSessionEnd().ToString());
								})
							]
						]
					]
				]
			]
		]


		// TRACER LISTVIEW
		+SVerticalBox::Slot()
		.Padding(0.0f, 8.0f, 0.0f, 0.0f)
		.FillHeight(1.0f)
		[
			SNew(SBorder)
			[
				SAssignNew(TracerList, STracerListView)
				.ListItemsSource(&SessionTracers)
				.OnGenerateRow_Lambda([this](TTracerListItem InItem, const TSharedRef<STableViewBase>& OwnerTable)
				{
					return this->OnGenerateRowForTracerListView(InItem, OwnerTable, this);
				})
				.OnMouseButtonDoubleClick(this, &STracerSessionOverview::SpawnTracerOverviewTab)
				.HeaderRow(
					SNew(SHeaderRow)
					+SHeaderRow::Column("Tracer")
					.HeaderContent()
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.MaxHeight(24.0f)
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							.MaxWidth(24.0f)
							.Padding(0.0f, 0.0f, 2.0f, 0.0f)
							[
								SNew(SBox)
								.WidthOverride(24.0f)
								.HeightOverride(24.0f)
								[
									SNew(SImage).Image(FStatsTracerEditorStyle::Get().GetBrush("TracerIcon.White"))
								]
							]
							+SHorizontalBox::Slot()
							.HAlign(HAlign_Left)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Font(FStatsTracerEditorStyle::Get().GetFontStyle("BigBoldFont"))
								.Text(FText::FromString("Tracer"))
							]
						]
					]
					.FillWidth(1.0f)
				)
				.SelectionMode(ESelectionMode::Single)
			]
		]
	];
	
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void STracerSessionOverview::RefreshSessionTracerList()
{
	this->SessionTracers.Reset();

	if (this->SelectedSessionItem.IsValid() == true && this->SelectedSessionItem->SessionHandle.IsValid() == true)
	{
		auto pinnedSessionHandle = this->SelectedSessionItem->SessionHandle.Pin();

		if (pinnedSessionHandle->GetTracerDataRepositoryMap().IsValid() == true)
		{
			StatsTracer::TTracerDataRepositoryMapHandle TDR = pinnedSessionHandle->GetTracerDataRepositoryMap().Pin();

			TArray<StatsTracer::TTracerDataRepositoryHandle> temp;
			TDR->GenerateValueArray(temp);

			for (auto it : temp)
			{
				if (it.IsValid() == false)
					continue;

				this->SessionTracers.Add(TTracerListItem(new FTracerListItem(it, STracerSessionOverview::FindOpenTabInstance(STracerSessionOverview::GetOpenTabArrayKey(it.Get())))));
			}
		}
	}

	if (TracerList.IsValid() == true)
		TracerList->RequestListRefresh();
}

TWeakPtr<SDockTab> STracerSessionOverview::OpenDataOverviewTab(TSharedPtr<SWidget> InParent, StatsTracer::TWeakTracerDataRepositoryHandle InDataRepositoryHandle)
{
	TSharedPtr<SDockTab>			TabInstace;
	TSharedPtr<STracerDataOverview> TracerDataOverviewWidget;


	if (InDataRepositoryHandle.IsValid() == false)
	{
		FGlobalTabmanager::Get()->InsertNewDocumentTab(
			StatsTracerEditorMajorTabName,
			FTabManager::ESearchPreference::PreferLiveTab,
			SAssignNew(TabInstace, SDockTab)
			.Label(FText::FromString("Invalid Pointer"))
			.OnTabClosed_Lambda([&](TSharedRef<SDockTab> tab)
			{		
				STracerSessionOverview::CloseDataOverviewTab(tab->GetLayoutIdentifier());
			})
			.TabRole(ETabRole::NomadTab) [ SNew(STextBlock).Text(FText::FromString("Invalid Pointer.")) ]
		);

		return TabInstace;
	}


	auto pinnedDataRepositoryHandle = InDataRepositoryHandle.Pin();

	const FString tabKey = STracerSessionOverview::GetOpenTabArrayKey(pinnedDataRepositoryHandle.Get());

	auto OpenTabInstance = STracerSessionOverview::FindOpenTabInstance(tabKey);
	if (OpenTabInstance.IsValid() == true)
	{
		OpenTabInstance.Pin()->DrawAttention();
		return OpenTabInstance;
	}

	

	FGlobalTabmanager::Get()->InsertNewDocumentTab(
		StatsTracerEditorMajorTabName,
		FTabManager::ESearchPreference::PreferLiveTab,
		SAssignNew(TabInstace, SDockTab)
		.Label(FText::FromString(pinnedDataRepositoryHandle->GetRepositoryName()))
		.OnTabClosed_Lambda([&](TSharedRef<SDockTab> tab)
		{		
			STracerSessionOverview::CloseDataOverviewTab(tab->GetLayoutIdentifier());
		})
		.TabRole(ETabRole::NomadTab)
		[
			SAssignNew(TracerDataOverviewWidget, STracerDataOverview)
			.Parent(InParent->AsShared())
			.TracerData(InDataRepositoryHandle)
		]
	);

	// pass the DockTab to the inner widget
	if (TracerDataOverviewWidget.IsValid() == true && TabInstace.IsValid() == true)
		TracerDataOverviewWidget->ThisTab = TabInstace;

	// store tab instance
	for (auto KVP : STracerSessionOverview::OpenTabArray)
	{
		if (KVP.Key.Equals(tabKey))
		{
			KVP.Value = TabInstace;
			return TabInstace;
		}
	}

	STracerSessionOverview::OpenTabArray.Add(tabKey, TabInstace);
	return TabInstace;
}

void STracerSessionOverview::CloseDataOverviewTab(const FTabId& InTabId)
{
	for (auto KVP : STracerSessionOverview::OpenTabArray)
	{
		if (KVP.Value.IsValid() == true && KVP.Value.Pin()->GetLayoutIdentifier() == InTabId)
		{
			STracerSessionOverview::CloseDataOverviewTab(KVP.Value.Pin().Get(), false);
			return;
		}
	}
}

void STracerSessionOverview::CloseDataOverviewTab(SDockTab* InTabInstace, bool ExecClose)
{
	if (InTabInstace != nullptr)
	{
		// close tab
		if(ExecClose == true)
			InTabInstace->RequestCloseTab();

		// remove it from array
		for (auto KVP : STracerSessionOverview::OpenTabArray)
		{
			if (KVP.Value.IsValid() == true && KVP.Value.HasSameObject(InTabInstace) == true)
			{
				STracerSessionOverview::OpenTabArray.Remove(KVP.Key);
				return;
			}
		}
	}
}

TWeakPtr<SDockTab> STracerSessionOverview::FindOpenTabInstance(const FString& InTabKey)
{
	for (auto KVP : STracerSessionOverview::OpenTabArray)
	{
		if (KVP.Key.Equals(InTabKey))
			return KVP.Value; // done
	}

	return TWeakPtr<SDockTab>(nullptr);
}

FString STracerSessionOverview::GetOpenTabArrayKey(const StatsTracer::FTracerDataRepository* InDataRepository)
{
	uint32 repoId = InDataRepository->GetRepositoryId();
	uint32 sessId = InDataRepository->GetSession().IsValid() ? InDataRepository->GetSession().Pin()->GetSessionId() : 0;

	return FString::Printf(TEXT("%d.%d"), sessId, repoId);
}


StatsTracer::TWeakTracerSessionHandle STracerSessionOverview::GetSessionHandle()
{
	if (this->SelectedSessionItem.IsValid() == false || this->SelectedSessionItem->SessionHandle.IsValid() == false)
		return false;

	return this->SelectedSessionItem->SessionHandle;
}

void STracerSessionOverview::RefreshSessionItemArray()
{
	for (auto it : StatsTracer::TDRM->GetSessionArray())
	{
		bool bContains = this->SessionItemArray.ContainsByPredicate([&](TSessionComboboxItem& other)
		{
			if (other.IsValid() == false || other->SessionHandle.IsValid() == false)
				return false;

			if (it.IsValid() == false)
				return true; // should not happen, but just to make sure not to polute list with invlaid pointer

			return it.Pin()->GetSessionId() == other->SessionHandle.Pin()->GetSessionId();
		});

		if(bContains == false)
			this->SessionItemArray.Add(TSessionComboboxItem(new FSessionComboboxItem(it)));
	}

	if(this->SessionSelection.IsValid())
		this->SessionSelection->RefreshOptions();
}

void STracerSessionOverview::OnSessionSelectionChanged(TSessionComboboxItem NewValue, ESelectInfo::Type type)
{
	this->SelectedSessionItem.Reset();
	this->SelectedSessionItem = NewValue;

	this->RefreshSessionTracerList();
} 

FText STracerSessionOverview::GetSelectedSessionName() const
{
	return this->SelectedSessionItem.IsValid() && this->SelectedSessionItem->SessionHandle.IsValid() == true ? FText::FromString(FString::Printf(TEXT("%s"), *this->SelectedSessionItem->SessionHandle.Pin()->GetAllias())) : FText::FromString("No sessions available");
}

TSharedRef<ITableRow> STracerSessionOverview::OnGenerateRowForTracerListView(TTracerListItem InItem, const TSharedRef<STableViewBase>& OwnerTable, STracerSessionOverview* sessionOverview)
{
	class STracerListItemWidget : public SMultiColumnTableRow<TTracerListItem>
	{
	private:

		STracerSessionOverview* SessionOverviewWidget;

		TTracerListItem Item;

		FReply SpawnTracerOverviewTab(TTracerListItem InItem)
		{
			// store tab instance for this list item
			InItem->OpenTabInstance = STracerSessionOverview::OpenDataOverviewTab(SessionOverviewWidget->AsShared(), InItem->ReposityDataHandle);
			return FReply::Handled();
		}

	public:

		SLATE_BEGIN_ARGS(STracerListItemWidget) :
			_SessionOverviewWidget()
		{
		}
		SLATE_ARGUMENT(STracerSessionOverview*, SessionOverviewWidget);
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TTracerListItem InListItem)
		{
			SessionOverviewWidget = InArgs._SessionOverviewWidget;
			Item = InListItem;

			SMultiColumnTableRow<TTracerListItem>::Construct(FSuperRowType::FArguments(), InOwnerTable);
		}	

		TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName)
		{			
			return 
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				[
					SNew(SHorizontalBox)

					// state icon
					+SHorizontalBox::Slot()
					.Padding(2.0f, 0.0f, 0.0f, 0.0f)
					.AutoWidth()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.MaxHeight(24.0f)
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							.MaxWidth(24.0f)
							[
								SNew(SBox)
								.HeightOverride(24.0f)
								.WidthOverride(24.0f)
								[
									SNew(SImage)
									.ToolTipText_Lambda([this]()
									{
										if (Item.IsValid() == false)
											return FText::FromString("Invalid tracer state!");

										switch (Item->ReposityDataHandle.Pin()->GetRepositoryState())
										{
											case StatsTracer::FTracerDataRepository::TRACING:
												return FText::FromString("Tracing");

											case StatsTracer::FTracerDataRepository::PAUSED:
												return FText::FromString("Paused");

											case StatsTracer::FTracerDataRepository::STOPPED:
												return FText::FromString("Stopped");

											case StatsTracer::FTracerDataRepository::COMPLETE:
												return FText::FromString("Completed");
										}

										return FText::FromString("Invalid tracer state!");
									})
									.Image_Lambda([this]()
									{
										if (Item.IsValid() == false)
											return FStatsTracerEditorStyle::Get().GetBrush("TracerIcon.White");

										switch (Item->ReposityDataHandle.Pin()->GetRepositoryState())
										{
											case StatsTracer::FTracerDataRepository::TRACING:
												return FStatsTracerEditorStyle::Get().GetBrush("TracerIcon.Green");

											case StatsTracer::FTracerDataRepository::PAUSED:
												return FStatsTracerEditorStyle::Get().GetBrush("TracerIcon.YellowGreen");

											case StatsTracer::FTracerDataRepository::STOPPED:
												return FStatsTracerEditorStyle::Get().GetBrush("TracerIcon.Red");

											case StatsTracer::FTracerDataRepository::COMPLETE:
												return FStatsTracerEditorStyle::Get().GetBrush("TracerIcon.Blue");
										}

										return FStatsTracerEditorStyle::Get().GetBrush("TracerIcon.White");
									})
								]
							]
						]
					]

					// Label
					+SHorizontalBox::Slot()	
					.FillWidth(1.0f)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.Padding(4.0f)
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.FillWidth(1.0f)
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							[
								SNew(STextBlock)
								.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont"))
								.Text(FText::FromString(Item->ReposityDataHandle.Pin()->GetRepositoryName()))
							]

							+SVerticalBox::Slot()
							[
								SNew(STextBlock)
								.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalItalicFont"))
								.Text(Item->ReposityDataHandle.Pin()->GetRepositoryDescription())
							]
						]

						// controls
						+SHorizontalBox::Slot()	
						.AutoWidth()
						.HAlign(HAlign_Right)
						.VAlign(VAlign_Center)
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.MaxHeight(28.0f)
							[
								SNew(SHorizontalBox)

								+SHorizontalBox::Slot()
								.MaxWidth(28.0f)
								[
									SNew(SButton)
									.ToolTipText(FText::FromString("Show tracer data"))
									.OnClicked(FOnClicked::CreateSP(this, &STracerListItemWidget::SpawnTracerOverviewTab, Item))
									.ForegroundColor(FSlateColor::UseForeground())
									[
										TSharedRef<SWidget>(SNew(SImage)
										.Image(FStatsTracerEditorStyle::Get().GetBrush("ShowDataIcon")))
									]
								]			

								+SHorizontalBox::Slot()
								.MaxWidth(28.0f)
								[
									SNew(SButton)
									.IsEnabled_Lambda([this]()
									{
										if (this->Item.IsValid() == false || this->Item->ReposityDataHandle.IsValid() == false)
											return false;

										return this->Item->ReposityDataHandle.Pin()->HasTracedActor();
									})
									.OnClicked_Lambda([this]()
									{
										if(this->Item->ReposityDataHandle.IsValid() == true)
											this->Item->ReposityDataHandle.Pin()->SelectTracedActor();

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
										if(this->Item.IsValid() == false || this->Item->ReposityDataHandle.IsValid() == false || this->Item->ReposityDataHandle.Pin()->HasTracedActor() == false)
											return FText::FromString("Lost traced object. Was it renamed or deleted?");

										return FText::FromString("Focus traced object");
									})
								]

								+SHorizontalBox::Slot()
								.MaxWidth(28.0f)
								[
									SNew(SButton)
									.IsEnabled_Lambda([this]()
									{
										if (this->Item.IsValid() == false || this->Item->ReposityDataHandle.IsValid() == false)
											return false;

										return this->Item->ReposityDataHandle.Pin()->IsActive();
									})
									.OnClicked_Lambda([this]()
									{
										if (this->Item.IsValid() == false || this->Item->ReposityDataHandle.IsValid() == false)
											return FReply::Handled();

										switch (this->Item->ReposityDataHandle.Pin()->GetRepositoryState())
										{
											case StatsTracer::FTracerDataRepository::State::TRACING:
												this->Item->ReposityDataHandle.Pin()->Pause();
												break;

											case StatsTracer::FTracerDataRepository::State::PAUSED:
												this->Item->ReposityDataHandle.Pin()->Resume();
												break;
										}

										return FReply::Handled();
									})
									.ForegroundColor(FSlateColor::UseForeground())
									[
										TSharedRef<SWidget>(SNew(SImage)
										.Image_Lambda([this]()
										{	
											if (this->Item->ReposityDataHandle.Pin()->GetRepositoryState() == StatsTracer::FTracerDataRepository::State::PAUSED)
												return FStatsTracerEditorStyle::Get().GetBrush("PlayIcon");

											return FStatsTracerEditorStyle::Get().GetBrush("PauseIcon");
										}))
									]
									.ToolTipText_Lambda([this]()
									{
										if (this->Item->ReposityDataHandle.Pin()->GetRepositoryState() == StatsTracer::FTracerDataRepository::State::PAUSED)
											return FText::FromString("Resume tracer");

										return FText::FromString("Pause tracer");
									})
								]

								+SHorizontalBox::Slot()
								.MaxWidth(28.0f)
								[
									SNew(SButton)
									.ToolTipText(FText::FromString("Stop tracer"))
									.IsEnabled_Lambda([this]()
									{
										if (this->Item.IsValid() == false || this->Item->ReposityDataHandle.IsValid() == false)
											return false;

										return this->Item->ReposityDataHandle.Pin()->IsActive();
									})
									.OnClicked_Lambda([this]()
									{
										if (this->Item.IsValid() == false || this->Item->ReposityDataHandle.IsValid() == false)
											return FReply::Handled();

										this->Item->ReposityDataHandle.Pin()->Stop();
										return FReply::Handled();
									})
									.ForegroundColor(FSlateColor::UseForeground())
									[
										TSharedRef<SWidget>(SNew(SImage)
										.Image(FStatsTracerEditorStyle::Get().GetBrush("StopIcon")))
									]
									
								]

								+SHorizontalBox::Slot()
								.MaxWidth(28.0f)
								[
									SNew(SButton)
									.ToolTipText(FText::FromString("Remove tracer from session"))
									.IsEnabled_Lambda([this]()
									{
										auto sessionHandle = this->SessionOverviewWidget->GetSessionHandle();
										if (sessionHandle.IsValid() == false)
											return false;	

										return (sessionHandle.Pin()->GetSessionState() >= StatsTracer::FTracerSession::STOPPED);
									})
									.IsEnabled_Lambda([this]()
									{
										if (this->Item.IsValid() == false || this->Item->ReposityDataHandle.IsValid() == false)
											return false;

										return this->Item->ReposityDataHandle.Pin()->IsActive() == false;
									})
									.OnClicked_Lambda([this]()
									{
										if (this->Item.IsValid() == false)
											return FReply::Handled();

										// close possible open tab
										if (this->Item->OpenTabInstance.IsValid() == true)
										{
											STracerSessionOverview::CloseDataOverviewTab(this->Item->OpenTabInstance.Pin().Get());
										}

										if (this->SessionOverviewWidget != nullptr)
										{
											auto sessionHandle = this->SessionOverviewWidget->GetSessionHandle();
											if (sessionHandle.IsValid() == true && Item->ReposityDataHandle.IsValid() == true)
											{
												auto pinnedTracerData = Item->ReposityDataHandle.Pin();

												sessionHandle.Pin()->DeleteTracerRepository(pinnedTracerData->GetRepositoryId());
												

												this->SessionOverviewWidget->RefreshSessionTracerList();
											}
										}

										if (StatsTracer::TDRM != nullptr)
											StatsTracer::TDRM->UpdatePhysicalMemoryUsage();

										return FReply::Handled();
									})
									.ForegroundColor(FSlateColor::UseForeground())
									[
										TSharedRef<SWidget>(SNew(SImage)
										.Image(FStatsTracerEditorStyle::Get().GetBrush("DeleteIcon")))
									]
									
								]
							]
						]
					]
				];
		}
	};

	return SNew(STracerListItemWidget, OwnerTable, InItem).SessionOverviewWidget(sessionOverview);
}

void STracerSessionOverview::SpawnTracerOverviewTab(TTracerListItem Item)
{
	Item->OpenTabInstance = STracerSessionOverview::OpenDataOverviewTab(this->AsShared(), Item->ReposityDataHandle);
}