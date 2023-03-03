///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Private\Widgets\STracerDataOverview.h
///
/// Summary:	Declares the tracer data overview class.
///-------------------------------------------------------------------------------------------------

#pragma once

//#include "CoreMinimal.h"
#include "Engine.h"

#include "StatsTracerCore.h"
#include "Widgets/SCompoundWidget.h"


/**
 * 
 */
class STracerDataOverview : public SCompoundWidget
{
	using TracerDataHandle = StatsTracer::TWeakTracerDataRepositoryHandle;


	struct FTracerDataListItem
	{
		const FString							DataGroupName;
		const StatsTracer::TDataSourceArray*	DataSourceArray; // this will be atleast valid as long this tab is open

		bool									IsExpanded;
		FTracerDataListItem(const FString& dataGroupName, const StatsTracer::TDataSourceArray* dataSourceArray) :
			DataGroupName(dataGroupName),
			DataSourceArray(dataSourceArray),
			IsExpanded(true)
		{}
	};

	using TTracerDataListItem	= TSharedPtr<FTracerDataListItem>;
	using FTracerDataList		= TArray<TTracerDataListItem>;
	using STracerDataListView	= SListView<TTracerDataListItem>;

	TSharedRef<ITableRow>	OnGenerateRowForTracerDataListView(TTracerDataListItem InItem, const TSharedRef<STableViewBase>& OwnerTable, STracerDataOverview* InParentWidget);

private:


	FText GetSessionId() const;
	FText GetSessionStart() const;
	FText GetSessionEnd() const;
	FText GetSessionState() const;

private:

	TracerDataHandle						TracerData;

	FTracerDataList							TracerDataItems;

	TSharedPtr<STracerDataListView>			TracerDataListview;

	bool									DescriptionExpanded;
	bool									StateExpanded;


	TWeakPtr<SWidget>						Parent;

public:

	TSharedPtr<SDockTab>					ThisTab;

	SLATE_BEGIN_ARGS(STracerDataOverview) :
		_Parent(),
		_TracerData()
	{}
	SLATE_ARGUMENT(TSharedPtr<SWidget>, Parent)
	SLATE_ARGUMENT(TracerDataHandle, TracerData)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	inline TWeakPtr<StatsTracer::FTracerSession> GetSessionHandle() const 
	{ 
		if (this->TracerData.IsValid() == false)
			return nullptr;

		return this->TracerData.Pin()->GetSession(); 
	}
};
