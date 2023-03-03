///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Private\Widgets\STracerDataChart.cpp
///
/// Summary:	Implements the tracer data chart class.
///-------------------------------------------------------------------------------------------------

#include "STracerDataChart.h"
#include "StatsTracerEditorPCH.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void STracerDataChart::Construct(const FArguments& InArgs)
{
	this->DataSourceArray	= InArgs._DataSourceArray;
	this->ZoomFactor		= InArgs._ZoomFactor;
	this->ChartAreaWidth	= InArgs._ChartAreaWidth;
	this->SessionHandle		= InArgs._SessionHandle;

	// fetch sessions sample frequency
	this->UpdateFrequency = this->SessionHandle.IsValid() ? this->SessionHandle.Pin()->GetSessionSampleFrequency() : 1;

	if (this->DataSourceArray == nullptr)
	{
		ChildSlot
		[
			SNew(STextBlock).Text(FText::FromString("No data."))
		];
		return;
	}

	this->TracerDataSourceListItems.Reset();
	for (auto& ds : *(this->DataSourceArray))
	{
		switch (ds->GetDataSourceType())
		{
			case StatsTracer::Bool:
			case StatsTracer::Int:
			case StatsTracer::Float:
			case StatsTracer::Byte:
				this->TracerDataSourceListItems.Add(TTracerDataSourceListItem(new FSimpleTracerDataSourceListItem(ds)));
				break;

			case StatsTracer::Vector:
			case StatsTracer::Rotator:
				this->TracerDataSourceListItems.Add(TTracerDataSourceListItem(new FVectorTracerDataSourceListItem(ds)));
				break;

			case StatsTracer::Transform:
				this->TracerDataSourceListItems.Add(TTracerDataSourceListItem(new FTransformTracerDataSourceListItem(ds)));
				break; 
		}
	}
		

	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SBorder)
		[
			// left side: data source names
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillWidth(0.25f)
			[
				SAssignNew(TracerDataSourceListView, STracerDataSourceListView)
				.ListItemsSource(&TracerDataSourceListItems)
				.OnGenerateRow(this, &STracerDataChart::OnGenerateRowForTracerDataSourceListView)
				.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("Stats")
					.DefaultLabel(FText::FromString(TEXT("Stats")))
					.FillWidth(1.0f)
				)
				.SelectionMode(ESelectionMode::None)
			]

			// right side: plot
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillWidth(0.75f)
			[
				SAssignNew(SPlotAreaScrollBox, SScrollBox)
				.Orientation(EOrientation::Orient_Horizontal)
				.OnUserScrolled_Lambda([this](float scroll) 
				{
					if (this->SPlotAreaWidget.IsValid() == true)
						this->SPlotAreaWidget->Invalidate(EInvalidateWidget::LayoutAndVolatility);
				})
				+SScrollBox::Slot()
				[
					SNew(SBox)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.WidthOverride_Lambda([this]()
					{
						return (this->ChartAreaWidth * 0.75f) * this->ZoomFactor;
					})
					[
						SAssignNew(SPlotAreaWidget, SPlotArea)
						.TracerDataChart(this)
					]
				]
			]
		]
	];

}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION


void STracerDataChart::STracerDataListItemWidget::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TTracerDataSourceListItem InListItem)
{
	Listview = static_cast<STracerDataSourceListView*>(&InOwnerTable.Get());
	Item = InListItem;

	SMultiColumnTableRow<TTracerDataSourceListItem>::Construct(FSuperRowType::FArguments(), InOwnerTable);
}

void STracerDataChart::STracerDataListItemWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	// hack: This creates a little bit of a delay until the triggered expandable area has resized,
	// right after that we need to call RequestListRefresh to make the scrollbar work
	if (this->PendingListRefreshDelay >= 0.0f)
	{
		this->PendingListRefreshDelay -= InDeltaTime;

		if (this->PendingListRefreshDelay <= 0.0f)
		{
			this->PendingListRefreshDelay = -1.0f;

			if (Listview != nullptr)
				Listview->RequestListRefresh();
		}
	}
}

void STracerDataChart::STracerDataListItemWidget::Refresh()
{
	if (PendingListRefreshDelay < 0.05f)
	{
		PendingListRefreshDelay = 0.1f;
	}
}

TSharedRef<SWidget> STracerDataChart::STracerDataListItemWidget::GenerateWidgetForColumn(const FName& ColumnName)
{
	return Item->BuildWidget(this);
}

TSharedRef<ITableRow> STracerDataChart::OnGenerateRowForTracerDataSourceListView(TTracerDataSourceListItem InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STracerDataListItemWidget, OwnerTable, InItem);
}

///-------------------------------------------------------------------------------------------------
/// STracerDataChart::SPlotArea::TChartLayout
///-------------------------------------------------------------------------------------------------

STracerDataChart::SPlotArea::TChartLayout::TChartLayout()
{}

STracerDataChart::SPlotArea::TChartLayout::TChartLayout(float x0, float y0, float x1, float y1, float scrollOffset)
{	
	const TSharedRef<FSlateFontMeasure>	FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	// fonts
	ChartDefaultFont	= FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont");
	ChartAidLableFont	= FStatsTracerEditorStyle::Get().GetFontStyle("SmallFont");
	ChartIndexLableFont	= FStatsTracerEditorStyle::Get().GetFontStyle("BigBoldFont");
	ChartTimeLabelFont	= FStatsTracerEditorStyle::Get().GetFontStyle("SmallFont");

	MaxFontCharHeight	= FontMeasureService->Measure(TEXT("!"), ChartDefaultFont).Y;
	ZeroCharWidth		= FontMeasureService->Measure(TEXT("0"), ChartDefaultFont).X;
	TimeLabelSize		= FontMeasureService->Measure(TEXT("00:00:00.000"), ChartTimeLabelFont).X;

	ScrollOffset		= scrollOffset;

	OutterRect			= TRect(x0, y0, x1, y1);
	InnerRect			= TRect(
							x0 + MARGIN.X0,
							y0 + MARGIN.Y0,
							x1 - MARGIN.X1,
							y1 - MARGIN.Y1 - MaxFontCharHeight - 4.0f);
}

STracerDataChart::SPlotArea::TChartLayout& STracerDataChart::SPlotArea::TChartLayout::operator=(const TChartLayout& rhs)
{
	if (this == &rhs)
		return *this;

	this->ChartDefaultFont		= rhs.ChartDefaultFont;
	this->MaxFontCharHeight		= rhs.MaxFontCharHeight;
	this->ZeroCharWidth			= rhs.ZeroCharWidth;
	this->ScrollOffset			= rhs.ScrollOffset;
								
	this->OutterRect			= rhs.OutterRect;
	this->InnerRect				= rhs.InnerRect;

	this->SampleDataXOffset		= rhs.SampleDataXOffset;
	this->XAxisGridSteps		= rhs.XAxisGridSteps;
	this->YAxisMajorStepN		= rhs.YAxisMajorStepN;
								
	this->XAxisGridStepSize	= rhs.XAxisGridStepSize;
								
	this->XPos0					= rhs.XPos0;
	this->XPos1					= rhs.XPos1;
	this->YPos0					= rhs.YPos0;
	this->YPos1					= rhs.YPos1;
							
	this->XAxisLabelAreaSize	= rhs.XAxisLabelAreaSize;
	this->YAxisLabelAreaSize	= rhs.YAxisLabelAreaSize;

	return *this;
}

bool STracerDataChart::SPlotArea::TChartLayout::IsValidLayout() const
{
	if (InnerRect.X1 < 0.0f || InnerRect.Y1 < 0.0f || FMath::IsNearlyZero(OutterRect.X1) || FMath::IsNearlyZero(OutterRect.Y1))
		return false;

	return true;
}

void STracerDataChart::SPlotArea::TChartLayout::Update()
{
	this->YAxisMajorStepN = FMath::DivideAndRoundDown<float>(this->InnerRect.Y1, (float)UStatsTracerEditorSettings::GetInstance()->ChartYAxisGridSize);
 
	const TSharedRef<FSlateFontMeasure>	FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	this->YAxisLabelAreaSize = FontMeasureService->Measure(FString::Printf(*FString::Printf(TEXT(" -%%.%df"), this->Precision), FMath::Max(FMath::Abs(YPos0), FMath::Abs(YPos1))), ChartDefaultFont);


	/*
	
	                 

			^
			|            ___sn
			+--   ____  /   *
			|    /    \/     \ sn+1
			+-__/             *___
			|/
			o-------+-------+-------+-------+------>

						-->| |<--						SampleDataXOffset
			|<-- -->|									XAxisGridSteps
			|<----							  ---->|	ChartPlotAreaWith


	*/


	this->ChartPlotAreaWith = this->InnerRect.X1 - this->InnerRect.X0 - this->YAxisLabelAreaSize.X;
	const float XAxisGridSize = (float)UStatsTracerEditorSettings::GetInstance()->ChartXAxisGridSize;

	this->SampleDataXOffset = FMath::Max<float>(1.0f, ChartPlotAreaWith) / FMath::Max<float>(1.0f, (float)this->SampleWindowSize); // SampleWindowSize >= 1


	this->XAxisGridSteps = FMath::DivideAndRoundDown<float>(ChartPlotAreaWith, XAxisGridSize);

	float RestChartWidth = FMath::Max<float>(0.0f, ChartPlotAreaWith - ((float)this->XAxisGridSteps * XAxisGridSize));

	// discretize x grid size to number of sample points.
	this->XAxisGridStepSize = FMath::RoundToInt((XAxisGridSize + (RestChartWidth / FMath::Max<float>(1.0f, this->XAxisGridSteps))) / this->SampleDataXOffset);

	this->XAxisGridSteps = FMath::DivideAndRoundDown<float>((float)this->SampleWindowSize, this->XAxisGridStepSize);


	this->XPos0 = this->GetXPos(0);
	this->XPos1 = this->GetXPos(this->SampleWindowSize);

	this->XAxisLabelAreaSize = FVector2D(InnerRect.X1 - InnerRect.X0, OutterRect.Y1 - InnerRect.Y1);
}

float STracerDataChart::SPlotArea::TChartLayout::MeasureTextWidth(const FString& text, const FSlateFontInfo& font) 
{ 
	const TSharedRef<FSlateFontMeasure>	FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	return FontMeasureService->Measure(text, font).X;
}

