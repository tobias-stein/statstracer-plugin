///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracer\Private\Tracer.cpp
///
/// Summary:	Implements the tracer class.
///-------------------------------------------------------------------------------------------------

#include "Tracer.h"
#include "StatsTracerPCH.h"

UTracer::UTracer()
{}

UTracer::~UTracer()
{}

void UTracer::Initialize(const FString& name, const FString& description, AActor* TracedActor, const bool streamToCsv, const bool autostartOnBeginPlay)
{
	if (StatsTracer::TDRM != nullptr)
	{
		// get handle to repository
		this->m_TracerDataRepository = StatsTracer::TDRM->CreateTracerRepository(name, description, TracedActor, streamToCsv, autostartOnBeginPlay);
		if (this->m_TracerDataRepository.IsValid() == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("Unable to create stats tracer for actor '%s'. Current session seems to be inactive."), *TracedActor->GetName());
		}
	}
}

void UTracer::StartTracer()
{
	if (this->m_TracerDataRepository.IsValid() == false)
		this->m_TracerDataRepository.Pin()->Start(FDateTime::UtcNow());
}

void UTracer::PauseTracer()
{
	if (this->m_TracerDataRepository.IsValid() == false)
		this->m_TracerDataRepository.Pin()->Pause();
}

void UTracer::ResumeTracer()
{
	if (this->m_TracerDataRepository.IsValid() == false)
		this->m_TracerDataRepository.Pin()->Resume();
}
void UTracer::StopTracer()
{
	if (this->m_TracerDataRepository.IsValid() == false)
		this->m_TracerDataRepository.Pin()->Stop();
}

void UTracer::AddBoolStat(
	UPARAM(ref) const bool& value,
	const FString& name,
	const FString& group,
	const FString& description,
	const FColor color,
	const bool streamToCsv)
{
	if (this->m_TracerDataRepository.IsValid())
	{
		auto pinned = m_TracerDataRepository.Pin();
		pinned->AddDataSource(
			new StatsTracer::FBoolDataSource(
				&value,
				name,
				group,
				description,
				color == FColor::Transparent ? pinned->GetNextDefaultDataSourceColor() : color,
				streamToCsv));
	}
}

void UTracer::AddIntStat(
	UPARAM(ref) const int32& value,
	const FString& name,
	const FString& group,
	const FString& description,
	const FColor color,
	const bool streamToCsv)
{
	if (this->m_TracerDataRepository.IsValid())
	{
		auto pinned = m_TracerDataRepository.Pin();
		pinned->AddDataSource(
			new StatsTracer::FIntDataSource(
				&value,
				name,
				group,
				description,
				color == FColor::Transparent ? pinned->GetNextDefaultDataSourceColor() : color,
				streamToCsv));
	}
}

void UTracer::AddFloatStat(
	UPARAM(ref) const float& value,
	const FString& name,
	const FString& group,
	const FString& description,
	const FColor color,
	const bool streamToCsv)
{
	if (this->m_TracerDataRepository.IsValid())
	{
		auto pinned = m_TracerDataRepository.Pin();
		pinned->AddDataSource(
			new StatsTracer::FFloatDataSource(
				&value,
				name,
				group,
				description,
				color == FColor::Transparent ? pinned->GetNextDefaultDataSourceColor() : color,
				streamToCsv));
	}
}

void UTracer::AddByteStat(
	UPARAM(ref) const uint8& value,
	const FString& name,
	const FString& group,
	const FString& description,
	const FColor color,
	const bool streamToCsv)
{
	if (this->m_TracerDataRepository.IsValid())
	{
		auto pinned = m_TracerDataRepository.Pin();
		pinned->AddDataSource(
			new StatsTracer::FByteDataSource(
				&value,
				name,
				group,
				description,
				color == FColor::Transparent ? pinned->GetNextDefaultDataSourceColor() : color,
				streamToCsv));
	}
}


void UTracer::AddVectorStat(
	UPARAM(ref) const FVector& value,
	const FString& name,
	const FString& group,
	const FString& description,
	const FColor color,
	const bool streamToCsv)
{
	if (this->m_TracerDataRepository.IsValid())
	{
		auto pinned = m_TracerDataRepository.Pin();
		pinned->AddDataSource(
			new StatsTracer::FVectorDataSource(
				&value,
				name,
				group.IsEmpty() ? name : group,
				description,
				color == FColor::Transparent ? pinned->GetNextDefaultDataSourceColor() : color,
				streamToCsv));
	}
}

void UTracer::AddRotatorStat(
	UPARAM(ref) const FRotator& value,
	const FString& name,
	const FString& group,
	const FString& description,
	const FColor color,
	const bool streamToCsv)
{
	if (this->m_TracerDataRepository.IsValid())
	{
		auto pinned = m_TracerDataRepository.Pin();
		pinned->AddDataSource(
			new StatsTracer::FRotatorDataSource(
				&value,
				name,
				group.IsEmpty() ? name : group,
				description,
				color == FColor::Transparent ? pinned->GetNextDefaultDataSourceColor() : color,
				streamToCsv));
	}
}

void UTracer::AddTransformStat(
	UPARAM(ref) const FTransform& value,
	const FString& name,
	const FString& group,
	const FString& description,
	const FColor color,
	const bool streamToCsv)
{
	if (this->m_TracerDataRepository.IsValid())
	{
		auto pinned = m_TracerDataRepository.Pin();
		pinned->AddDataSource(
			new StatsTracer::FTransformDataSource(
				&value,
				name,
				group.IsEmpty() ? name : group,
				description,
				color == FColor::Transparent ? pinned->GetNextDefaultDataSourceColor() : color,
				streamToCsv));
	}
}