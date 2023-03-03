///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracer\Public\Tracer.h
///
/// Summary:	Declares the tracer class.
///-------------------------------------------------------------------------------------------------

#pragma once

#include "Engine.h"
#include "StatsTracerCore.h"

#include "Tracer.generated.h"

/** Tracers are proxy objects that are used by the 'StatsTracer' plugin to establish a connection between the target objects stats and an internal data-repository.  */
UCLASS(meta = (BlueprintSpawnableComponent))
class STATSTRACER_API UTracer : public UObject
{
	GENERATED_BODY()

	StatsTracer::TWeakTracerDataRepositoryHandle m_TracerDataRepository;

public:

	UTracer();
	~UTracer();

	///-------------------------------------------------------------------------------------------------
	/// Fn:
	/// void UTracer::Initialize(const FString& name, const FString& description, AActor* TracedActor,
	/// const bool streamToCsv = false, const bool autostartOnBeginPlay = true);
	///
	/// Summary:	This method must be called before using any of the 'AddStats' methods. Initializing a 
	/// tracer will effectively cause the internal data-repository to be created. 
	///
	/// Author:	Tobias Stein
	///
	/// Date:	2/04/2018
	///
	/// Parameters:
	/// name - 				   	The name.
	/// description - 		   	The description.
	/// TracedActor - 		   	[in,out] If non-null, the traced actor.
	/// streamToCsv - 		   	(Optional) True to stream to CSV.
	/// autostartOnBeginPlay - 	(Optional) True to autostart on begin play.
	///-------------------------------------------------------------------------------------------------

	void Initialize(const FString& name, const FString& description, AActor* TracedActor, const bool streamToCsv = false, const bool autostartOnBeginPlay = true);

	/** Adds a new data-source for a bool type variable of the target object to the data-repository. */
	UFUNCTION(BlueprintCallable, Category = "Stats Tracer")
	void AddBoolStat(
		UPARAM(ref) const bool& value,
		const FString& name,
		const FString& group = "",
		const FString& description = "",
		const FColor color = FColor(0, 0, 0, 0),
		const bool streamToCsv = true);

	/** Adds a new data-source for an int32 type variable of the target object to the data-repository. */
	UFUNCTION(BlueprintCallable, Category = "Stats Tracer")
	void AddIntStat(
		UPARAM(ref) const int32& value,
		const FString& name,
		const FString& group = "",
		const FString& description = "",
		const FColor color = FColor(0, 0, 0, 0),
		const bool streamToCsv = true);
	
	/** Adds a new data-source for a float type variable of the target object to the data-repository. */
	UFUNCTION(BlueprintCallable, Category = "Stats Tracer")
	void AddFloatStat(
		UPARAM(ref) const float& value,
		const FString& name,
		const FString& group = "",
		const FString& description = "",
		const FColor color = FColor(0, 0, 0, 0),
		const bool streamToCsv = true);
	
	/** Adds a new data-source for a byte type variable of the target object to the data-repository. */
	UFUNCTION(BlueprintCallable, Category = "Stats Tracer")
	void AddByteStat(
		UPARAM(ref) const uint8& value,
		const FString& name,
		const FString& group = "",
		const FString& description = "",
		const FColor color = FColor(0, 0, 0, 0),
		const bool streamToCsv = true);
	
	/** Adds a new data-source for a FVector type variable of the target object to the data-repository. */
	UFUNCTION(BlueprintCallable, Category = "Stats Tracer")
	void AddVectorStat(
		UPARAM(ref) const FVector& value,
		const FString& name,
		const FString& group = "",
		const FString& description = "",
		const FColor color = FColor(0, 0, 0, 0),
		const bool streamToCsv = true);
	
	/** Adds a new data-source for a FRotator type variable of the target object to the data-repository. */
	UFUNCTION(BlueprintCallable, Category = "Stats Tracer")
	void AddRotatorStat(
		UPARAM(ref) const FRotator& value,
		const FString& name,
		const FString& group = "",
		const FString& description = "",
		const FColor color = FColor(0, 0, 0, 0),
		const bool streamToCsv = true);
	
	/** Adds a new data-source for a FTransform type variable of the target object to the data-repository. */
	UFUNCTION(BlueprintCallable, Category = "Stats Tracer")
	void AddTransformStat(
		UPARAM(ref) const FTransform& value,
		const FString& name,
		const FString& group = "",
		const FString& description = "",
		const FColor color = FColor(0, 0, 0, 0),
		const bool streamToCsv = true);


	/** Stats the tracer. This method should be called after all data-sources are added. After calling this method no more data-sources can be added. */
	UFUNCTION(BlueprintCallable, Category = "Stats Tracer")
	void StartTracer();

	/** Pause the tracer. Pausing the tracer will stop the sampling process of any data-sources of the target object. */
	UFUNCTION(BlueprintCallable, Category = "Stats Tracer")
	void PauseTracer();

	/** Resume the tracer. Resuming the tracer will continue the sampling process of any data-sources of the target object. */
	UFUNCTION(BlueprintCallable, Category = "Stats Tracer")
	void ResumeTracer();

	/** Stop the tracer. Stopping the tracer will stop the sampling process of any data-sources of the target object. A stopped tracer cannot be resumed. */
	UFUNCTION(BlueprintCallable, Category = "Stats Tracer")
	void StopTracer();
};
