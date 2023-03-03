///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
/// 
/// File:	StatsTracer\Public\StatsTracerProfiler.h
///
/// Summary:	Declares the statistics tracer profiler class.
///-------------------------------------------------------------------------------------------------

#pragma once

// Declare Group

DECLARE_STATS_GROUP(TEXT("StatsTracer-Plugin"), STATGROUP_StatsTracerPlugin, STATCAT_Advanced);


// Declare Stats
DECLARE_CYCLE_STAT(TEXT("CreateStatsTracer"), STAT_CreateStatsTracer, STATGROUP_StatsTracerPlugin);

DECLARE_CYCLE_STAT(TEXT("UTracerComponent::BeginPlay"), STAT_UTracerComponent_BeginPlay, STATGROUP_StatsTracerPlugin);
DECLARE_CYCLE_STAT(TEXT("UTracerComponent::ScanStats"), STAT_UTracerComponent_ScanStats, STATGROUP_StatsTracerPlugin);

DECLARE_CYCLE_STAT(TEXT("CreateSession"), STAT_CreateSession, STATGROUP_StatsTracerPlugin);
DECLARE_CYCLE_STAT(TEXT("RemoveSession"), STAT_RemoveSession, STATGROUP_StatsTracerPlugin);

DECLARE_CYCLE_STAT(TEXT("CreateRepository"), STAT_CreateRepository, STATGROUP_StatsTracerPlugin);
DECLARE_CYCLE_STAT(TEXT("RemoveRepository"), STAT_RemoveRepository, STATGROUP_StatsTracerPlugin);
DECLARE_CYCLE_STAT(TEXT("UpdateRepository"), STAT_UpdateRepository, STATGROUP_StatsTracerPlugin);

DECLARE_CYCLE_STAT(TEXT("AddDatasource"), STAT_AddDatasource, STATGROUP_StatsTracerPlugin);
DECLARE_CYCLE_STAT(TEXT("SampleDatasource"), STAT_SampleDatasource, STATGROUP_StatsTracerPlugin);

DECLARE_CYCLE_STAT(TEXT("CSVStream::operator<<"), STAT_CSVSteamOperator, STATGROUP_StatsTracerPlugin);
DECLARE_CYCLE_STAT(TEXT("CSVStream::Flush()"), STAT_CSVFlush, STATGROUP_StatsTracerPlugin);

DECLARE_CYCLE_STAT(TEXT("UpdatePhysicalMemoryUsage"), STAT_UpdatePhysicalMemoryUsage, STATGROUP_StatsTracerPlugin);