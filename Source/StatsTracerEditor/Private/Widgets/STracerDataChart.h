///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Private\Widgets\STracerDataChart.h
///
/// Summary:	Declares the tracer data chart class.
///-------------------------------------------------------------------------------------------------

#pragma once

#include "CoreMinimal.h"

#include "StatsTracerCore.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class STracerDataChart : public SCompoundWidget
{
	struct ITracerDataSourceListItem;

	using TDataSourceArrayPtr		= const StatsTracer::TDataSourceArray*;

	using TTracerDataSourceListItem = TSharedPtr<ITracerDataSourceListItem>;
	using FTracerDataSourceList		= TArray<TTracerDataSourceListItem>;
	using STracerDataSourceListView	= SListView<TTracerDataSourceListItem>;

	class STracerDataListItemWidget : public SMultiColumnTableRow<TTracerDataSourceListItem>
	{
	private:

		float						PendingListRefreshDelay;

		STracerDataSourceListView*	Listview;
		TTracerDataSourceListItem	Item;

	public:

		SLATE_BEGIN_ARGS(STracerDataListItemWidget) {}
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TTracerDataSourceListItem InListItem);

		virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

		void Refresh();

		TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName);
	};

	struct ITracerDataSourceListItem : public TSharedFromThis<ITracerDataSourceListItem>
	{
		// The handle to the actual data
		StatsTracer::TDataSourceHandle		DataSource;

		int32								DataSourceIndex;

		// Visual state
		bool								IsVisible;


		ITracerDataSourceListItem(StatsTracer::TDataSourceHandle dataSource) :
			DataSource(dataSource),
			DataSourceIndex(-1),
			IsVisible(true)
		{}

		virtual ~ITracerDataSourceListItem()
		{}

		virtual TSharedRef<SWidget> BuildWidget(STracerDataListItemWidget* parent) = 0;
	};

	struct FSimpleTracerDataSourceListItem : public ITracerDataSourceListItem
	{
		

		FSimpleTracerDataSourceListItem(StatsTracer::TDataSourceHandle dataSource) :
			ITracerDataSourceListItem(dataSource)
		{}

		virtual ~FSimpleTracerDataSourceListItem()
		{}

		virtual TSharedRef<SWidget> BuildWidget(STracerDataListItemWidget* parent) override;
	};

	struct FVectorTracerDataSourceListItem : public ITracerDataSourceListItem
	{
		bool IsExpanded;
		bool ShowX, ShowY, ShowZ;

		FVectorTracerDataSourceListItem(StatsTracer::TDataSourceHandle dataSource) :
			ITracerDataSourceListItem(dataSource),
			IsExpanded(true),
			ShowX(true),
			ShowY(true),
			ShowZ(true)
		{}

		virtual ~FVectorTracerDataSourceListItem()
		{}

		virtual TSharedRef<SWidget> BuildWidget(STracerDataListItemWidget* parent) override;
	};

	struct FTransformTracerDataSourceListItem : public ITracerDataSourceListItem
	{
		bool IsExpanded, IsPExpanded, IsRExpanded, IsSExpanded;
		bool ShowP, ShowPX, ShowPY, ShowPZ;
		bool ShowR, ShowRX, ShowRY, ShowRZ;
		bool ShowS, ShowSX, ShowSY, ShowSZ;

		FTransformTracerDataSourceListItem(StatsTracer::TDataSourceHandle dataSource) :
			ITracerDataSourceListItem(dataSource),
			IsExpanded(true), IsPExpanded(true), IsRExpanded(true), IsSExpanded(false),
			ShowP(true), ShowPX(true), ShowPY(true), ShowPZ(true),
			ShowR(true), ShowRX(true), ShowRY(true), ShowRZ(true),
			ShowS(false), ShowSX(true), ShowSY(true), ShowSZ(true)
		{}

		virtual ~FTransformTracerDataSourceListItem()
		{}

		virtual TSharedRef<SWidget> BuildWidget(STracerDataListItemWidget* parent) override;
	};
	

	TSharedRef<ITableRow>					OnGenerateRowForTracerDataSourceListView(TTracerDataSourceListItem InItem, const TSharedRef<STableViewBase>& OwnerTable);

private:

	TDataSourceArrayPtr						DataSourceArray;
	float									ChartAreaWidth;
	float									ZoomFactor;
	int32									UpdateFrequency;

	TWeakPtr<StatsTracer::FTracerSession>	SessionHandle;

	FTracerDataSourceList					TracerDataSourceListItems;
	TSharedPtr<STracerDataSourceListView>	TracerDataSourceListView;

	TSharedPtr<SScrollBox>					SPlotAreaScrollBox;
	
	FGeometry								ThisAllottedGeometry;
	
