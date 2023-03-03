// Fill out your copyright notice in the Description page of Project Settings.

#include "StatsTracerSessionRunner.h"
#include "StatsTracerPCH.h"

// Sets default values
AStatsTracerSessionRunner::AStatsTracerSessionRunner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	this->SetActorHiddenInGame(true);
	this->SetActorEnableCollision(false);
}

// Called when the game starts or when spawned
void AStatsTracerSessionRunner::BeginPlay()
{
	Super::BeginPlay();

	if (StatsTracer::TDRM == nullptr)
		Destroy();
}

// Called every frame
void AStatsTracerSessionRunner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// we simply abuse this actor to update the current active tracer session for us
	StatsTracer::TDRM->UpdateActiveTracerSession(DeltaTime);
}

