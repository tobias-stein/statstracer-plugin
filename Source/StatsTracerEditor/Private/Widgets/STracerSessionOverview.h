///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Private\Widgets\STracerSessionOverview.h
///
/// Summary:	Declares the tracer session overview class.
///-------------------------------------------------------------------------------------------------

#pragma once

//#include "CoreMinimal.h"
#include "Engine.h"

#include "StatsTracerCore.h"
#include "Widgets/SCompoundWidget.h"


class FReply;

/**
 * 
 */
class STracerSessionOverview : public SCompoundWidget
{

	struct FTracerListItem
	{
		StatsTracer::TWeakTracerDataRepositoryHandle	ReposityDataHandle;
		TWeakPtr<SDockTab>								OpenTabInstance;

		FTracerListItem(StatsTracer::TWeakTracerDataRepositoryHandle InReposityDataHandle, TWeakPtr<SDockTab> InTabInstance) :
			ReposityDataHandle(InReposityDataHandle),
			OpenTabInstance(InTabInstance)
		{}
	};

	struct FSessionComboboxItem
	{
		StatsTracer::TWeakTracerSessionHandle			SessionHandle;

		FSessionComboboxItem(StatsTracer::TWeakTracerSessionHandle InHandle) :
			SessionHandle(InHandle)
		{}
	};

	///-------------------------------------------------------------------------------------------------
	/// Session Selection
	///-------------------------------------------------------------------------------------------------

	using TSessionComboboxItem			= TSharedPtr<FSessionComboboxItem>;
	using TSessionSelection				= TSharedPtr<SComboBox<TSessionComboboxItem>>;

	void								OnSessionSelectionChanged(TSessionComboboxItem NewValue, ESelectInfo::Type type);
	FText								GetSelectedSessionName() const;

	///-------------------------------------------------------------------------------------------------
	/// Tracer Selection
	///-------------------------------------------------------------------------------------------------

	using TTracerListItem				= TSharedPtr<FTracerListItem>;

	using STracerListView				= SListView<TTracerListItem>;
	using FTracerList					= TArray<TTracerListItem>;


	TSharedRef<ITableRow>				OnGenerateRowForTracerListView(TTracerListItem InItem, const TSharedRef<STableViewBase>& OwnerTable, STracerSessionOverview* sessionOverview);

	void								SpawnTracerOverviewTab(TTracerListItem Item);

private:

	using TOpenTabArray					= TMap<FString, TWeakPtr<SDockTab>>;

	static TOpenTabArray				OpenTabArray;

	TSessionSelection					SessionSelection;

	// list of available sessions
	TArray<TSessionComboboxItem>		SessionItemArray;

	// current selected session
	TSessionComboboxItem				SelectedSessionItem;


	// tracer data from selected session
	FTracerList							SessionTracers;

	TSharedPtr<STracerListView>			TracerList;

public:

	SLATE_BEGIN_ARGS(STracerSessionOverview)
	{
	}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void								Construct(const FArguments& InArgs);

	StatsTracer::TWeakTracerSessionHandle GetSessionHandle();

	void								RefreshSessionTracerList();

	static TWeakPtr<SDockTab>			OpenDataOverviewTab(TSharedPtr<SWidget> InParent, StatsTracer::TWeakTracerDataRepositoryHandle InDataRepositoryHandle);

	static void							CloseDataOverviewTab(const FTabId& InTabId);
	static void							CloseDataOverviewTab(SDockTab* InTabInstace, bool ExecClose = true);
	

	static TWeakPtr<SDockTab>			FindOpenTabInstance(const FString& InTabKey);
	static FString						GetOpenTabArrayKey(const StatsTracer::FTracerDataRepository* InDataRepository);

	void								RefreshSessionItemArray();
};