///-------------------------------------------------------------------------------------------------
/// STracerDataChart::SPlotArea
///-------------------------------------------------------------------------------------------------

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void STracerDataChart::SPlotArea::Construct(const FArguments & InArgs)
{
	this->TracerDataChart = InArgs._TracerDataChart;

	ChildSlot
	[
		SNew(SOverlay)
		.Visibility( EVisibility::SelfHitTestInvisible )
		
		+SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void STracerDataChart::SPlotArea::Tick(const FGeometry & AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (this->IsIndexed == true)
		for (auto& DSItem : this->TracerDataChart->TracerDataSourceListItems)
			if (DSItem.IsValid() == true)
				DSItem->DataSourceIndex = this->BufferIndex;
}

int32 STracerDataChart::SPlotArea::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList & OutDrawElements, int32 LayerId, const FWidgetStyle & InWidgetStyle, bool bParentEnabled) const
{
	// Rendering info.
	const bool bEnabled									= ShouldBeEnabled(bParentEnabled);
	ESlateDrawEffect DrawEffects						= bEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;

	// Brushes
	const FSlateBrush* TimelineAreaBrush				= FEditorStyle::GetBrush("Profiler.LineGraphArea");
	const FSlateBrush* WhiteBrush						= FEditorStyle::GetBrush("WhiteTexture");
	const FSlateBrush* ChartAxisLabelBackgroundBrush	= FEditorStyle::GetBrush("ChildWindow.Background");

	// Define Chart Layers (ORDER MATTERS!)
	const int32 ChartBackgroundLayerId					= LayerId++;
	const int32 ChartGridLayerId						= LayerId++;
	const int32 ChartPlotLayerId						= LayerId++;
	const int32 ChartXAxisTextBackgroundLayerId			= LayerId++;
	const int32 ChartXAxisTextForegroundLayerId			= LayerId++;
	const int32 ChartYAxisTextBackgroundLayerId			= LayerId++;
	const int32 ChartYAxisTextForegroundLayerId			= LayerId++;
	const int32 ChartIndexLayerId						= LayerId++;
	// more layer...	

	// Vertex buffer for lines
	TArray<FVector2D> LineBuffer0;

	// Update Layout
	TChartLayout Layout(0.0f, 0.0f, AllottedGeometry.GetLocalSize().X, AllottedGeometry.GetLocalSize().Y, this->TracerDataChart->GetScrollOffset());

	if(Layout.IsValidLayout() == false)
		return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled && IsEnabled());


	class XAxisLabelFormater
	{
	private:

		ETimelineMode							Mode;

		float									MaxTime;
		uint64									MaxFrame;

		const StatsTracer::TDataSourceHandle	DataSource0;

	public:

		XAxisLabelFormater(const StatsTracer::TDataSourceHandle InDataSource0, ETimelineMode mode) :
			Mode(mode),
			DataSource0(InDataSource0)
		{
			if (InDataSource0.IsValid())
			{
				MaxTime  = InDataSource0->GetElapsedTime(InDataSource0->GetSampleCount() - 1);
				MaxFrame = InDataSource0->GetFrameNumber(InDataSource0->GetSampleCount() - 1);
			}
		}

		~XAxisLabelFormater()
		{}

		FString GetLabelForIndex(uint32 index)
		{
			if ( this->Mode == ETimelineMode::None || this->DataSource0.IsValid() == false || index >= this->DataSource0->GetSampleCount())
				return FString("");

			FString OutLabel = "";

			switch (this->Mode)
			{
				// Time
				case ETimelineMode::Time:
				{
					OutLabel = FTimespan::FromSeconds(this->DataSource0->GetElapsedTime(index)).ToString().RightChop(1);
					break;
				}

				// Relative Time
				case ETimelineMode::RelativeTime:
				{
					OutLabel = FTimespan::FromSeconds(this->MaxTime - this->DataSource0->GetElapsedTime(index)).ToString().RightChop(1);
					break;
				}

				// Frame
				case ETimelineMode::Frame:
				{
					OutLabel = FString::Printf(TEXT("%u"), this->DataSource0->GetFrameNumber(index));
					break;
				}

				// Relative Frame
				case ETimelineMode::RelativeFrame:
				{
					OutLabel = FString::Printf(TEXT("%u"), this->MaxFrame - this->DataSource0->GetFrameNumber(index));
					break;
				}
			}
			return OutLabel;
		}
	} XFormaterAxis((*this->TracerDataChart)[0]->DataSource, UStatsTracerEditorSettings::GetInstance()->ChartXAxisTimelineMode), 
	  XFormaterReferencePointer((*this->TracerDataChart)[0]->DataSource, UStatsTracerEditorSettings::GetInstance()->ChartReferencePointerTimelineMode);


	// Determine Chart Layout
	{
		float DS_MinValue = 0.0f;
		float DS_MaxValue = 0.0f;

		// determine the overall Min/Max value of all visivle data sources
		{
			for (int i = 0; i < this->TracerDataChart->GetDataSourceCount(); ++i)
			{
				// only care about visible data sources
				if ((*this->TracerDataChart)[i]->IsVisible == false)
					continue;

				StatsTracer::TDataSourceHandle dataSourceHandle = (*this->TracerDataChart)[i]->DataSource;

				switch (dataSourceHandle->GetDataSourceType())
				{
					case StatsTracer::Bool:
					{
						if (DS_MaxValue < 1.0f)
							DS_MaxValue = 1.0f;

						break;
					}

					case StatsTracer::Int:
					{
						auto ds = dataSourceHandle->GetAs<StatsTracer::FIntDataSource>();
						StatsTracer::FIntDataSourceView view(ds);

						if (view.GetMinValue() < DS_MinValue)
							DS_MinValue = view.GetMinValue();

						if (view.GetMaxValue() > DS_MaxValue)
							DS_MaxValue = view.GetMaxValue();

						break;
					}

					case StatsTracer::Float:
					{
						StatsTracer::FFloatDataSourceView view(dataSourceHandle->GetAs<StatsTracer::FFloatDataSource>());

						if (view.GetMinValue() < DS_MinValue)
							DS_MinValue = view.GetMinValue();

						if (view.GetMaxValue() > DS_MaxValue)
							DS_MaxValue = view.GetMaxValue();

						break;
					}

					case StatsTracer::Byte:
					{
						StatsTracer::FByteDataSourceView view(dataSourceHandle->GetAs<StatsTracer::FByteDataSource>());

						if (view.GetMinValue() < DS_MinValue)
							DS_MinValue = view.GetMinValue();

						if (view.GetMaxValue() > DS_MaxValue)
							DS_MaxValue = view.GetMaxValue();

						break;
					}

					case StatsTracer::Vector:
					{
						FVectorTracerDataSourceListItem* li = (FVectorTracerDataSourceListItem*)((*this->TracerDataChart)[i].Get());
						StatsTracer::FVectorDataSourceView view(dataSourceHandle->GetAs<StatsTracer::FVectorDataSource>());

						if (li->ShowX == true)
						{
							if (view.GetMinValueX() < DS_MinValue)
								DS_MinValue = view.GetMinValueX();

							if (view.GetMaxValueX() > DS_MaxValue)
								DS_MaxValue = view.GetMaxValueX();
						}

						if (li->ShowY == true)
						{
							if (view.GetMinValueY() < DS_MinValue)
								DS_MinValue = view.GetMinValueY();

							if (view.GetMaxValueY() > DS_MaxValue)
								DS_MaxValue = view.GetMaxValueY();
						}

						if (li->ShowZ == true)
						{
							if (view.GetMinValueZ() < DS_MinValue)
								DS_MinValue = view.GetMinValueZ();

							if (view.GetMaxValueZ() > DS_MaxValue)
								DS_MaxValue = view.GetMaxValueZ();
						}

						break;
					}

					case StatsTracer::Rotator:
					{
						FVectorTracerDataSourceListItem* li = (FVectorTracerDataSourceListItem*)((*this->TracerDataChart)[i].Get());
						StatsTracer::FRotatorDataSourceView view(dataSourceHandle->GetAs<StatsTracer::FRotatorDataSource>());

						if (li->ShowX == true)
						{
							if (view.GetMinValueX() < DS_MinValue)
								DS_MinValue = view.GetMinValueX();

							if (view.GetMaxValueX() > DS_MaxValue)
								DS_MaxValue = view.GetMaxValueX();
						}

						if (li->ShowY == true)
						{
							if (view.GetMinValueY() < DS_MinValue)
								DS_MinValue = view.GetMinValueY();

							if (view.GetMaxValueY() > DS_MaxValue)
								DS_MaxValue = view.GetMaxValueY();
						}

						if (li->ShowZ == true)
						{
							if (view.GetMinValueZ() < DS_MinValue)
								DS_MinValue = view.GetMinValueZ();

							if (view.GetMaxValueZ() > DS_MaxValue)
								DS_MaxValue = view.GetMaxValueZ();
						}

						break;
					}

					case StatsTracer::Transform:
					{
						FTransformTracerDataSourceListItem* li = (FTransformTracerDataSourceListItem*)((*this->TracerDataChart)[i].Get());
						StatsTracer::FTransformDataSourceView view(dataSourceHandle->GetAs<StatsTracer::FTransformDataSource>());

						if (li->ShowP == true)
						{
							if (li->ShowPX == true)
							{
								if (view.GetMinValuePX() < DS_MinValue)
									DS_MinValue = view.GetMinValuePX();

								if (view.GetMaxValuePX() > DS_MaxValue)
									DS_MaxValue = view.GetMaxValuePX();
							}

							if (li->ShowPY == true)
							{
								if (view.GetMinValuePY() < DS_MinValue)
									DS_MinValue = view.GetMinValuePY();

								if (view.GetMaxValuePY() > DS_MaxValue)
									DS_MaxValue = view.GetMaxValuePY();
							}

							if (li->ShowPZ == true)
							{
								if (view.GetMinValuePZ() < DS_MinValue)
									DS_MinValue = view.GetMinValuePZ();

								if (view.GetMaxValuePZ() > DS_MaxValue)
									DS_MaxValue = view.GetMaxValuePZ();
							}
						}

						if (li->ShowR == true)
						{
							if (li->ShowRX == true)
							{
								if (view.GetMinValueRX() < DS_MinValue)
									DS_MinValue = view.GetMinValueRX();

								if (view.GetMaxValueRX() > DS_MaxValue)
									DS_MaxValue = view.GetMaxValueRX();
							}

							if (li->ShowRY == true)
							{
								if (view.GetMinValueRY() < DS_MinValue)
									DS_MinValue = view.GetMinValueRY();

								if (view.GetMaxValueRY() > DS_MaxValue)
									DS_MaxValue = view.GetMaxValueRY();
							}

							if (li->ShowRZ == true)
							{
								if (view.GetMinValueRZ() < DS_MinValue)
									DS_MinValue = view.GetMinValueRZ();

								if (view.GetMaxValueRZ() > DS_MaxValue)
									DS_MaxValue = view.GetMaxValueRZ();
							}
						}

						if (li->ShowS == true)
						{
							if (li->ShowSX == true)
							{
								if (view.GetMinValueSX() < DS_MinValue)
									DS_MinValue = view.GetMinValueSX();

								if (view.GetMaxValueSX() > DS_MaxValue)
									DS_MaxValue = view.GetMaxValueSX();
							}

							if (li->ShowSY == true)
							{
								if (view.GetMinValueSY() < DS_MinValue)
									DS_MinValue = view.GetMinValueSY();

								if (view.GetMaxValueSY() > DS_MaxValue)
									DS_MaxValue = view.GetMaxValueSY();
							}

							if (li->ShowSZ == true)
							{
								if (view.GetMinValueSZ() < DS_MinValue)
									DS_MinValue = view.GetMinValueSZ();

								if (view.GetMaxValueSZ() > DS_MaxValue)
									DS_MaxValue = view.GetMaxValueSZ();
							}
						}

						break;
					}
				}
			}
		}

		// Update Layout, set necessary properties
		static const auto RoundToNextN = [](float value, int32* precision)
		{		
			if (FMath::IsNearlyZero(value) == true)
			{
				*precision = 0;
				return 0.0f;
			}

			const bool IsFraction = FMath::Abs(value) < 1.0f;

			int D = 0;			
			if (IsFraction == false)
			{
				int V = FMath::Abs(value);
				while (V) { V /= 10; D++; }
			}
			else
			{
				float V = FMath::Abs(value);
				while (V < 1.0f) { V *= 10; D++; }
			}
			const float N = FMath::Pow(10, D - (IsFraction == false ? 1 : 0));
			
			if (IsFraction == true)
			{
				*precision = FMath::Max<int32>(1, D + 1);
				return (FMath::Sign(value) * (FMath::CeilToFloat((FMath::Abs(value) * N)) / N));
			}
			else
				*precision = 2;

			return (FMath::Sign(value) * (FMath::CeilToFloat((FMath::Abs(value) / N)) * N));
		};


		// round min/max to the next .. 0.0001th, 0.001th, 0.01th, 0.1th .. 1, 10th, 100th, 1000th ..
		int32 precisionMin, precisionMax;

		DS_MinValue = RoundToNextN(DS_MinValue, &precisionMin);
		DS_MaxValue = RoundToNextN(DS_MaxValue, &precisionMax);	

		Layout.SetY0(FMath::IsNearlyZero(DS_MinValue) ? (FMath::Abs(DS_MaxValue) > 1.0f ? -1.0f : -1.0f / FMath::Pow(10.0f, FMath::Max<float>(1.0f, precisionMax))) : DS_MinValue * 1.05f);
		Layout.SetY1(FMath::IsNearlyZero(DS_MaxValue) ? (FMath::Abs(DS_MinValue) > 1.0f ?  1.0f :  1.0f / FMath::Pow(10.0f, FMath::Max<float>(1.0f, precisionMin))) : DS_MaxValue * 1.05f);

		Layout.SetPrecision(FMath::Max<int32>(precisionMin, precisionMax));
		Layout.SetSampleWindowSize((*this->TracerDataChart)[0]->DataSource->GetSampleWindowSize());

		Layout.Update();
	}

	// Background layer
	;{
		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			ChartBackgroundLayerId,
			AllottedGeometry.ToPaintGeometry(Layout.OutterRect.GetTopLeft(), Layout.OutterRect.GetBottomRight()),
			TimelineAreaBrush,
			DrawEffects,
			TimelineAreaBrush->GetTint(InWidgetStyle) * 0.25f
		);
	}

	// Grid Layer
	;{	
		const bool enoughSpaceAvailable =
			(Layout.GetXAxisStepSize() == 0) == false
			|| FMath::IsNearlyZero(Layout.GetXPos1()) == false
			|| FMath::IsNearlyZero(Layout.GetYAxisMajorStepSize()) == false
			|| FMath::IsNearlyZero(Layout.GetYPos1()) == false;

		// Grid
		if (enoughSpaceAvailable == true && UStatsTracerEditorSettings::GetInstance()->ChartShowGrid == true)
		{	
			// Vertical Grid lines
			int32 HX = 0;
			float freeSpace = Layout.TimeLabelSize;
			{
				for (int32 StepX = 0; StepX < Layout.GetXAxisStepN() + 1; ++StepX)
				{
					const float XMajor = Layout.GetXPos(HX);
					float gridAlpha = 0.25f;

					// draw label
					if (freeSpace >= (2.0f * Layout.TimeLabelSize))
					{
						// consume space
						freeSpace = 0.0f;

						if (XMajor + freeSpace <= Layout.ChartPlotAreaWith)
						{
							// show stronger grid line
							gridAlpha = 0.5f;

							FString label = XFormaterAxis.GetLabelForIndex(HX);

							FSlateDrawElement::MakeText
							(
								OutDrawElements,
								LayerId,
								AllottedGeometry.ToPaintGeometry(FVector2D(XMajor - (Layout.MeasureTextWidth(label, Layout.ChartTimeLabelFont) * 0.5f), Layout.InnerRect.Y1 + 2.0f), FVector2D(Layout.TimeLabelSize, Layout.MaxFontCharHeight)),
								label,
								Layout.ChartTimeLabelFont,
								DrawEffects,
								InWidgetStyle.GetColorAndOpacityTint() * FLinearColor(1.0f, 1.0f, 1.0f, 0.5f)
							);
						}
					}

					// draw vertical line
					LineBuffer0.Empty(2);
					new (LineBuffer0)FVector2D(XMajor, Layout.InnerRect.Y0);
					new (LineBuffer0)FVector2D(XMajor, Layout.InnerRect.Y1);					

					FSlateDrawElement::MakeLines
					(
						OutDrawElements,
						ChartGridLayerId,
						AllottedGeometry.ToPaintGeometry(),
						LineBuffer0,
						DrawEffects,
						InWidgetStyle.GetColorAndOpacityTint() * FLinearColor(1.0f, 1.0f, 1.0f, gridAlpha),
						false
					);

					HX += Layout.GetXAxisStepSize(); 
					freeSpace += (Layout.GetXAxisStepSize() * Layout.SampleDataXOffset);
				}
			}

			// Horizontal Grid lines
			float HY = Layout.GetYPos0();
			{
				for (int32 StepY = 0; StepY < Layout.GetYAxisMajorStepN() + 1; ++StepY)
				{
					if (FMath::IsNearlyZero(HY) == true)
					{
						HY += Layout.GetYAxisMajorStepSize();
						continue;
					}

					const float YMajor = Layout.GetYPos(HY);


					// this will fade out the grid line and label if near index line
					float alphaTween = 1.0f;
					if (this->IsIndexed == true && UStatsTracerEditorSettings::GetInstance()->ChartShowIndicatorCross == true)
					{
						const float Index2LabelDiff = FMath::Abs(this->MousePosition.Y - YMajor);

						if (Index2LabelDiff < Layout.MaxFontCharHeight)
							alphaTween = (Index2LabelDiff / Layout.MaxFontCharHeight);
					}


					LineBuffer0.Empty(2);
					new (LineBuffer0)FVector2D(Layout.ScrollOffset + Layout.GetXPos0(), YMajor);
					new (LineBuffer0)FVector2D(Layout.GetXPos1() + 4.0f, YMajor);

					FSlateDrawElement::MakeLines
					(
						OutDrawElements,
						ChartGridLayerId,
						AllottedGeometry.ToPaintGeometry(),
						LineBuffer0,
						DrawEffects,
						InWidgetStyle.GetColorAndOpacityTint() * FLinearColor(1.0f, 1.0f, 1.0f, 0.33f * alphaTween),
						false
					);

					// Draw Axis-Tick Value
					const FString Label = FString::Printf(*FString::Printf(TEXT("%%.%df"), Layout.GetPrecision()), HY);
					const FVector2D YAxisMajorTickLabelPos(
						Layout.InnerRect.X0 + Layout.ScrollOffset + Layout.GetYAxisLabelAreaWidth() - Layout.MeasureTextWidth(Label, Layout.ChartDefaultFont) - 2.0f,
						Layout.InnerRect.Y0 + YMajor - Layout.MaxFontCharHeight + 2.0f);

					FSlateDrawElement::MakeText
					(
						OutDrawElements,
						ChartYAxisTextForegroundLayerId,
						AllottedGeometry.ToPaintGeometry(YAxisMajorTickLabelPos, Layout.YAxisLabelAreaSize),
						Label,
						Layout.ChartDefaultFont,
						DrawEffects,
						InWidgetStyle.GetColorAndOpacityTint() * FLinearColor(1.0f, 1.0f, 1.0f, alphaTween)
					);

					HY += Layout.GetYAxisMajorStepSize();
				}
			}
		}
		LineBuffer0.Empty();

		// Zero-Axis
		{
			const float YZeroPos = Layout.GetYPos(0.0f);

			LineBuffer0.Empty(2);
			new (LineBuffer0)FVector2D(Layout.ScrollOffset + Layout.GetXPos0(), YZeroPos);
			new (LineBuffer0)FVector2D(Layout.GetXPos1() + 4.0f, YZeroPos);


			// this will fade out the grid line and label if near index line
			float alphaTween = 1.0f;
			if (this->IsIndexed == true && UStatsTracerEditorSettings::GetInstance()->ChartShowIndicatorCross == true)
			{
				const float Index2LabelDiff = FMath::Abs(this->MousePosition.Y - YZeroPos);

				if (Index2LabelDiff < Layout.MaxFontCharHeight)
					alphaTween = (Index2LabelDiff / Layout.MaxFontCharHeight);
			}

			FSlateDrawElement::MakeLines
			(
				OutDrawElements,
				ChartGridLayerId,
				AllottedGeometry.ToPaintGeometry(),
				LineBuffer0,
				DrawEffects,
				InWidgetStyle.GetColorAndOpacityTint() * FLinearColor(1.0f, 1.0f, 1.0f, alphaTween),
				false,
				2.0f
			);

			// Draw ZERO
			
			const FVector2D YAxisMajorTickLabelPos(
				Layout.InnerRect.X0 + Layout.ScrollOffset + Layout.GetYAxisLabelAreaWidth() - Layout.MeasureTextWidth("0", Layout.ChartIndexLableFont) - 2.0f,
				Layout.InnerRect.Y0 + YZeroPos - Layout.MaxFontCharHeight + 2.0f);

			FSlateDrawElement::MakeText
			(
				OutDrawElements,
				ChartYAxisTextForegroundLayerId,
				AllottedGeometry.ToPaintGeometry(YAxisMajorTickLabelPos, Layout.YAxisLabelAreaSize),
				FString::Printf(TEXT("0")),
				Layout.ChartIndexLableFont,
				DrawEffects,
				InWidgetStyle.GetColorAndOpacityTint() * FLinearColor(1.0f, 1.0f, 1.0f, alphaTween)
			);
		}
		LineBuffer0.Empty();
	}

	// Chart Layer
	;{
		//ParallelFor(this->TracerDataChart->GetDataSourceCount(), [this, Layout](int32 i)
		for (int i = 0; i < this->TracerDataChart->GetDataSourceCount(); ++i)
		{
			// only care about visible data sources
			if ((*this->TracerDataChart)[i]->IsVisible == false)
				continue;

			StatsTracer::TDataSourceHandle dataSourceHandle = (*this->TracerDataChart)[i]->DataSource;

			TArray<FVector2D> TimePlotValues;
			TimePlotValues.Empty(Layout.SampleWindowSize);

			switch (dataSourceHandle->GetDataSourceType())
			{
				case StatsTracer::Bool:
				{
					auto ds = dataSourceHandle->GetAs<StatsTracer::FBoolDataSource>();
					for (uint32 t = 0; t < ds->GetSampleCount(); ++t)
						new (TimePlotValues)FVector2D(Layout.GetXPos(t), Layout.GetYPos((*ds)[t].Value));

					// plot it!
					FSlateDrawElement::MakeLines
					(
						OutDrawElements,
						ChartPlotLayerId,
						AllottedGeometry.ToPaintGeometry(),
						TimePlotValues,
						DrawEffects,
						InWidgetStyle.GetColorAndOpacityTint() * ds->GetColor().ReinterpretAsLinear(),
						false
					);

					break;
				}

				case StatsTracer::Int:
				{
					auto ds = dataSourceHandle->GetAs<StatsTracer::FIntDataSource>();

					for (uint32 t = 0; t < ds->GetSampleCount(); ++t)
						new (TimePlotValues)FVector2D(Layout.GetXPos(t), Layout.GetYPos((*ds)[t].Value));

					// plot it!
					FSlateDrawElement::MakeLines
					(
						OutDrawElements,
						ChartPlotLayerId,
						AllottedGeometry.ToPaintGeometry(),
						TimePlotValues,
						DrawEffects,
						InWidgetStyle.GetColorAndOpacityTint() * ds->GetColor().ReinterpretAsLinear(),
						false
					);

					break;
				}

				case StatsTracer::Float:
				{
					auto ds = dataSourceHandle->GetAs<StatsTracer::FFloatDataSource>();
					for (uint32 t = 0; t < ds->GetSampleCount(); ++t)
						new (TimePlotValues)FVector2D(Layout.GetXPos(t), Layout.GetYPos((*ds)[t].Value));

					// plot it!
					FSlateDrawElement::MakeLines
					(
						OutDrawElements,
						ChartPlotLayerId,
						AllottedGeometry.ToPaintGeometry(),
						TimePlotValues,
						DrawEffects,
						InWidgetStyle.GetColorAndOpacityTint() * ds->GetColor().ReinterpretAsLinear(),
						false
					);

					break;
				}

				case StatsTracer::Byte:
				{
					auto ds = dataSourceHandle->GetAs<StatsTracer::FByteDataSource>();
					for (uint32 t = 0; t < ds->GetSampleCount(); ++t)
						new (TimePlotValues)FVector2D(Layout.GetXPos(t), Layout.GetYPos((*ds)[t].Value));

					// plot it!
					FSlateDrawElement::MakeLines
					(
						OutDrawElements,
						ChartPlotLayerId,
						AllottedGeometry.ToPaintGeometry(),
						TimePlotValues,
						DrawEffects,
						InWidgetStyle.GetColorAndOpacityTint() * ds->GetColor().ReinterpretAsLinear(),
						false
					);
					break;
				}
			
				case StatsTracer::Vector:
				{

					auto li = (FVectorTracerDataSourceListItem*)(*this->TracerDataChart)[i].Get();
					auto ds = dataSourceHandle->GetAs<StatsTracer::FVectorDataSource>();


					// plot X
					if (li->ShowX == true)
					{
						TimePlotValues.Empty();

						for (uint32 t = 0; t < ds->GetSampleCount(); ++t)
							new (TimePlotValues)FVector2D(Layout.GetXPos(t), Layout.GetYPos((*ds)[t].Value.X));

						FSlateDrawElement::MakeLines
						(
							OutDrawElements,
							ChartPlotLayerId,
							AllottedGeometry.ToPaintGeometry(),
							TimePlotValues,
							DrawEffects,
							InWidgetStyle.GetColorAndOpacityTint() * ds->GetColor().ReinterpretAsLinear(),
							false,
							1.0f
						);

						// draw an label along the index line
						DrawIndexAidLabels(
							(*ds)[FMath::Clamp<int32>(this->BufferIndex, 0, ds->GetSampleCount() - 1)].Value.X,
							ds->GetColor(),
							TEXT(".X"),
							AllottedGeometry,
							OutDrawElements,
							DrawEffects,
							ChartIndexLayerId,
							Layout);
					}

					// plot Y
					if (li->ShowY == true)
					{
						TimePlotValues.Empty();

						for (uint32 t = 0; t < ds->GetSampleCount(); ++t)
							new (TimePlotValues)FVector2D(Layout.GetXPos(t), Layout.GetYPos((*ds)[t].Value.Y));

						FSlateDrawElement::MakeLines
						(
							OutDrawElements,
							ChartPlotLayerId,
							AllottedGeometry.ToPaintGeometry(),
							TimePlotValues,
							DrawEffects,
							InWidgetStyle.GetColorAndOpacityTint() * ds->GetColor().ReinterpretAsLinear(),
							false,
							1.0f
						);

						// draw an label along the index line
						DrawIndexAidLabels(
							(*ds)[FMath::Clamp<int32>(this->BufferIndex, 0, ds->GetSampleCount() - 1)].Value.Y,
							ds->GetColor(),
							TEXT(".Y"),
							AllottedGeometry,
							OutDrawElements,
							DrawEffects,
							ChartIndexLayerId,
							Layout);
					}

					// plot Z
					if (li->ShowZ == true)
					{
						TimePlotValues.Empty();

						for (uint32 t = 0; t < ds->GetSampleCount(); ++t)
							new (TimePlotValues)FVector2D(Layout.GetXPos(t), Layout.GetYPos((*ds)[t].Value.Z));

						FSlateDrawElement::MakeLines
						(
							OutDrawElements,
							ChartPlotLayerId,
							AllottedGeometry.ToPaintGeometry(),
							TimePlotValues,
							DrawEffects,
							InWidgetStyle.GetColorAndOpacityTint() * ds->GetColor().ReinterpretAsLinear(),
							false,
							1.0f
						);

						// draw an label along the index line
						DrawIndexAidLabels(
							(*ds)[FMath::Clamp<int32>(this->BufferIndex, 0, ds->GetSampleCount() - 1)].Value.Z,
							ds->GetColor(),
							TEXT(".Z"),
							AllottedGeometry,
							OutDrawElements,
							DrawEffects,
							ChartIndexLayerId,
							Layout);
					}

					break;
				}

				case StatsTracer::Rotator:
				{
					auto li = (FVectorTracerDataSourceListItem*)(*this->TracerDataChart)[i].Get();
					auto ds = dataSourceHandle->GetAs<StatsTracer::FRotatorDataSource>();


					// plot X
					if (li->ShowX == true)
					{
						TimePlotValues.Empty();

						for (uint32 t = 0; t < ds->GetSampleCount(); ++t)
							new (TimePlotValues)FVector2D(Layout.GetXPos(t), Layout.GetYPos((*ds)[t].Value.Roll));

						FSlateDrawElement::MakeLines
						(
							OutDrawElements,
							ChartPlotLayerId,
							AllottedGeometry.ToPaintGeometry(),
							TimePlotValues,
							DrawEffects,
							InWidgetStyle.GetColorAndOpacityTint() * ds->GetColor().ReinterpretAsLinear(),
							false,
							1.0f
						);

						// draw an label along the index line
						DrawIndexAidLabels(
							(*ds)[FMath::Clamp<int32>(this->BufferIndex, 0, ds->GetSampleCount() - 1)].Value.Roll,
							ds->GetColor(),
							TEXT(".Roll"),
							AllottedGeometry,
							OutDrawElements,
							DrawEffects,
							ChartIndexLayerId,
							Layout);
					}

					// plot Y
					if (li->ShowY == true)
					{
						TimePlotValues.Empty();

						for (uint32 t = 0; t < ds->GetSampleCount(); ++t)
							new (TimePlotValues)FVector2D(Layout.GetXPos(t), Layout.GetYPos((*ds)[t].Value.Pitch));

						FSlateDrawElement::MakeLines
						(
							OutDrawElements,
							ChartPlotLayerId,
							AllottedGeometry.ToPaintGeometry(),
							TimePlotValues,
							DrawEffects,
							InWidgetStyle.GetColorAndOpacityTint() * ds->GetColor().ReinterpretAsLinear(),
							false,
							1.0f
						);

						// draw an label along the index line
						DrawIndexAidLabels(
							(*ds)[FMath::Clamp<int32>(this->BufferIndex, 0, ds->GetSampleCount() - 1)].Value.Pitch,
							ds->GetColor(),
							TEXT(".Pitch"),
							AllottedGeometry,
							OutDrawElements,
							DrawEffects,
							ChartIndexLayerId,
							Layout);
					}

					// plot Z
					if (li->ShowZ == true)
					{
						TimePlotValues.Empty();

						for (uint32 t = 0; t < ds->GetSampleCount(); ++t)
							new (TimePlotValues)FVector2D(Layout.GetXPos(t), Layout.GetYPos((*ds)[t].Value.Yaw));

						FSlateDrawElement::MakeLines
						(
							OutDrawElements,
							ChartPlotLayerId,
							AllottedGeometry.ToPaintGeometry(),
							TimePlotValues,
							DrawEffects,
							InWidgetStyle.GetColorAndOpacityTint() * ds->GetColor().ReinterpretAsLinear(),
							false,
							1.0f
						);

						// draw an label along the index line
						DrawIndexAidLabels(
							(*ds)[FMath::Clamp<int32>(this->BufferIndex, 0, ds->GetSampleCount() - 1)].Value.Yaw,
							ds->GetColor(),
							TEXT(".Yaw"),
							AllottedGeometry,
							OutDrawElements,
							DrawEffects,
							ChartIndexLayerId,
							Layout);
					}

					break;
				}

				case StatsTracer::Transform:
				{
					auto li = (FTransformTracerDataSourceListItem*)(*this->TracerDataChart)[i].Get();
					auto ds = dataSourceHandle->GetAs<StatsTracer::FTransformDataSource>();

					// LOCATION
					if (li->ShowP == true)
					{
						// plot PX
						if (li->ShowPX == true)
						{
							TimePlotValues.Empty();

							for (uint32 t = 0; t < ds->GetSampleCount(); ++t)
								new (TimePlotValues)FVector2D(Layout.GetXPos(t), Layout.GetYPos((*ds)[t].Value.GetLocation().X));

							FSlateDrawElement::MakeLines
							(
								OutDrawElements,
								ChartPlotLayerId,
								AllottedGeometry.ToPaintGeometry(),
								TimePlotValues,
								DrawEffects,
								InWidgetStyle.GetColorAndOpacityTint() * ds->GetColor().ReinterpretAsLinear(),
								false,
								1.0f
							);

							// draw an label along the index line
							DrawIndexAidLabels(
								(*ds)[FMath::Clamp<int32>(this->BufferIndex, 0, ds->GetSampleCount() - 1)].Value.GetLocation().X,
								ds->GetColor(),
								TEXT(".Location.X"),
								AllottedGeometry,
								OutDrawElements,
								DrawEffects,
								ChartIndexLayerId,
								Layout);
						}

						// plot PY
						if (li->ShowPY == true)
						{
							TimePlotValues.Empty();

							for (uint32 t = 0; t < ds->GetSampleCount(); ++t)
								new (TimePlotValues)FVector2D(Layout.GetXPos(t), Layout.GetYPos((*ds)[t].Value.GetLocation().Y));

							FSlateDrawElement::MakeLines
							(
								OutDrawElements,
								ChartPlotLayerId,
								AllottedGeometry.ToPaintGeometry(),
								TimePlotValues,
								DrawEffects,
								InWidgetStyle.GetColorAndOpacityTint() * ds->GetColor().ReinterpretAsLinear(),
								false,
								1.0f
							);

							// draw an label along the index line
							DrawIndexAidLabels(
								(*ds)[FMath::Clamp<int32>(this->BufferIndex, 0, ds->GetSampleCount() - 1)].Value.GetLocation().Y,
								ds->GetColor(),
								TEXT(".Location.Y"),
								AllottedGeometry,
								OutDrawElements,
								DrawEffects,
								ChartIndexLayerId,
								Layout);
						}

						// plot PZ
						if (li->ShowPZ == true)
						{
							TimePlotValues.Empty();

							for (uint32 t = 0; t < ds->GetSampleCount(); ++t)
								new (TimePlotValues)FVector2D(Layout.GetXPos(t), Layout.GetYPos((*ds)[t].Value.GetLocation().Z));

							FSlateDrawElement::MakeLines
							(
								OutDrawElements,
								ChartPlotLayerId,
								AllottedGeometry.ToPaintGeometry(),
								TimePlotValues,
								DrawEffects,
								InWidgetStyle.GetColorAndOpacityTint() * ds->GetColor().ReinterpretAsLinear(),
								false,
								1.0f
							);

							// draw an label along the index line
							DrawIndexAidLabels(
								(*ds)[FMath::Clamp<int32>(this->BufferIndex, 0, ds->GetSampleCount() - 1)].Value.GetLocation().Z,
								ds->GetColor(),
								TEXT(".Location.Z"),
								AllottedGeometry,
								OutDrawElements,
								DrawEffects,
								ChartIndexLayerId,
								Layout);
						}
					}

					// ROTATION
					if (li->ShowR == true)
					{
						// plot RX
						if (li->ShowRX == true)
						{
							TimePlotValues.Empty();

							for (uint32 t = 0; t < ds->GetSampleCount(); ++t)
								new (TimePlotValues)FVector2D(Layout.GetXPos(t), Layout.GetYPos((*ds)[t].Value.GetRotation().Rotator().Roll));

							FSlateDrawElement::MakeLines
							(
								OutDrawElements,
								ChartPlotLayerId,
								AllottedGeometry.ToPaintGeometry(),
								TimePlotValues,
								DrawEffects,
								InWidgetStyle.GetColorAndOpacityTint() * ds->GetColor().ReinterpretAsLinear(),
								false,
								1.0f
							);

							// draw an label along the index line
							DrawIndexAidLabels(
								(*ds)[FMath::Clamp<int32>(this->BufferIndex, 0, ds->GetSampleCount() - 1)].Value.GetRotation().Rotator().Roll,
								ds->GetColor(),
								TEXT(".Rotation.Roll"),
								AllottedGeometry,
								OutDrawElements,
								DrawEffects,
								ChartIndexLayerId,
								Layout);
						}

						// plot RY
						if (li->ShowRY == true)
						{
							TimePlotValues.Empty();

							for (uint32 t = 0; t < ds->GetSampleCount(); ++t)
								new (TimePlotValues)FVector2D(Layout.GetXPos(t), Layout.GetYPos((*ds)[t].Value.GetRotation().Rotator().Pitch));

							FSlateDrawElement::MakeLines
							(
								OutDrawElements,
								ChartPlotLayerId,
								AllottedGeometry.ToPaintGeometry(),
								TimePlotValues,
								DrawEffects,
								InWidgetStyle.GetColorAndOpacityTint() * ds->GetColor().ReinterpretAsLinear(),
								false,
								1.0f
							);

							// draw an label along the index line
							DrawIndexAidLabels(
								(*ds)[FMath::Clamp<int32>(this->BufferIndex, 0, ds->GetSampleCount() - 1)].Value.GetRotation().Rotator().Pitch,
								ds->GetColor(),
								TEXT(".Rotation.Pitch"),
								AllottedGeometry,
								OutDrawElements,
								DrawEffects,
								ChartIndexLayerId,
								Layout);
						}

						// plot RZ
						if (li->ShowRZ == true)
						{
							TimePlotValues.Empty();

							for (uint32 t = 0; t < ds->GetSampleCount(); ++t)
								new (TimePlotValues)FVector2D(Layout.GetXPos(t), Layout.GetYPos((*ds)[t].Value.GetRotation().Rotator().Yaw));

							FSlateDrawElement::MakeLines
							(
								OutDrawElements,
								ChartPlotLayerId,
								AllottedGeometry.ToPaintGeometry(),
								TimePlotValues,
								DrawEffects,
								InWidgetStyle.GetColorAndOpacityTint() * ds->GetColor().ReinterpretAsLinear(),
								false,
								1.0f
							);

							// draw an label along the index line
							DrawIndexAidLabels(
								(*ds)[FMath::Clamp<int32>(this->BufferIndex, 0, ds->GetSampleCount() - 1)].Value.GetRotation().Rotator().Yaw,
								ds->GetColor(),
								TEXT(".Rotation.Yaw"),
								AllottedGeometry,
								OutDrawElements,
								DrawEffects,
								ChartIndexLayerId,
								Layout);
						}
					}

					// SCALE
					if (li->ShowS == true)
					{
						// plot SX
						if (li->ShowSX == true)
						{
							TimePlotValues.Empty();

							for (uint32 t = 0; t < ds->GetSampleCount(); ++t)
								new (TimePlotValues)FVector2D(Layout.GetXPos(t), Layout.GetYPos((*ds)[t].Value.GetScale3D().X));

							FSlateDrawElement::MakeLines
							(
								OutDrawElements,
								ChartPlotLayerId,
								AllottedGeometry.ToPaintGeometry(),
								TimePlotValues,
								DrawEffects,
								InWidgetStyle.GetColorAndOpacityTint() * ds->GetColor().ReinterpretAsLinear(),
								false,
								1.0f
							);

							// draw an label along the index line
							DrawIndexAidLabels(
								(*ds)[FMath::Clamp<int32>(this->BufferIndex, 0, ds->GetSampleCount() - 1)].Value.GetScale3D().X,
								ds->GetColor(),
								TEXT(".Scale.X"),
								AllottedGeometry,
								OutDrawElements,
								DrawEffects,
								ChartIndexLayerId,
								Layout);
						}

						// plot SY
						if (li->ShowSY == true)
						{
							TimePlotValues.Empty();

							for (uint32 t = 0; t < ds->GetSampleCount(); ++t)
								new (TimePlotValues)FVector2D(Layout.GetXPos(t), Layout.GetYPos((*ds)[t].Value.GetScale3D().Y));

							FSlateDrawElement::MakeLines
							(
								OutDrawElements,
								ChartPlotLayerId,
								AllottedGeometry.ToPaintGeometry(),
								TimePlotValues,
								DrawEffects,
								InWidgetStyle.GetColorAndOpacityTint() * ds->GetColor().ReinterpretAsLinear(),
								false,
								1.0f
							);

							// draw an label along the index line
							DrawIndexAidLabels(
								(*ds)[FMath::Clamp<int32>(this->BufferIndex, 0, ds->GetSampleCount() - 1)].Value.GetScale3D().Y,
								ds->GetColor(),
								TEXT(".Scale.Y"),
								AllottedGeometry,
								OutDrawElements,
								DrawEffects,
								ChartIndexLayerId,
								Layout);
						}

						// plot SZ
						if (li->ShowSZ == true)
						{
							TimePlotValues.Empty();

							for (uint32 t = 0; t < ds->GetSampleCount(); ++t)
								new (TimePlotValues)FVector2D(Layout.GetXPos(t), Layout.GetYPos((*ds)[t].Value.GetScale3D().Z));

							FSlateDrawElement::MakeLines
							(
								OutDrawElements,
								ChartPlotLayerId,
								AllottedGeometry.ToPaintGeometry(),
								TimePlotValues,
								DrawEffects,
								InWidgetStyle.GetColorAndOpacityTint() * ds->GetColor().ReinterpretAsLinear(),
								false,
								1.0f
							);

							// draw an label along the index line
							DrawIndexAidLabels(
								(*ds)[FMath::Clamp<int32>(this->BufferIndex, 0, ds->GetSampleCount() - 1)].Value.GetScale3D().Z,
								ds->GetColor(),
								TEXT(".Scale.Z"),
								AllottedGeometry,
								OutDrawElements,
								DrawEffects,
								ChartIndexLayerId,
								Layout);
						}
					}

					break;
				}
			}
			TimePlotValues.Empty();

		}//);
	}

	// X/YAxis Label Background Layer
	;{
		// XAxis
		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			ChartXAxisTextBackgroundLayerId,
			AllottedGeometry.ToPaintGeometry(
				FVector2D(Layout.InnerRect.X0, Layout.InnerRect.Y1),
				FVector2D(Layout.OutterRect.X1 - Layout.InnerRect.X0, Layout.OutterRect.Y1 - Layout.InnerRect.Y1)),
			ChartAxisLabelBackgroundBrush,
			DrawEffects
		);

		// YAxis
		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			ChartYAxisTextBackgroundLayerId,
			AllottedGeometry.ToPaintGeometry(
				FVector2D(Layout.OutterRect.X0 + Layout.ScrollOffset, Layout.OutterRect.Y0),
				FVector2D(Layout.GetYAxisLabelAreaWidth() + Layout.InnerRect.X0, Layout.OutterRect.Y1)),
			ChartAxisLabelBackgroundBrush,
			DrawEffects
		);
	}

	// Draw Index Lines and Labels
	if (this->IsIndexed == true && UStatsTracerEditorSettings::GetInstance()->ChartShowIndicatorCross == true)
	{
		const bool bDrawVertical	= (this->MousePosition.X >= Layout.GetXPos0() && this->MousePosition.X <= Layout.GetXPos1());
		const bool bDrawHorizontal	= (this->MousePosition.Y >= Layout.InnerRect.Y0 && this->MousePosition.Y <= Layout.InnerRect.Y1);

		const int32 HX				= (int32)((this->MousePosition.X - Layout.GetXPos0()) / Layout.SampleDataXOffset);
		const float XMajor			= this->MousePosition.X;

		// Vertical index line
		if (bDrawVertical == true)
		{
			LineBuffer0.Empty(2);
			new (LineBuffer0)FVector2D(XMajor, Layout.InnerRect.Y0);
			new (LineBuffer0)FVector2D(XMajor, Layout.InnerRect.Y1);

			FSlateDrawElement::MakeLines
			(
				OutDrawElements,
				ChartIndexLayerId,
				AllottedGeometry.ToPaintGeometry(),
				LineBuffer0,
				DrawEffects,
				InWidgetStyle.GetColorAndOpacityTint() * FLinearColor(1.0f, 1.0f, 1.0f, 1.0f),
				false
			);


			uint64 frameNumber = HX * this->TracerDataChart->GetSampleUpdateFrequency();

			StatsTracer::TDataSourceHandle dataSourceHandle = (*this->TracerDataChart)[0]->DataSource;
			if(dataSourceHandle->GetSampleCount() == dataSourceHandle->GetSampleWindowSize())
				frameNumber = dataSourceHandle->GetFrameNumber(HX);	



			// Draw Axis-Tick Value
			const FString Label = XFormaterReferencePointer.GetLabelForIndex(HX);
			float LabelWidht = Layout.MeasureTextWidth(Label, Layout.ChartIndexLableFont);

			const FVector2D XAxisMajorTickLabelPos(
				Layout.InnerRect.X0 + XMajor - (LabelWidht * 0.66f),
				Layout.InnerRect.Y1 + 2.0f);

			FSlateDrawElement::MakeText
			(
				OutDrawElements,
				ChartIndexLayerId,
				AllottedGeometry.ToPaintGeometry(XAxisMajorTickLabelPos, FVector2D(LabelWidht, Layout.MaxFontCharHeight)),
				Label,
				Layout.ChartIndexLableFont,
				DrawEffects,
				InWidgetStyle.GetColorAndOpacityTint() * FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)
			);

			// remember index
			const_cast<STracerDataChart::SPlotArea*>(this)->BufferIndex = HX;
		}	

		// Horizontal index line
		if (bDrawHorizontal == true)
		{
			LineBuffer0.Empty(2);
			new (LineBuffer0)FVector2D(Layout.InnerRect.X0 + Layout.GetYAxisLabelAreaWidth() + Layout.ScrollOffset, this->MousePosition.Y);
			new (LineBuffer0)FVector2D(Layout.OutterRect.X1, this->MousePosition.Y);

			FSlateDrawElement::MakeLines
			(
				OutDrawElements,
				ChartIndexLayerId,
				AllottedGeometry.ToPaintGeometry(),
				LineBuffer0,
				DrawEffects,
				InWidgetStyle.GetColorAndOpacityTint() * FLinearColor(1.0f, 1.0f, 1.0f, 1.0f),
				false
			);
	
			// Draw Axis-Tick Value 
			const FString Label = FString::Printf(*FString::Printf(TEXT("%%.%df"), Layout.GetPrecision()), ((((Layout.GetYPos1() - Layout.GetYPos0()) * (1.0f - ((this->MousePosition.Y - Layout.InnerRect.Y0) / (Layout.InnerRect.Y1 - Layout.InnerRect.Y0))))) + Layout.GetYPos0()));
			const FVector2D YAxisMajorTickLabelPos(
				Layout.InnerRect.X0 + Layout.ScrollOffset + Layout.GetYAxisLabelAreaWidth() - Layout.MeasureTextWidth(Label, Layout.ChartIndexLableFont) - 2.0f,
				Layout.InnerRect.Y0 + this->MousePosition.Y - Layout.MaxFontCharHeight + 2.0f);

			FSlateDrawElement::MakeText
			(
				OutDrawElements,
				ChartIndexLayerId,
				AllottedGeometry.ToPaintGeometry(YAxisMajorTickLabelPos, Layout.YAxisLabelAreaSize),
				Label,
				Layout.ChartIndexLableFont,
				DrawEffects,
				InWidgetStyle.GetColorAndOpacityTint() * FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)
			);
		}
	}

	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled && IsEnabled());
}


