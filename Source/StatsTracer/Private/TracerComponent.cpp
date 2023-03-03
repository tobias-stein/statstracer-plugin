///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracer\Private\TracerComponent.cpp
///
/// Summary:	Implements the tracer component class.
///-------------------------------------------------------------------------------------------------

#include "TracerComponent.h"
#include "StatsTracerPCH.h"


// Sets default values for this component's properties
UTracerComponent::UTracerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// set a default tracer name
	this->TracerName = "DefaultStatsTracer";
	
	// try to construct a more appropriate default name
	{
		// get owning actor
		AActor* myActor = this->GetOwner();
		if (myActor != nullptr)
			this->TracerName = myActor->GetName();
	}
	
	this->TracerDescription = TEXT("");
	this->EnableStreamToCsv = false;
	this->NextColorStartHue = 0.0f;
}


// Called when the game starts
void UTracerComponent::BeginPlay()
{
	SCOPE_CYCLE_COUNTER(STAT_UTracerComponent_BeginPlay);

	Super::BeginPlay();

	if (DetectedStats.Num() == 0)
		return;

	// get owning actor
	AActor* actor = this->GetOwner();

	UTracer* tracer = UStatsTracerBPLibrary::CreateStatsTracer(actor, *this->TracerName, *this->TracerDescription, this->EnableStreamToCsv);
	if (tracer == nullptr)
		return;

	TArray<UObject*> candidates;

	// Get iterator to actors and its components perterties
	if (actor != nullptr)
	{
		for (auto actorComponent : actor->GetComponents())
		{
			if (actorComponent->IsA<UTracerComponent>())
				continue;

			if (actorComponent)
				if (actorComponent != nullptr)
					candidates.Add(actorComponent);
		}

		candidates.Add(actor);
	}

	while (candidates.Num() > 0)
	{
		UObject* candidate = candidates.Pop();
		const FName ownerName = candidate->GetFName();
		TFieldIterator<UProperty> PropIt(candidate->GetClass());

		auto stats = DetectedStats.Find(ownerName);
		if (stats == nullptr)
			continue;

		for (PropIt; PropIt; ++PropIt)
		{
			UProperty* property = *PropIt;

			if (property == nullptr || property->IsValidLowLevel() == false)
				continue;

			for (int32 i = 0; i < (*stats).Num(); ++i)
			{
				// get matching property
				if (((*stats)[i] == property) && ((*stats)[i].IsTraced == true))
				{
					const FString name = (*stats)[i].AlliasName.IsEmpty() == false ? (*stats)[i].AlliasName : (*stats)[i].PropertyName;

					if ((*stats)[i].PropertyType.Equals("bool"))
						tracer->AddBoolStat(*((bool*)property->ContainerPtrToValuePtr<bool>(candidate)), name, (*stats)[i].Group, (*stats)[i].Description, (*stats)[i].Color.ToFColor(true), (*stats)[i].StreamToCsv);
					else if ((*stats)[i].PropertyType.Equals("int32"))
						tracer->AddIntStat(*((int32*)property->ContainerPtrToValuePtr<int32>(candidate)), name, (*stats)[i].Group, (*stats)[i].Description, (*stats)[i].Color.ToFColor(true), (*stats)[i].StreamToCsv);
					else if ((*stats)[i].PropertyType.Equals("float"))
						tracer->AddFloatStat(*((float*)property->ContainerPtrToValuePtr<float>(candidate)), name, (*stats)[i].Group, (*stats)[i].Description, (*stats)[i].Color.ToFColor(true), (*stats)[i].StreamToCsv);
					else if ((*stats)[i].PropertyType.Equals("uint8"))
						tracer->AddByteStat(*((uint8*)property->ContainerPtrToValuePtr<uint8>(candidate)), name, (*stats)[i].Group, (*stats)[i].Description, (*stats)[i].Color.ToFColor(true), (*stats)[i].StreamToCsv);
					else if ((*stats)[i].PropertyType.Equals("FVector"))
						tracer->AddVectorStat(*((FVector*)property->ContainerPtrToValuePtr<FVector>(candidate)), name, (*stats)[i].Group, (*stats)[i].Description, (*stats)[i].Color.ToFColor(true), (*stats)[i].StreamToCsv);
					else if ((*stats)[i].PropertyType.Equals("FRotator"))
						tracer->AddRotatorStat(*((FRotator*)property->ContainerPtrToValuePtr<FRotator>(candidate)), name, (*stats)[i].Group, (*stats)[i].Description, (*stats)[i].Color.ToFColor(true), (*stats)[i].StreamToCsv);
					else if ((*stats)[i].PropertyType.Equals("FTransform"))
						tracer->AddTransformStat(*((FTransform*)property->ContainerPtrToValuePtr<FTransform>(candidate)), name, (*stats)[i].Group, (*stats)[i].Description, (*stats)[i].Color.ToFColor(true), (*stats)[i].StreamToCsv);

					// check next stat
					break;
				}
			}
		}
	}
}

