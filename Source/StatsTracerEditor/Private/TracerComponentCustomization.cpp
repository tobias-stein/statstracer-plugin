///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Private\TracerComponentCustomization.cpp
///
/// Summary:	Implements the tracer component customization class.
///-------------------------------------------------------------------------------------------------

#include "TracerComponentCustomization.h"
#include "StatsTracerEditorPCH.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"

#include "PropertyHandle.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void STracerPropertyListUI::Construct(const FArguments& InArgs)
{
	#define LOCTEXT_NAMESPACE "TracerPropertyListUI"

	// save reference to the UTracerComponent
	this->m_TracerComp = InArgs._TracerComp;

	ChildSlot
		[
			SNew(SBox)
			.HeightOverride(400.0f)
			[
				SAssignNew(this->m_PropertyOwnerListView, STracerStatOwnerArrayListView)
				.ItemHeight(32)
				.SelectionMode(ESelectionMode::None)
				.ListItemsSource(&this->m_ListItems)
				.OnGenerateRow(this, &STracerPropertyListUI::OnGenerateRowForPropertyOwnerListView, this)
				.HeaderRow
				(
					SNew(SHeaderRow)
					+SHeaderRow::Column("Stats")
					.VAlignHeader(VAlign_Center)
					.HeaderContent()
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.MaxHeight(26.0f)
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.MaxWidth(24.0f)
								.Padding(2.0f, 0.0f, 2.0f, 0.0f)
								[
									SNew(SImage)
									.Image(FStatsTracerEditorStyle::Get().GetBrush("StatsIcon"))
								]

								+SHorizontalBox::Slot()
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("BigBoldFont"))
									.ToolTipText(FText::FromString("Traced Stats / Total Stats"))
									.Text_Lambda([&]() 
									{
										return FText::FromString(FString::Printf(TEXT("[%d/%d]%s"), this->m_TracedStatsNum, this->m_TotalStatsNum, GetFilterString().IsEmpty() ? TEXT("") : TEXT("*")));
									})
								]
							]

							+SHorizontalBox::Slot()
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.HAlign(HAlign_Right)
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot()
									.MaxWidth(24.0f)
									[
										SNew(SButton)
										.ToolTipText(FText::FromString("Refresh Stats List"))
										.OnClicked_Lambda([&]()
										{
											if (this->m_TracerComp != nullptr && this->m_TracerComp.IsValid() == true)
												this->m_TracerComp->ScanStats();

											this->UpdateListview();

											return FReply::Handled();
										})


										.ForegroundColor(FSlateColor::UseForeground())
										[
											TSharedRef<SWidget>(SNew(SImage)
											.Image(FStatsTracerEditorStyle::Get().GetBrush("RefreshIcon")))
										]
									]
								]

								+SHorizontalBox::Slot()
								.Padding(2.0f, 0.0f, -2.0f, 0.0f)
								[
									SAssignNew(this->m_ListViewFilter, SSearchBox)
									.OnTextChanged_Lambda([this](const FText& InFilterText) { UpdateListview(); })
								]
							]
						]
					]
				)
			]
		];

	// update list
	this->UpdateListview();

	#undef LOCTEXT_NAMESPACE
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FString STracerPropertyListUI::GetFilterString() const
{
	if (m_ListViewFilter.IsValid() == false)
		return FString("");

	return m_ListViewFilter->GetText().ToString();
}