private:

	class SPlotArea : public SCompoundWidget
	{
	private:

		/** Helper struct for chart layout **/
		struct TChartLayout
		{
			struct TRect
			{
				float X0, Y0, X1, Y1;

				TRect() : X0(0), Y0(0), X1(0), Y1(0)
				{}

				TRect(float x0, float y0, float x1, float y1) : X0(x0), Y0(y0), X1(x1), Y1(y1)
				{}

				inline FVector2D			GetTopLeft() const { return FVector2D(X0, Y0); }
				inline FVector2D			GetBottomRight() const { return FVector2D(X1, Y1); }
			};

			FSlateFontInfo		ChartDefaultFont;
			FSlateFontInfo		ChartAidLableFont;
			FSlateFontInfo		ChartIndexLableFont;
			FSlateFontInfo		ChartTimeLabelFont;

			float				MaxFontCharHeight;
			float				ZeroCharWidth;
			float				TimeLabelSize;

			const TRect			MARGIN{ TRect(5.0f, 5.0f, 5.0f, 5.0f) };

			TRect				OutterRect, InnerRect;

			float				ScrollOffset;

			float				SampleDataXOffset; 

			int32				XAxisGridSteps;
			float				YAxisMajorStepN;

			int32				XAxisGridStepSize;

			float				XPos0, XPos1;
			float				YPos0, YPos1;

			float				ChartPlotAreaWith;

			FVector2D			XAxisLabelAreaSize;
			FVector2D			YAxisLabelAreaSize;

			int32				Precision;
			int32				SampleWindowSize;

			TChartLayout();
			TChartLayout(float x0, float y0, float x1, float y1, float scrollOffset);

			TChartLayout&		operator=(const TChartLayout& rhs);

			bool				IsValidLayout() const;
			void				Update();
			float				MeasureTextWidth(const FString& text, const FSlateFontInfo& font);

			inline void			SetY0(float value) { this->YPos0 = value; }
			inline void			SetY1(float value) { this->YPos1 = value; }

			inline float		GetYZeroOffset() const { return -this->YPos0; }

			inline void			SetPrecision(int32 precision) { this->Precision = precision; }
			inline int32		GetPrecision() const { return this->Precision; }

			inline void			SetSampleWindowSize(int32 sampleWindowSize) { this->SampleWindowSize = sampleWindowSize; }
			inline int32		GetSampleWindowSize() const { return this->SampleWindowSize; }

			inline float		GetXPos(int32 t) { return this->InnerRect.X0 + this->YAxisLabelAreaSize.X + (t * this->SampleDataXOffset); }
			inline float		GetYPos(float value) { return this->InnerRect.Y0 + ((this->InnerRect.Y1 - this->InnerRect.Y0) * (1.0f - ((value - this->YPos0) / (this->YPos1 - this->YPos0)))); }

			// min/max scaled value
			inline float		GetXPos0() const { return this->XPos0; }
			inline float		GetXPos1() const { return this->XPos1; }
			inline float		GetYPos0() const { return this->YPos0; }
			inline float		GetYPos1() const { return this->YPos1; }

			inline int32		GetXAxisStepN() const { return this->XAxisGridSteps; }
			inline int32		GetXAxisStepSize() const { return this->XAxisGridStepSize; }


			inline float		GetXAxisLabelAreaHeight() const { return this->XAxisLabelAreaSize.Y; }

			inline int32		GetYAxisMajorStepN() const { return this->YAxisMajorStepN; }
			inline float		GetYAxisMajorStepSize() const { return ((this->YPos1 - this->YPos0) / FMath::Max<int32>(1.0f, this->YAxisMajorStepN)); }

			inline float		GetYAxisLabelAreaWidth() const { return this->YAxisLabelAreaSize.X; }			
		};

		STracerDataChart*		TracerDataChart;

		bool					IsIndexed;
		FVector2D				MousePosition;
		int32					BufferIndex;

	public:

		SLATE_BEGIN_ARGS(SPlotArea) :
			_TracerDataChart()
		{}

		SLATE_ARGUMENT(STracerDataChart*, TracerDataChart);
		SLATE_END_ARGS()

		/** Constructs this widget with InArgs */
		void Construct(const FArguments& InArgs);

		virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	
	
		virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
		virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
		virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	private:

		void DrawIndexAidLabels(
			float										value,
			const FColor&								color,
			const FString&								text,
			const FGeometry&							AllottedGeometry,
			FSlateWindowElementList&					OutDrawElements,
			const ESlateDrawEffect						DrawEffects,
			const uint32								LayerId,
			STracerDataChart::SPlotArea::TChartLayout&	Layout) const;
	};

	TSharedPtr<SPlotArea>					SPlotAreaWidget;

public:

	SLATE_BEGIN_ARGS(STracerDataChart) :
		_DataSourceArray(),
		_ChartAreaWidth(),
		_ZoomFactor(),
		_SessionHandle()
	{}
	SLATE_ARGUMENT(TDataSourceArrayPtr, DataSourceArray);
	SLATE_ARGUMENT(float, ChartAreaWidth);
	SLATE_ARGUMENT(float, ZoomFactor);
	SLATE_ARGUMENT(TWeakPtr<StatsTracer::FTracerSession>, SessionHandle);
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	inline void SetChartAreaWidth(float NewWidth) { this->ChartAreaWidth = NewWidth; }
	inline void SetChartZoom(float NewZoom) 
	{ 
		if (this->SPlotAreaScrollBox.IsValid() == true)
			this->SPlotAreaScrollBox->ScrollToStart();

		this->ZoomFactor = NewZoom; 
	}

	inline int32 GetDataSourceCount() const { return this->DataSourceArray->Num(); }
	inline TTracerDataSourceListItem operator[](int32 i) const { return this->TracerDataSourceListItems[i]; }

	inline float GetScrollOffset() const 
	{ 
		if (this->SPlotAreaScrollBox.IsValid() == false)
			return 0.0f;

		return this->SPlotAreaScrollBox->GetScrollOffset();
	}

	inline int32 GetSampleUpdateFrequency() const { return this->UpdateFrequency; }

	inline float GetMinDesiredHeight() const { return TracerDataSourceListView->GetDesiredSize().Y + 30.0f; }

	inline TWeakPtr<StatsTracer::FTracerSession> GetSessionHandle() const { return this->SessionHandle; }
};