void UTracerComponent::ScanStats()
{
	SCOPE_CYCLE_COUNTER(STAT_UTracerComponent_ScanStats);

	TArray<UObject*> candidates;

	// Get iterator to actors and its components perterties
	AActor* actor = this->GetOwner();
	if (actor != nullptr)
	{
		for (auto actorComponent : actor->GetComponents())
		{
			if (actorComponent->IsA<UTracerComponent>() || actorComponent == actor->GetRootComponent())
				continue;

			if(actorComponent)
			if (actorComponent != nullptr)
				candidates.Add(actorComponent);
		}

		auto RSC = actor->GetRootComponent();
		if(RSC != nullptr)
			candidates.Add(RSC);

		candidates.Add(actor);
	}


	// getglobal filter, specified in settings
	FString FILTER;
	for (FString f : UStatsTracerEditorSettings::GetInstance()->GlobalStatsFilter)
	{
		if (f.IsEmpty() == false)
			FILTER += FString::Printf(TEXT("\\b%s\\b|"), *f);
	}
	FILTER.RemoveFromEnd("|");

	const FRegexPattern reFilterPattern(FILTER);

	TMap<FName, FTracableStatsOwner> DetectedStatsUpdated;

	while(candidates.Num() > 0)
	{
		UObject* candidate = candidates.Pop();

		const FName ownerName	= candidate->GetFName();

		TFieldIterator<UProperty> PropIt(candidate->GetClass());

		DetectedStatsUpdated.Add(ownerName, FTracableStatsOwner(PropIt.GetStruct()->GetFName()));
		for (PropIt; PropIt; ++PropIt)
		{
			UProperty* property = *PropIt;

			if (property == nullptr || property->IsValidLowLevel() == false)
				continue;

			// apply filter, if set
			if (FILTER.IsEmpty() == false)
			{
				FRegexMatcher reMatcher(reFilterPattern, property->GetName());
				if (reMatcher.FindNext())
					continue;
			}

			// make sure its tracable
			if (StatsTracer::IsTracableProperty(property->GetCPPType()) == true)
			{
				// check if stat is already detected
				bool isDetected = false;

				auto propertyOwner = this->DetectedStats.Find(ownerName);
				if (propertyOwner != nullptr)
				{
					for (int32 i = 0; i < propertyOwner->Num(); ++i)
					{
						if ((*propertyOwner)[i] == property)
						{
							DetectedStatsUpdated[ownerName].Add((*propertyOwner)[i]);
							isDetected = true;
							break;
						}
					}
				}
				// add stat
				if (isDetected == false)
				{
					auto stat = FTracableStat(property, StatsTracer::GetNextLabelColor(this->NextColorStartHue, &(this->NextColorStartHue)));

					// put multi dimensional stats in unique Group by default (makes it more readable)
					if (StatsTracer::IsMultidimensionalTracableProperty(property->GetCPPType()) == true)
						stat.Group = property->GetName();

					DetectedStatsUpdated[ownerName].Add(stat);
				}
			}
		}
	}

	// update detected stats
	this->DetectedStats.Empty();
	this->DetectedStats = DetectedStatsUpdated;
}

void UTracerComponent::OnComponentCreated()
{
	// only allow one tracer component per Actor, check for duplicates.
	TArray<UTracerComponent*> temp;
	this->GetOwner()->GetComponents<UTracerComponent>(temp);

	if (temp.Num() > 1)
	{
		DestroyComponent();
		return;
	}		

	this->ScanStats();	
}