void STracerPropertyListUI::UpdateListview()
{
	this->m_ListItems.Reset();
	
	this->m_TotalStatsNum = 0;
	this->m_TracedStatsNum = 0;

	const FString filter = GetFilterString();

	FString outterName;
	FString properName;
	FString fullName;

	if (this->m_TracerComp != nullptr && this->m_TracerComp.IsValid() == true)
	{		
		for (auto KVP : this->m_TracerComp->DetectedStats)
		{

			int32 FilteredStatsNum = KVP.Value.Num();

			for (auto S : KVP.Value.StatsArray)
			{
				if (filter.IsEmpty() == true)
				{
					this->m_TotalStatsNum++;
					if (S.IsTraced == true)
						this->m_TracedStatsNum++;
				}
				else
				{
					outterName = S.OutterName;
					properName = S.PropertyName;
					fullName = outterName.AppendChar('.').Append(properName);
					fullName.TrimStartAndEndInline();

					if (fullName.Contains(filter) == true)
					{
						this->m_TotalStatsNum++;
						if (S.IsTraced == true)
							this->m_TracedStatsNum++;
					}
					else
					{
						FilteredStatsNum--;
					}
				}
			}

			// only add item, iff it has items remain after filtering
			if(filter.IsEmpty() || FilteredStatsNum > 0)
				this->m_ListItems.Add(TTracerStatItem(new FTracerStatListItem(this->m_TracerComp.Get(), KVP.Value.OwnerType, KVP.Key, -1)));
		}
	}

	RefreshListview();
}