void STracerDataChart::SPlotArea::DrawIndexAidLabels(
	float										value, 
	const FColor&								color, 
	const FString&								text,
	const FGeometry&							AllottedGeometry, 
	FSlateWindowElementList&					OutDrawElements, 
	const ESlateDrawEffect						DrawEffects,
	const uint32								LayerId,
	STracerDataChart::SPlotArea::TChartLayout&	Layout) const
{
	if (UStatsTracerEditorSettings::GetInstance()->ChartShowMultiStatsSubLabels == true && this->IsIndexed == true)
	{
		const TSharedRef<FSlateFontMeasure>	FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		const FVector2D TextSize = FontMeasureService->Measure(text, Layout.ChartAidLableFont);

		const float LabelXPos = FMath::Clamp<int32>(
			Layout.InnerRect.X0 + this->MousePosition.X,
			Layout.InnerRect.X0 + Layout.GetYAxisLabelAreaWidth(),
			Layout.GetXPos((*this->TracerDataChart)[0]->DataSource->GetSampleCount()));

		const float LabelYPos = Layout.GetYPos(value);

		float XOffset = 0.0f; // by default draw label to right hand side
		float YOffset = 0.0f; // by default draw label under the plotted line

		if (Layout.GetXPos1() - LabelXPos < 100.0f)
			XOffset = 12.0f + TextSize.X; // if not enough space, draw label on the left hand side

		if (Layout.InnerRect.Y1 - LabelYPos < TextSize.Y)
			YOffset = TextSize.Y; // if not enough space, draw label above the plotted line

		FSlateDrawElement::MakeText
		(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(FVector2D(LabelXPos - XOffset, LabelYPos - YOffset), FVector2D(TextSize.X, Layout.MaxFontCharHeight)),
			//AllottedGeometry.ToPaintGeometry(FVector2D(TextSize.X, Layout.MaxFontCharHeight), FSlateLayoutTransform(FVector2D(LabelXPos - XOffset, LabelYPos - YOffset))),
			text,
			Layout.ChartAidLableFont,
			DrawEffects,
			FLinearColor(color)
		);
	}
};

