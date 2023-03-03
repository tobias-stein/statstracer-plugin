///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Private\TracerComponentCustomization.h
///
/// Summary:	Declares the tracer component customization class.
///-------------------------------------------------------------------------------------------------

#pragma once

//#include "CoreMinimal.h"
#include "Engine.h"
#include "IDetailCustomization.h"

#include "TracerComponent.h"

#include "SlateFwd.h"
#include "Layout/Visibility.h"
#include "Input/Reply.h"

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"

class FReply;
class IDetailLayoutBuilder;
class UTracerComponent;

template <typename ItemType> 
class SListView;

class STracerPropertyListUI : public SCompoundWidget
{
private:

	struct FTracerStatListItem
	{
		UTracerComponent*			TracerComp;

		const FName					OwnerType;

		const FName					StatKey;
		const uint32				StatIndex;

		bool						IsExpanded;

		FTracerStatListItem(UTracerComponent* tracerComp, const FName ownerName, const FName statKey, const uint32 statIndex) :
			TracerComp(tracerComp),
			OwnerType(ownerName),
			StatKey(statKey),
			StatIndex(statIndex),
			IsExpanded(false)
		{}

		FTracableStat& GetStat() const
		{ 
			auto propertyOwner = this->TracerComp->DetectedStats.Find(this->StatKey);
			return (*propertyOwner)[this->StatIndex];
		}
	};

private:

	using TTracerStatItem						= TSharedPtr<FTracerStatListItem>;

	using STracerStatOwnerArrayListView			= SListView<TTracerStatItem>;
	using STracerStatItemListView				= SListView<TTracerStatItem>;

	using TTracerStatOwnerArrayItemArray		= TArray<TTracerStatItem>;
	using TTracerStatItemArray					= TArray<TTracerStatItem>;

	// the item array
	TTracerStatOwnerArrayItemArray				m_ListItems;

	// the list view
	TSharedPtr<STracerStatOwnerArrayListView>	m_PropertyOwnerListView;

	// a simple searchbox for looking items up by their name
	TSharedPtr<SSearchBox>						m_ListViewFilter;

	// reference to the UTracerComponent
	TWeakObjectPtr<class UTracerComponent>		m_TracerComp;

	int32										m_TotalStatsNum;
	int32										m_TracedStatsNum;

public:

	SLATE_BEGIN_ARGS(STracerPropertyListUI) :
		_TracerComp()
	{}
	SLATE_ARGUMENT(TWeakObjectPtr<class UTracerComponent>, TracerComp);
	SLATE_END_ARGS()


	void Construct(const FArguments& InArgs);

	void UpdateListview();

	FString GetFilterString() const;

	inline void IncrementTraced() { this->m_TracedStatsNum++; }
	inline void DecrementTraced() { this->m_TracedStatsNum--; }

	inline void RefreshListview()
	{
		if (this->m_PropertyOwnerListView.IsValid())
			this->m_PropertyOwnerListView->RequestListRefresh();
	}

private:

	TSharedRef<ITableRow> OnGenerateRowForPropertyOwnerListView(TTracerStatItem InItem, const TSharedRef<STableViewBase>& OwnerTable, STracerPropertyListUI* InParent);
};


class TracerComponentCustomization : public IDetailCustomization
{
public:

	
	static TSharedRef<IDetailCustomization> MakeInstance();


	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	
	TWeakObjectPtr<class UTracerComponent>	m_TracerComp;

	TSharedPtr<STracerPropertyListUI>		m_TracerPropertyList;

	FReply DetectStatsButtonClicked(UTracerComponent* tracerComponentRef);
};