TSharedRef<ITableRow> STracerPropertyListUI::OnGenerateRowForPropertyOwnerListView(TTracerStatItem InItem, const TSharedRef<STableViewBase>& OwnerTable, STracerPropertyListUI* InParent)
{
	class STracerPropertyListItemWidget : public SMultiColumnTableRow<TTracerStatItem>
	{
	private:

		STracerPropertyListUI*						MyUiParent;

		STracerStatOwnerArrayListView*				View;
		TSharedPtr<FTracerStatListItem>				Item;

		float										PendingListRefreshDelay;


		// the item array
		TTracerStatItemArray						ChildListItems;

		// the list view
		TSharedPtr<STracerStatItemListView>			ChildListView;


	public:

		SLATE_BEGIN_ARGS(STracerPropertyListItemWidget) :
			_MyUiParent()
		{}
		SLATE_ARGUMENT(STracerPropertyListUI*, MyUiParent);
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TSharedPtr<FTracerStatListItem> InListItem)
		{
			MyUiParent = InArgs._MyUiParent;

			View = static_cast<STracerStatOwnerArrayListView*>(&InOwnerTable.Get());
			Item = InListItem;

			RefreshListview(MyUiParent != nullptr ? MyUiParent->GetFilterString() : "");

			SMultiColumnTableRow<TSharedPtr<FTracerStatListItem>>::Construct(FSuperRowType::FArguments(), InOwnerTable);
		}

		void RefreshListview(const FString& filter)
		{
			ChildListItems.Reset();

			if (Item.IsValid() == false || Item->TracerComp == nullptr)
				return;

			auto stats = Item->TracerComp->DetectedStats.Find(Item->StatKey);
			if (stats == nullptr)
				return;
			
			for (int32 i = 0; i < stats->Num(); ++i)
			{
				FString outterName = (*stats)[i].OutterName;
				FString properName = (*stats)[i].PropertyName;
				FString fullName = outterName.AppendChar('.').Append(properName);
				fullName.TrimStartAndEndInline();

				if ((!filter.IsEmpty() && fullName.Contains(filter)) || filter.IsEmpty())
					ChildListItems.Add(TTracerStatItem(new FTracerStatListItem(Item->TracerComp, stats->OwnerType, Item->StatKey, i)));
			}
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

					if (View != nullptr)
						View->RequestListRefresh();

					if (MyUiParent != nullptr)
						MyUiParent->RefreshListview();
				}
			}
		}

		TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName)
		{
			return

				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.MaxHeight(400.0f)
				[
					SNew(SBorder)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[			
						SNew(SExpandableArea)
						.InitiallyCollapsed(this->Item->IsExpanded == false)
						.OnAreaExpansionChanged_Lambda([this](bool state)
						{
							if (PendingListRefreshDelay < 0.05f)
							{
								this->PendingListRefreshDelay = 0.25f;
								this->Item->IsExpanded = state;
							}
						})
						.HeaderContent() 
						[ 
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							.HAlign(HAlign_Left)
							.FillWidth(1.0f)
							[
								SNew(STextBlock)
								.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont"))
								.Text_Lambda([this]()
								{
									if(Item.IsValid() == false)
										return FText::FromString(TEXT("INVALID"));

									return FText::FromName(Item->StatKey);
								})
							]
						]	
						.BodyContent()
						[
							SNew(SBox)
							.HeightOverride(300.0f)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							[
								SAssignNew(this->ChildListView, STracerStatItemListView)
								.ListItemsSource(&this->ChildListItems)
								.SelectionMode(ESelectionMode::None)
								.OnGenerateRow(this, &STracerPropertyListItemWidget::OnGenerateRowForPropertyListView, MyUiParent)
								.HeaderRow
								(
									SNew(SHeaderRow)
									+SHeaderRow::Column("Stats")
									.VAlignHeader(VAlign_Center)
									.HeaderContent()
									[
										SNew(SVerticalBox)
										+SVerticalBox::Slot()
										.Padding(10.0f, 2.0f)
										[
											SNew(SHorizontalBox)
											+SHorizontalBox::Slot()
											.VAlign(VAlign_Center)
											.HAlign(HAlign_Left)
											.AutoWidth()
											[
												SNew(STextBlock)
												.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
												.Text(FText::FromString(FString::Printf(TEXT("(%s)"), *Item->OwnerType.ToString())))
											]
										]
									]
								)
							]
						]
					]
				];
		}	

		TSharedRef<ITableRow> OnGenerateRowForPropertyListView(TTracerStatItem InItem, const TSharedRef<STableViewBase>& OwnerTable, STracerPropertyListUI* InParent)
		{
			class STracerPropertyListItemWidgetInner : public SMultiColumnTableRow<TTracerStatItem>
			{
			private:

				STracerPropertyListUI*				MyUiParent;

				STracerStatItemListView*			View;
				TSharedPtr<FTracerStatListItem>		Item;

				float								PendingListRefreshDelay;

			public:

				SLATE_BEGIN_ARGS(STracerPropertyListItemWidgetInner) :
					_MyUiParent()
				{}
				SLATE_ARGUMENT(STracerPropertyListUI*, MyUiParent);
				SLATE_END_ARGS()

				void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TSharedPtr<FTracerStatListItem> InListItem)
				{
					MyUiParent = InArgs._MyUiParent;

					View = static_cast<STracerStatOwnerArrayListView*>(&InOwnerTable.Get());
					Item = InListItem;

					SMultiColumnTableRow<TSharedPtr<FTracerStatListItem>>::Construct(FSuperRowType::FArguments(), InOwnerTable);
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

							if (View != nullptr)
								View->RequestListRefresh();
						}
					}
				}

				TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName)
				{
					return

						SNew(SBorder)
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							.Padding(2.0f, 2.0f, 2.0f, 4.0f)
							.AutoHeight()
							.MaxHeight(24.0f)
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.HAlign(HAlign_Left)
								.AutoWidth()
								[
									SNew(SCheckBox)
									.HAlign(HAlign_Center)
									.OnCheckStateChanged_Lambda([&](ECheckBoxState state) 
									{ 
										if(state == ECheckBoxState::Checked)
										{
											Item->GetStat().IsTraced = true; 
											if (MyUiParent != nullptr)
												MyUiParent->IncrementTraced();
										}
										else
										{
											Item->GetStat().IsTraced = false;
											if (MyUiParent != nullptr)
												MyUiParent->DecrementTraced();
										}
									})
									.IsChecked(Item->GetStat().IsTraced == true ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
								]

								+SHorizontalBox::Slot()
								.HAlign(HAlign_Left)
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont"))
									.Text_Lambda([this]()
									{
										return FText::FromString(Item->GetStat().PropertyName);
									})
									.ToolTipText_Lambda([this]()
									{
										FString outterName = Item->GetStat().OutterName;
										FString properName = Item->GetStat().PropertyName;
										FString fullName = outterName.AppendChar('.').Append(properName);
										return FText::FromString(fullName);
									})
								]

								+SHorizontalBox::Slot()
								.HAlign(HAlign_Right)
								.AutoWidth()
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont"))
									.Text(FText::FromString(Item->GetStat().PropertyType))
									.ColorAndOpacity_Lambda([this]()
									{
										const FString& strType = Item->GetStat().PropertyType;
										FLinearColor typeColor(1.0f, 1.0f, 1.0f, 1.0f);

										if (strType.Equals("bool"))
											typeColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.0f);
										else if (strType.Equals("int32"))
											typeColor = FLinearColor(0.0f, 0.9f, 0.8f, 1.0f);
										else if (strType.Equals("float"))
											typeColor = FLinearColor(0.5f, 0.9f, 0.2f, 1.0f);
										else if (strType.Equals("uint8"))
											typeColor = FLinearColor(0.0f, 0.5f, 0.1f, 1.0f);
										else if (strType.Equals("FVector"))
											typeColor = FLinearColor(1.0f, 0.9f, 0.0f, 1.0f);
										else if (strType.Equals("FRotator"))
											typeColor = FLinearColor(0.7f, 0.5f, 0.7f, 1.0f);
										else if (strType.Equals("FTransform"))
											typeColor = FLinearColor(0.8f, 0.5f, 0.1f, 1.0f);

										return typeColor;
									})
								]
							]

							// more ...
							+SVerticalBox::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							.FillHeight(1.0f)
							[
								SNew(SExpandableArea)
								.InitiallyCollapsed(this->Item->IsExpanded == false)
								.OnAreaExpansionChanged_Lambda([this](bool state)
								{
									if (PendingListRefreshDelay < 0.05f)
									{
										this->PendingListRefreshDelay = 0.1f;
										this->Item->IsExpanded = state;
									}
								})
								.HeaderContent() 
								[ 
									SNew(STextBlock)
									.ToolTipText(FText::FromString("More options."))
									.Text(FText::FromString("Advanced")) 
								]
								.BodyContent()
								[
									SNew(SBox)
									.Padding(8.0f)
									.HeightOverride(230.0f)
									.HAlign(HAlign_Fill)
									.VAlign(VAlign_Fill)
									[
										SNew(SVerticalBox)
										+SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(SVerticalBox)
											+SVerticalBox::Slot() [ SNew(STextBlock).Text(FText::FromString("Alias:")) ]
											+SVerticalBox::Slot() 
											.HAlign(HAlign_Fill)
											[ 
												SNew(SEditableTextBox)
												.Text(FText::FromString(Item->GetStat().AlliasName))
												.OnTextChanged_Lambda([this](const FText& InText)
												{
													Item->GetStat().AlliasName = InText.ToString();
												})
											]
										]

										+SVerticalBox::Slot()
										.AutoHeight()
										.Padding(0.0f, 2.0f, 0.0f, 0.0f)
										[
											SNew(SVerticalBox)
											+SVerticalBox::Slot() [ SNew(STextBlock).Text(FText::FromString("Group:")) ]
											+SVerticalBox::Slot() 
											.HAlign(HAlign_Fill)
											[ 
												SNew(SEditableTextBox)
												.Text(FText::FromString(Item->GetStat().Group))
												.OnTextChanged_Lambda([this](const FText& InText)
												{
													Item->GetStat().Group = InText.ToString();
												})
											]
										]

										+SVerticalBox::Slot()
										.AutoHeight()
										.Padding(0.0f, 2.0f, 0.0f, 0.0f)
										[
											SNew(SVerticalBox)
											+SVerticalBox::Slot() [ SNew(STextBlock).Text(FText::FromString("Color:")) ]
											+SVerticalBox::Slot() 
											.HAlign(HAlign_Fill)
											[ 
												SNew(SButton)
												.ButtonColorAndOpacity_Lambda([this]() { return Item->GetStat().Color; })
												.OnClicked_Lambda([this]() 
												{	
													FColorPickerArgs args;
													{
														args.InitialColorOverride = Item->GetStat().Color;
														args.bUseAlpha = false;
														args.OnColorCommitted.BindLambda([this](FLinearColor InColor) { Item->GetStat().Color = InColor; });
													}
													OpenColorPicker(args);

													return FReply::Handled();
												})
											]
										]
									
										+SVerticalBox::Slot()
										.FillHeight(1.0f)
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Fill)
										.Padding(0.0f, 2.0f, 0.0f, 0.0f)
										[
											SNew(SVerticalBox)
											+SVerticalBox::Slot() 
											.AutoHeight()
											.MaxHeight(24.0f)
											[ 
												SNew(STextBlock).Text(FText::FromString("Description:")) 
											]
											+SVerticalBox::Slot() 
											.FillHeight(1.0f)
											[ 
												SNew(SMultiLineEditableTextBox)
												.Text(FText::FromString(Item->GetStat().Description))
												.OnTextChanged_Lambda([this](const FText& InText)
												{
													Item->GetStat().Description = InText.ToString();
												})
											]
										]

										+SVerticalBox::Slot()
										.AutoHeight()
										.Padding(0.0f, 2.0f, 0.0f, 0.0f)
										[
											SNew(SVerticalBox)
											+SVerticalBox::Slot()
											.Padding(0.0f, 2.0f, 0.0f, 0.0f)
											[SNew(STextBlock).Text(FText::FromString("Miscellaneous:"))]
											
											+SVerticalBox::Slot() 
											.HAlign(HAlign_Fill)
											.AutoHeight()					
											[ 
												SNew(SBorder)
												.HAlign(HAlign_Fill)
												.VAlign(VAlign_Fill)
												[
													SNew(SHorizontalBox)
													+SHorizontalBox::Slot()
													.AutoWidth()
													[
														SNew(SCheckBox)
														.ToolTipText(FText::FromString(TEXT("If enabled this stat will be streamed to csv file.")))
														.IsChecked(Item->GetStat().StreamToCsv ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
														.OnCheckStateChanged_Lambda([&](ECheckBoxState checkState)
														{
															switch (checkState)
															{
															case ECheckBoxState::Checked:
																Item->GetStat().StreamToCsv = true;
																break;

															case ECheckBoxState::Unchecked:
															case ECheckBoxState::Undetermined:
															default:
																Item->GetStat().StreamToCsv = false;
																break;
															}
														})
													]

													+SHorizontalBox::Slot()
													.FillWidth(1.0f)
													[
														SNew(STextBlock)
														.Text(FText::FromString(TEXT("Stream to CSV file")))
													]
												] 
											]
										]
									]
								]
							]
						];
				}			
			};

			return SNew(STracerPropertyListItemWidgetInner, OwnerTable, InItem).MyUiParent(InParent);
		}		
	};

	return SNew(STracerPropertyListItemWidget, OwnerTable, InItem).MyUiParent(InParent);
}