FReply STracerDataChart::SPlotArea::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	this->MousePosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	return FReply::Handled();
}

void STracerDataChart::SPlotArea::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	this->IsIndexed = true;
}

void STracerDataChart::SPlotArea::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	this->IsIndexed = false;

	for (auto& DSItem : this->TracerDataChart->TracerDataSourceListItems)
		if (DSItem.IsValid())
			DSItem->DataSourceIndex = -1;
}

//-----------------------------------------------------//
// WARNING: Heavy GUI SLATE STUFF! BETTER NOT TOUCH!   //
//-----------------------------------------------------//

// bool, int, float, byte
TSharedRef<SWidget> STracerDataChart::FSimpleTracerDataSourceListItem::BuildWidget(STracerDataListItemWidget* parent)
{
	return SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.Padding(FMargin(4.0f))
		[
			SNew(SHorizontalBox)
			
			// DataSource visibility
			+SHorizontalBox::Slot()
			.Padding(0.0f, 0.0f, 17.0f, 0.0f)
			.AutoWidth()
			.HAlign(HAlign_Left)
			[
				SNew(SChartPlotVisibilityToggle)
				.InitialVisible(this->IsVisible)
				.OnToggleVisibility_Lambda([this](bool visible) {this->IsVisible = visible; })
			]
			// DataSource color
			+SHorizontalBox::Slot()
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			.AutoWidth()
			[
				SNew(SBox)
				.ToolTipText_Lambda([this]()
				{
					return this->DataSource->GetDescription();
				})
				.WidthOverride(32)
				.HeightOverride(8)	
				[
					SNew(SButton)		
					.ButtonColorAndOpacity_Lambda([this]() { return FSlateColor(this->DataSource->GetColor().ReinterpretAsLinear()); })
					.OnClicked_Lambda([this]() 
					{
						FColorPickerArgs args;
						{
							args.InitialColorOverride = this->DataSource->GetColor().ReinterpretAsLinear();
							args.bUseAlpha = false;
							args.OnColorCommitted.BindLambda([this](FLinearColor InColor) { this->DataSource->GetColor() = InColor.ToFColor(true); });
						}
						OpenColorPicker(args);
	
						return FReply::Handled();
					})
				]
			]	
	
			// DataSource name
			+SHorizontalBox::Slot()
			.FillWidth(1.0)
			[
				SNew(STextBlock)
				.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont"))
				.Text(FText::FromString(this->DataSource->GetName()))
			]
	
			// DataSource value
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(STextBlock)
				.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
				.Text_Lambda([this]()
				{
					if (this->DataSourceIndex < 0 || this->DataSource.IsValid() == false || this->IsVisible == false)
						return FText::FromString("");

					FString valueStr(TEXT("#ERR-VAL"));
					switch (this->DataSource->GetDataSourceType())
					{
						case StatsTracer::Bool:
						{
							auto ds = this->DataSource->GetAs<StatsTracer::FBoolDataSource>();
							if (ds != nullptr)
								valueStr = ((*ds)[this->DataSourceIndex].Value == true ? TEXT("true") : TEXT("false"));

							break;
						}
						case StatsTracer::Int:
						{
							auto ds = this->DataSource->GetAs<StatsTracer::FIntDataSource>();
							if (ds != nullptr)
								valueStr = FString::Printf(TEXT("%d"), (*ds)[this->DataSourceIndex].Value);

							break;
						}
						case StatsTracer::Float:
						{
							auto ds = this->DataSource->GetAs<StatsTracer::FFloatDataSource>();
							if (ds != nullptr)
								valueStr = FString::Printf(TEXT("%f"), (*ds)[this->DataSourceIndex].Value);

							break;
						}
						case StatsTracer::Byte:
						{
							auto ds = this->DataSource->GetAs<StatsTracer::FByteDataSource>();
							if (ds != nullptr)
								valueStr = FString::Printf(TEXT("%u"), (*ds)[this->DataSourceIndex].Value);

							break;
						}
					}

					return FText::FromString(valueStr);
				})
			]
		];
}

