///-------------------------------------------------------------------------------------------------
/// Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
/// 
/// File:	StatsTracer\Public\TracerComponent.h
///
/// Summary:	Declares the tracer component class.
///-------------------------------------------------------------------------------------------------

#pragma once

#include "Engine.h"
#include "Components/ActorComponent.h"

#include "TracerComponent.generated.h"

///-------------------------------------------------------------------------------------------------
/// struct FTracableStat
///
/// Summary:	Unreal structure used for storing information about tracable stats. This sturct also 
/// contains properties for displaying it in chart views, e.g. color, description ..
///
/// Author:	Tobias Stein
///
/// Date:	26/02/2018
///-------------------------------------------------------------------------------------------------

USTRUCT()
struct FTracableStat
{
	GENERATED_BODY()

	UPROPERTY()
	FString							OutterName;

	UPROPERTY()
	FString							PropertyName;

	UPROPERTY()
	FString							PropertyType;

	UPROPERTY()
	FString							AlliasName;

	UPROPERTY()
	FString							Group;

	UPROPERTY()
	FString							Description;

	UPROPERTY()
	FLinearColor					Color;

	UPROPERTY()
	bool							IsTraced;

	UPROPERTY()
	bool							StreamToCsv;

	FTracableStat() :
		OutterName(TEXT("")),
		PropertyName(TEXT("INAVLID")),
		PropertyType(TEXT("INAVLID")),
		AlliasName(TEXT("")),
		Group(TEXT("Default")),
		Description(TEXT("")),
		Color(FLinearColor::MakeRandomColor()),
		IsTraced(false),
		StreamToCsv(false)
	{}

	FTracableStat(const UProperty* uprop, FLinearColor color) :
		OutterName(uprop->GetOuter()->GetName()),
		PropertyName(uprop->GetName()),
		PropertyType(uprop->GetCPPType()),
		AlliasName(TEXT("")),
		Group(TEXT("Default")),
		Description(TEXT("")),
		Color(color),
		IsTraced(false),
		StreamToCsv(true)
	{}

	FTracableStat(const FTracableStat& other) :
		OutterName(other.OutterName),
		PropertyName(other.PropertyName),
		PropertyType(other.PropertyType),
		AlliasName(other.AlliasName),
		Group(other.Group),
		Description(other.Description),
		Color(other.Color),
		IsTraced(other.IsTraced),
		StreamToCsv(true)
	{}

	FTracableStat& operator=(const FTracableStat& other)
	{
		if (this == &other)
			return *this;

		this->OutterName = other.OutterName;
		this->PropertyName = other.PropertyName;
		this->PropertyType = other.PropertyType;
		this->AlliasName = other.AlliasName;
		this->Group = other.Group;
		this->Description = other.Description;
		this->Color = other.Color;
		this->IsTraced = other.IsTraced;
		this->StreamToCsv = other.StreamToCsv;

		return *this;
	}

	///-------------------------------------------------------------------------------------------------
	/// Fn:	inline bool operator==(const UProperty* uprop) const
	///
	/// Summary:	Equality operator. Allows us to compare a UProperty with FTracableStat. Returns
	/// true if   name, outter name and type match.
	///
	/// Author:	Tobias Stein
	///
	/// Date:	7/03/2018
	///
	/// Parameters:
	/// uprop - 	The uprop.
	///
	/// Returns:	True if the parameters are considered equivalent.
	///-------------------------------------------------------------------------------------------------

	inline bool operator==(const UProperty* uprop) const
	{
		if (uprop->GetOuter()->GetName().Equals(this->OutterName) && uprop->GetName().Equals(this->PropertyName) && uprop->GetCPPType().Equals(this->PropertyType))
			return true;

		return false;
	}

	static FTracableStat INVALID;
};

USTRUCT()
struct FTracableStatsOwner
{
	GENERATED_BODY()

	UPROPERTY()
	FName					OwnerType;

	UPROPERTY()
	TArray<FTracableStat>	StatsArray;

	FTracableStatsOwner()
	{}

	FTracableStatsOwner(const FName& ownerType) :
		OwnerType(ownerType)
	{}

	// proxy methods
	inline int32			Num() const { return this->StatsArray.Num(); }
	inline void				Add(FTracableStat stat) { this->StatsArray.Add(stat); }

	inline FTracableStat&	operator[](const int32 index) { return this->StatsArray[index]; }
};

UCLASS(ClassGroup=(StatsTracer), meta=(BlueprintSpawnableComponent))
class STATSTRACER_API UTracerComponent : public UActorComponent
{
	GENERATED_BODY()

	float								NextColorStartHue;

public:	

	UPROPERTY()
	FString								TracerName;

	UPROPERTY()
	FString								TracerDescription;

	UPROPERTY()
	TMap<FName, FTracableStatsOwner>	DetectedStats;

	UPROPERTY()
	bool								EnableStreamToCsv;

	// Sets default values for this component's properties
	UTracerComponent();


protected:

	// Called when the game starts
	virtual void BeginPlay() override;

public:	

	///-------------------------------------------------------------------------------------------------
	/// Fn:	void UTracerComponent::ScanStats();
	///
	/// Summary:	Invokes a complete rescan of possible tracable stats (int, float, byte, vector ...).
	/// This will fill/update the member array 'DetectedStats', which holds all detected stats of the parent
	/// actor.
	///
	/// Author:	Tobias Stein
	///
	/// Date:	26/02/2018
	///-------------------------------------------------------------------------------------------------

	void ScanStats();

	virtual void OnComponentCreated() override;
};