TSharedRef<IDetailCustomization> TracerComponentCustomization::MakeInstance()
{
	return MakeShareable(new TracerComponentCustomization);
}

void TracerComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& tracerCategory = DetailBuilder.EditCategory("Tracer Component", FText::FromString("Stats Tracaing"), ECategoryPriority::Important);
	
	TArray<TWeakObjectPtr<UObject>> outObjects;
	DetailBuilder.GetObjectsBeingCustomized(outObjects);

	if (outObjects.Num() == 1 && outObjects[0].IsValid())
	{
		this->m_TracerComp = TWeakObjectPtr<UTracerComponent>((UTracerComponent*)outObjects[0].Get());
		
		tracerCategory.AddCustomRow(FText::FromString("TracerProperties"))
		.WholeRowContent()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				// Tracer Name
				SNew(SVerticalBox)
				+SVerticalBox::Slot() 
				.AutoHeight()
				.MaxHeight(24.0f)
				[ 
					SNew(STextBlock).Text(FText::FromString("Tracer Name:")) 
				]

				+SVerticalBox::Slot() 
				.HAlign(HAlign_Fill)
				.AutoHeight()
				.MaxHeight(24.0f)
				[ 
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SNew(SEditableTextBox)
						.Font(FStatsTracerEditorStyle::Get().GetFontStyle("BigBoldFont"))
						.Text(FText::FromString(this->m_TracerComp.IsValid() ? this->m_TracerComp->TracerName : TEXT("#ERR")))
						.OnTextChanged_Lambda([this](const FText& InText)
						{
							if(this->m_TracerComp.IsValid() == true)
								this->m_TracerComp->TracerName = InText.ToString();
						})
					]

					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2.0f, 0.0f, 0.0f, 0.0f)
					[
						SNew(SBox)
						.WidthOverride(24.0f)
						.HeightOverride(24.0f)
						[
							SNew(SButton)
							.ToolTipText_Lambda([&]() 
							{
								if (this->m_TracerComp.IsValid() == false)
									return FText::FromString(TEXT(""));

								if(this->m_TracerComp->EnableStreamToCsv == true)
									return FText::FromString(TEXT("CSV streaming enabled."));

								
								return FText::FromString(TEXT("CSV streaming disabled."));
							})
							.OnClicked_Lambda([&]() 
							{
								if (this->m_TracerComp.IsValid() == true)
									this->m_TracerComp->EnableStreamToCsv = !this->m_TracerComp->EnableStreamToCsv;

								return FReply::Handled();
							})
							.ForegroundColor(FSlateColor::UseForeground())
							[
								TSharedRef<SWidget>(SNew(SImage)
								.Image_Lambda([this]()
								{	
									if (this->m_TracerComp.IsValid() == true && this->m_TracerComp->EnableStreamToCsv == true)
										return FStatsTracerEditorStyle::Get().GetBrush("CSVStreamOnIcon");

									return FStatsTracerEditorStyle::Get().GetBrush("CSVStreamOffIcon");
								}))
							]
						]
					]
				]

				// Tracer Description
				+SVerticalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.AutoHeight()
				.Padding(0.0f, 4.0f, 0.0f, 0.0f)
				[
					SNew(SBox)
					.HeightOverride(72.0f)
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot() 
						.AutoHeight()
						.MaxHeight(24.0f)
						[ 
							SNew(STextBlock).Text(FText::FromString("Tracer Description:")) 
						]
						+SVerticalBox::Slot() 
						.FillHeight(1.0f)
						[ 
							SNew(SMultiLineEditableTextBox)
							.Text(FText::FromString(this->m_TracerComp.IsValid() ? this->m_TracerComp->TracerDescription : TEXT("#ERR")))
							.OnTextChanged_Lambda([&](const FText& InText)
							{
								if (this->m_TracerComp.IsValid() == true)
									this->m_TracerComp->TracerDescription = InText.ToString();
							})
						]
					]
				]
			]
		];

		// Stats list
		tracerCategory.AddCustomRow(FText::FromString("Traced Stats"), true)
		.WholeRowContent()
		[
			SAssignNew(this->m_TracerPropertyList, STracerPropertyListUI)
			.TracerComp(this->m_TracerComp)
		];

	}
}

FReply TracerComponentCustomization::DetectStatsButtonClicked(UTracerComponent* tracerComponentRef)
{
	if(tracerComponentRef != nullptr)
		tracerComponentRef->ScanStats();

	if (this->m_TracerPropertyList.IsValid() == true)
		this->m_TracerPropertyList.ToSharedRef()->UpdateListview();

	return FReply::Handled();
}