// FVector, FRotator
TSharedRef<SWidget> STracerDataChart::FVectorTracerDataSourceListItem::BuildWidget(STracerDataListItemWidget* parent)
{
	return 
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		[ 
			SNew(SBox)
			.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
			[
				SNew(SChartPlotVisibilityToggle)
				.InitialVisible(this->IsVisible)
				.OnToggleVisibility_Lambda([this](bool visible) {this->IsVisible = visible; })
			]
		]

		+SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SExpandableArea)
			.InitiallyCollapsed(this->IsExpanded == false)
			.OnAreaExpansionChanged_Lambda([this, parent](bool state) { parent->Refresh(); this->IsExpanded = state; })
			.Padding(FMargin(-16.0f, 2.0f, 0.0f, 0.0f))
			.HeaderContent()
			[
				SNew(SHorizontalBox)
		
				// DataSource color
				+SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, 4.0f, 0.0f)
				.AutoWidth()
				[
					SNew(SBox)
					.ToolTipText_Lambda([this]()
					{
						return this->DataSource->GetDescription();
					})
					.WidthOverride(32)
					.HeightOverride(8)	
					[
						SNew(SButton)		
						.ButtonColorAndOpacity_Lambda([this]() { return FSlateColor(this->DataSource->GetColor().ReinterpretAsLinear()); })
						.OnClicked_Lambda([this]() 
						{
	
							FColorPickerArgs args;
							{
								args.InitialColorOverride = this->DataSource->GetColor().ReinterpretAsLinear();
								args.bUseAlpha = false;
								args.OnColorCommitted.BindLambda([this](FLinearColor InColor) { this->DataSource->GetColor() = InColor.ToFColor(true); });
							}
							OpenColorPicker(args);
	
							return FReply::Handled();
						})
					]
				]	
	
				// DataSource name
				+SHorizontalBox::Slot()
				.FillWidth(1.0)
				[
					SNew(STextBlock)
					.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont"))
					.Text(FText::FromString(this->DataSource->GetName()))
				]
			]
			.BodyContent()
			[
				SNew(SVerticalBox)			
				+SVerticalBox::Slot() // X
				.Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.0f, 0.0f, 17.0f, 0.0f)
					.AutoWidth()
					.HAlign(HAlign_Left)
					[
						SNew(SChartPlotVisibilityToggle)
						.InitialVisible(this->ShowX)
						.OnToggleVisibility_Lambda([this](bool visible) {this->ShowX = visible; })
					]

					+SHorizontalBox::Slot() 
					.FillWidth(1.0f)
					[
						SNew(STextBlock)
						.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
						.Text(FText::FromString("X"))
					]

					+SHorizontalBox::Slot() 
					.HAlign(HAlign_Right)
					.AutoWidth()
					[
						SNew(STextBlock)
						.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
						.Text_Lambda([this]()
						{
							// visibility check
							if (this->DataSourceIndex < 0 
								|| this->DataSource.IsValid() == false 
								|| this->IsVisible == false
								|| this->ShowX == false)
								return FText::FromString("");

							FString valueStr(TEXT("#ERR-VAL"));

							if (this->DataSource->GetDataSourceType() == StatsTracer::Vector)
							{
								auto ds = this->DataSource->GetAs<StatsTracer::FVectorDataSource>();
								if (ds != nullptr)
									valueStr = FString::Printf(TEXT("%f"), (*ds)[this->DataSourceIndex].Value.X);
							}
							else
							{
								auto ds = this->DataSource->GetAs<StatsTracer::FRotatorDataSource>();
								if (ds != nullptr)
									valueStr = FString::Printf(TEXT("%f"), (*ds)[this->DataSourceIndex].Value.Roll);
							}

							return FText::FromString(valueStr);
						})
					]
				]

				+SVerticalBox::Slot() // Y
				.Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.0f, 0.0f, 17.0f, 0.0f)
					.AutoWidth()
					.HAlign(HAlign_Left)
					[
						SNew(SChartPlotVisibilityToggle)
						.InitialVisible(this->ShowY)
						.OnToggleVisibility_Lambda([this](bool visible) {this->ShowY = visible; })
					]

					+SHorizontalBox::Slot() 
					.FillWidth(1.0f)
					[
						SNew(STextBlock)
						.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
						.Text(FText::FromString("Y"))
					]

					+SHorizontalBox::Slot() 
					.HAlign(HAlign_Right)
					.AutoWidth()
					[
						SNew(STextBlock)
						.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
						.Text_Lambda([this]()
						{

							// visibility check
							if (this->DataSourceIndex < 0 
								|| this->DataSource.IsValid() == false 
								|| this->IsVisible == false
								|| this->ShowY == false)
								return FText::FromString("");

							FString valueStr(TEXT("#ERR-VAL"));

							if (this->DataSource->GetDataSourceType() == StatsTracer::Vector)
							{
								auto ds = this->DataSource->GetAs<StatsTracer::FVectorDataSource>();
								if (ds != nullptr)
									valueStr = FString::Printf(TEXT("%f"), (*ds)[this->DataSourceIndex].Value.Y);
							}
							else
							{
								auto ds = this->DataSource->GetAs<StatsTracer::FRotatorDataSource>();
								if (ds != nullptr)
									valueStr = FString::Printf(TEXT("%f"), (*ds)[this->DataSourceIndex].Value.Pitch);
							}

							return FText::FromString(valueStr);
						})
					]
				]

				+SVerticalBox::Slot() // Z
				.Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.0f, 0.0f, 17.0f, 0.0f)
					.AutoWidth()
					.HAlign(HAlign_Left)
					[
						SNew(SChartPlotVisibilityToggle)
						.InitialVisible(this->ShowZ)
						.OnToggleVisibility_Lambda([this](bool visible) {this->ShowZ = visible; })
					]

					+SHorizontalBox::Slot() 
					.FillWidth(1.0f)
					[
						SNew(STextBlock)
						.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
						.Text(FText::FromString("Z"))
					]

					+SHorizontalBox::Slot() 
					.HAlign(HAlign_Right)
					.AutoWidth()
					[
						SNew(STextBlock)
						.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
						.Text_Lambda([this]()
						{
							// visibility check
							if (this->DataSourceIndex < 0 
								|| this->DataSource.IsValid() == false 
								|| this->IsVisible == false
								|| this->ShowZ == false)
								return FText::FromString("");


							FString valueStr(TEXT("#ERR-VAL"));

							if (this->DataSource->GetDataSourceType() == StatsTracer::Vector)
							{
								auto ds = this->DataSource->GetAs<StatsTracer::FVectorDataSource>();
								if (ds != nullptr)
									valueStr = FString::Printf(TEXT("%f"), (*ds)[this->DataSourceIndex].Value.Z);
							}
							else
							{
								auto ds = this->DataSource->GetAs<StatsTracer::FRotatorDataSource>();
								if (ds != nullptr)
									valueStr = FString::Printf(TEXT("%f"), (*ds)[this->DataSourceIndex].Value.Yaw);
							}

							return FText::FromString(valueStr);
						})
					]
				]
			]
		];
}

// FTransform
TSharedRef<SWidget> STracerDataChart::FTransformTracerDataSourceListItem::BuildWidget(STracerDataListItemWidget* parent)
{
	return 

		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		[ 
			SNew(SBox)
			.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
			[
				SNew(SChartPlotVisibilityToggle)
				.InitialVisible(this->IsVisible)
				.OnToggleVisibility_Lambda([this](bool visible) {this->IsVisible = visible; })
			]
		]

		+SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SExpandableArea)
			.InitiallyCollapsed(this->IsExpanded == false)
			.OnAreaExpansionChanged_Lambda([this, parent](bool state) { parent->Refresh(); this->IsExpanded = state; })
			.Padding(FMargin(-20.0f, 2.0f, 0.0f, 0.0f))
			.HeaderContent()
			[
				SNew(SHorizontalBox)
			
				// DataSource color
				+SHorizontalBox::Slot()
				.Padding(0.0f, 0.0f, 4.0f, 0.0f)
				.AutoWidth()
				[
					SNew(SBox)
					.ToolTipText_Lambda([this]()
					{
						return this->DataSource->GetDescription();
					})
					.WidthOverride(32)
					.HeightOverride(8)	
					[
						SNew(SButton)		
						.ButtonColorAndOpacity_Lambda([this]() { return FSlateColor(this->DataSource->GetColor().ReinterpretAsLinear()); })
						.OnClicked_Lambda([this]() 
						{
	
							FColorPickerArgs args;
							{
								args.InitialColorOverride = this->DataSource->GetColor().ReinterpretAsLinear();
								args.bUseAlpha = false;
								args.OnColorCommitted.BindLambda([this](FLinearColor InColor) { this->DataSource->GetColor() = InColor.ToFColor(true); });
							}
							OpenColorPicker(args);
	
							return FReply::Handled();
						})
					]
				]	
	
				// DataSource name
				+SHorizontalBox::Slot()
				.FillWidth(1.0)
				[
					SNew(STextBlock)
					.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalBoldFont"))
					.Text(FText::FromString(this->DataSource->GetName()))
				]
			]
			.BodyContent()
			[
				SNew(SVerticalBox)

				//-------------------------------
				// Translation
				//-------------------------------
				+SVerticalBox::Slot() 
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Top)
					[ 
						SNew(SBox)
						.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
						[
							SNew(SChartPlotVisibilityToggle)
							.InitialVisible(this->ShowP)
							.OnToggleVisibility_Lambda([this](bool visible) {this->ShowP = visible; })
						]
					]

					+SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
					[
						SNew(SExpandableArea)
						.InitiallyCollapsed(this->IsPExpanded == false)
						.OnAreaExpansionChanged_Lambda([this, parent](bool state) { parent->Refresh(); this->IsPExpanded = state; })
						.Padding(FMargin(4.0f, 2.0f, 0.0f, 0.0f))
						.HeaderContent()
						[
							SNew(SHorizontalBox)

							+SHorizontalBox::Slot()
							.FillWidth(1.0)
							[ SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont")).Text(FText::FromString("Translation")) ]
						]
						.BodyContent()
						[
							SNew(SVerticalBox)			
							+SVerticalBox::Slot() // PX
							.Padding(FMargin(-28.0f, 0.0f, 8.0f, 0.0f))
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SChartPlotVisibilityToggle)
									.InitialVisible(this->ShowPX)
									.OnToggleVisibility_Lambda([this](bool visible) {this->ShowPX = visible; })
								]

								+SHorizontalBox::Slot() 
								.Padding(FMargin(26.0f, 0.0f, 8.0f, 0.0f))
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
									.Text(FText::FromString("X"))
								]

								+SHorizontalBox::Slot() 
								.HAlign(HAlign_Right)
								.AutoWidth()
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
									.Text_Lambda([this]()
									{
										// visibility check
										if (this->DataSourceIndex < 0 
											|| this->DataSource.IsValid() == false 
											|| this->IsVisible == false
											|| this->ShowP == false
											|| this->ShowPX == false)
											return FText::FromString("");

										FString valueStr(TEXT("#ERR-VAL"));
										auto ds = this->DataSource->GetAs<StatsTracer::FTransformDataSource>();
										if (ds != nullptr)
											valueStr = FString::Printf(TEXT("%f"), (*ds)[this->DataSourceIndex].Value.GetTranslation().X);

										return FText::FromString(valueStr);
									})
								]
							]

							+SVerticalBox::Slot() // Y
							.Padding(FMargin(-28.0f, 0.0f, 8.0f, 0.0f))
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SChartPlotVisibilityToggle)
									.InitialVisible(this->ShowPY)
									.OnToggleVisibility_Lambda([this](bool visible) {this->ShowPY = visible; })
								]

								+SHorizontalBox::Slot() 
								.Padding(FMargin(26.0f, 0.0f, 8.0f, 0.0f))
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
									.Text(FText::FromString("Y"))
								]

								+SHorizontalBox::Slot() 
								.HAlign(HAlign_Right)
								.AutoWidth()
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
									.Text_Lambda([this]()
									{
										// visibility check
										if (this->DataSourceIndex < 0 
											|| this->DataSource.IsValid() == false 
											|| this->IsVisible == false
											|| this->ShowP == false
											|| this->ShowPY == false)
											return FText::FromString("");

										FString valueStr(TEXT("#ERR-VAL"));
										auto ds = this->DataSource->GetAs<StatsTracer::FTransformDataSource>();
										if (ds != nullptr)
											valueStr = FString::Printf(TEXT("%f"), (*ds)[this->DataSourceIndex].Value.GetTranslation().Y);

										return FText::FromString(valueStr);
									})
								]
							]

							+SVerticalBox::Slot() // Z
							.Padding(FMargin(-28.0f, 0.0f, 8.0f, 0.0f))
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SChartPlotVisibilityToggle)
									.InitialVisible(this->ShowPZ)
									.OnToggleVisibility_Lambda([this](bool visible) {this->ShowPZ = visible; })
								]

								+SHorizontalBox::Slot() 
								.Padding(FMargin(26.0f, 0.0f, 8.0f, 0.0f))
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
									.Text(FText::FromString("Z"))
								]

								+SHorizontalBox::Slot() 
								.HAlign(HAlign_Right)
								.AutoWidth()
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
									.Text_Lambda([this]()
									{
										// visibility check
										if (this->DataSourceIndex < 0 
											|| this->DataSource.IsValid() == false 
											|| this->IsVisible == false
											|| this->ShowP == false
											|| this->ShowPZ == false)
											return FText::FromString("");

										FString valueStr(TEXT("#ERR-VAL"));
										auto ds = this->DataSource->GetAs<StatsTracer::FTransformDataSource>();
										if (ds != nullptr)
											valueStr = FString::Printf(TEXT("%f"), (*ds)[this->DataSourceIndex].Value.GetTranslation().Z);

										return FText::FromString(valueStr);
									})
								]
							]
						]
					]
				]

				//-------------------------------
				// Rotation
				//-------------------------------	
				+SVerticalBox::Slot() 
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[	
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Top)
					[ 
						SNew(SBox)
						.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
						[
							SNew(SChartPlotVisibilityToggle)
							.InitialVisible(this->ShowR)
							.OnToggleVisibility_Lambda([this](bool visible) {this->ShowR = visible; })
						]
					]

					+SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
					[
						SNew(SExpandableArea)
						.InitiallyCollapsed(this->IsPExpanded == false)
						.OnAreaExpansionChanged_Lambda([this, parent](bool state) { parent->Refresh(); this->IsRExpanded = state; })
						.Padding(FMargin(4.0f, 2.0f, 0.0f, 0.0f))
						.HeaderContent()
						[
							SNew(SHorizontalBox)
	
							+SHorizontalBox::Slot()
							.FillWidth(1.0)
							[ SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont")).Text(FText::FromString("Rotation")) ]
						]
						.BodyContent()
						[
							SNew(SVerticalBox)			
							+SVerticalBox::Slot() // RX
							.Padding(FMargin(-28.0f, 0.0f, 8.0f, 0.0f))
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SChartPlotVisibilityToggle)
									.InitialVisible(this->ShowRX)
									.OnToggleVisibility_Lambda([this](bool visible) {this->ShowRX = visible; })
								]

								+SHorizontalBox::Slot() 
								.Padding(FMargin(26.0f, 0.0f, 8.0f, 0.0f))
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
									.Text(FText::FromString("X"))
								]

								+SHorizontalBox::Slot() 
								.HAlign(HAlign_Right)
								.AutoWidth()
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
									.Text_Lambda([this]()
									{
										// visibility check
										if (this->DataSourceIndex < 0 
											|| this->DataSource.IsValid() == false 
											|| this->IsVisible == false
											|| this->ShowR == false
											|| this->ShowRX == false)
											return FText::FromString("");

										FString valueStr(TEXT("#ERR-VAL"));
										auto ds = this->DataSource->GetAs<StatsTracer::FTransformDataSource>();
										if (ds != nullptr)
											valueStr = FString::Printf(TEXT("%f"), (*ds)[this->DataSourceIndex].Value.GetRotation().Rotator().Roll);

										return FText::FromString(valueStr);
									})
								]
							]

							+SVerticalBox::Slot() // Y
							.Padding(FMargin(-28.0f, 0.0f, 8.0f, 0.0f))
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SChartPlotVisibilityToggle)
									.InitialVisible(this->ShowRY)
									.OnToggleVisibility_Lambda([this](bool visible) {this->ShowRY = visible; })
								]

								+SHorizontalBox::Slot() 
								.Padding(FMargin(26.0f, 0.0f, 8.0f, 0.0f))
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
									.Text(FText::FromString("Y"))
								]

								+SHorizontalBox::Slot() 
								.HAlign(HAlign_Right)
								.AutoWidth()
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
									.Text_Lambda([this]()
									{
										// visibility check
										if (this->DataSourceIndex < 0 
											|| this->DataSource.IsValid() == false 
											|| this->IsVisible == false
											|| this->ShowR == false
											|| this->ShowRY == false)
											return FText::FromString("");

										FString valueStr(TEXT("#ERR-VAL"));
										auto ds = this->DataSource->GetAs<StatsTracer::FTransformDataSource>();
										if (ds != nullptr)
											valueStr = FString::Printf(TEXT("%f"), (*ds)[this->DataSourceIndex].Value.GetRotation().Rotator().Pitch);

										return FText::FromString(valueStr);
									})
								]
							]

							+SVerticalBox::Slot() // Z
							.Padding(FMargin(-28.0f, 0.0f, 8.0f, 0.0f))
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SChartPlotVisibilityToggle)
									.InitialVisible(this->ShowRZ)
									.OnToggleVisibility_Lambda([this](bool visible) {this->ShowRZ = visible; })
								]

								+SHorizontalBox::Slot() 
								.Padding(FMargin(26.0f, 0.0f, 8.0f, 0.0f))
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
									.Text(FText::FromString("Z"))
								]

								+SHorizontalBox::Slot() 
								.HAlign(HAlign_Right)
								.AutoWidth()
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
									.Text_Lambda([this]()
									{
										// visibility check
										if (this->DataSourceIndex < 0 
											|| this->DataSource.IsValid() == false 
											|| this->IsVisible == false
											|| this->ShowR == false
											|| this->ShowRZ == false)
											return FText::FromString("");

										FString valueStr(TEXT("#ERR-VAL"));
										auto ds = this->DataSource->GetAs<StatsTracer::FTransformDataSource>();
										if (ds != nullptr)
											valueStr = FString::Printf(TEXT("%f"), (*ds)[this->DataSourceIndex].Value.GetRotation().Rotator().Yaw);

										return FText::FromString(valueStr);
									})
								]
							]
						]
					]
				]


				//-------------------------------
				// Scale
				//-------------------------------
				+SVerticalBox::Slot() 
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[	
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Top)
					[ 
						SNew(SBox)
						.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
						[
							SNew(SChartPlotVisibilityToggle)
							.InitialVisible(this->ShowS)
							.OnToggleVisibility_Lambda([this](bool visible) {this->ShowS = visible; })
						]
					]

					+SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
					[
						SNew(SExpandableArea)
						.InitiallyCollapsed(this->IsSExpanded == false)
						.OnAreaExpansionChanged_Lambda([this, parent](bool state) { parent->Refresh(); this->IsSExpanded = state; })
						.Padding(FMargin(4.0f, 2.0f, 0.0f, 0.0f))
						.HeaderContent()
						[
							SNew(SHorizontalBox)
	
							+SHorizontalBox::Slot()
							.FillWidth(1.0)
							[ SNew(STextBlock).Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont")).Text(FText::FromString("Scale")) ]
						]
						.BodyContent()
						[
							SNew(SVerticalBox)			
							+SVerticalBox::Slot() // RX
							.Padding(FMargin(-28.0f, 0.0f, 8.0f, 0.0f))
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SChartPlotVisibilityToggle)
									.InitialVisible(this->ShowSX)
									.OnToggleVisibility_Lambda([this](bool visible) {this->ShowSX = visible; })
								]

								+SHorizontalBox::Slot() 
								.Padding(FMargin(26.0f, 0.0f, 8.0f, 0.0f))
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
									.Text(FText::FromString("X"))
								]

								+SHorizontalBox::Slot() 
								.HAlign(HAlign_Right)
								.AutoWidth()
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
									.Text_Lambda([this]()
									{
										// visibility check
										if (this->DataSourceIndex < 0 
											|| this->DataSource.IsValid() == false 
											|| this->IsVisible == false
											|| this->ShowS == false
											|| this->ShowSX == false)
											return FText::FromString("");

										FString valueStr(TEXT("#ERR-VAL"));
										auto ds = this->DataSource->GetAs<StatsTracer::FTransformDataSource>();
										if (ds != nullptr)
											valueStr = FString::Printf(TEXT("%f"), (*ds)[this->DataSourceIndex].Value.GetScale3D().X);

										return FText::FromString(valueStr);
									})
								]
							]

							+SVerticalBox::Slot() // Y
							.Padding(FMargin(-28.0f, 0.0f, 8.0f, 0.0f))
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SChartPlotVisibilityToggle)
									.InitialVisible(this->ShowSY)
									.OnToggleVisibility_Lambda([this](bool visible) {this->ShowSY = visible; })
								]

								+SHorizontalBox::Slot() 
								.Padding(FMargin(26.0f, 0.0f, 8.0f, 0.0f))
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
									.Text(FText::FromString("Y"))
								]

								+SHorizontalBox::Slot() 
								.HAlign(HAlign_Right)
								.AutoWidth()
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
									.Text_Lambda([this]()
									{
										// visibility check
										if (this->DataSourceIndex < 0 
											|| this->DataSource.IsValid() == false 
											|| this->IsVisible == false
											|| this->ShowS == false
											|| this->ShowSY == false)
											return FText::FromString("");

										FString valueStr(TEXT("#ERR-VAL"));
										auto ds = this->DataSource->GetAs<StatsTracer::FTransformDataSource>();
										if (ds != nullptr)
											valueStr = FString::Printf(TEXT("%f"), (*ds)[this->DataSourceIndex].Value.GetScale3D().Y);

										return FText::FromString(valueStr);
									})
								]
							]

							+SVerticalBox::Slot() // Z
							.Padding(FMargin(-28.0f, 0.0f, 8.0f, 0.0f))
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SChartPlotVisibilityToggle)
									.InitialVisible(this->ShowSZ)
									.OnToggleVisibility_Lambda([this](bool visible) {this->ShowSZ = visible; })
								]

								+SHorizontalBox::Slot() 
								.Padding(FMargin(26.0f, 0.0f, 8.0f, 0.0f))
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
									.Text(FText::FromString("Z"))
								]

								+SHorizontalBox::Slot() 
								.HAlign(HAlign_Right)
								.AutoWidth()
								[
									SNew(STextBlock)
									.Font(FStatsTracerEditorStyle::Get().GetFontStyle("NormalFont"))
									.Text_Lambda([this]()
									{
										// visibility check
										if (this->DataSourceIndex < 0 
											|| this->DataSource.IsValid() == false 
											|| this->IsVisible == false
											|| this->ShowS == false
											|| this->ShowSZ == false)
											return FText::FromString("");

										FString valueStr(TEXT("#ERR-VAL"));
										auto ds = this->DataSource->GetAs<StatsTracer::FTransformDataSource>();
										if (ds != nullptr)
											valueStr = FString::Printf(TEXT("%f"), (*ds)[this->DataSourceIndex].Value.GetScale3D().Z);

										return FText::FromString(valueStr);
									})
								]
							]
						]
					]
				]
			]
		];
}