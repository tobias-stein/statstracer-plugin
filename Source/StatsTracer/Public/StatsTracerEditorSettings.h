///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracer\Public\StatsTracerEditorSettings.h
///
/// Summary:	Declares the statistics tracer editor settings class.
///-------------------------------------------------------------------------------------------------

#pragma once

#include "Engine.h"
#include "StatsTracerEditorSettings.generated.h"


UENUM(BlueprintType)
enum class ETimelineMode : uint8
{
	None            UMETA(DisplayName = "None"),
	Time            UMETA(DisplayName = "Time"),
	RelativeTime    UMETA(DisplayName = "Relative Time"),
	Frame           UMETA(DisplayName = "Frame"),
	RelativeFrame   UMETA(DisplayName = "Relative Frame")
};

/**
 * 
 */
UCLASS(config = Editor, defaultconfig)
class STATSTRACER_API UStatsTracerEditorSettings : public UObject
{
	GENERATED_BODY()

	static const int32 GRID_SCALE_STEP_SIZE { 5 };

public:

	UStatsTracerEditorSettings(const FObjectInitializer& ObjectInitializer);

	static UStatsTracerEditorSettings* GetInstance() { return GetMutableDefault<UStatsTracerEditorSettings>(); }

	///-------------------------------------------------------------------------------------------------
	/// General 
	///-------------------------------------------------------------------------------------------------

	/** The total amount of samples each tracer can store per session. If the number of sampled data exceeds this buffer size the oldest data sample will be dropped. */
	UPROPERTY(
		config, 
		EditAnywhere, 
		Category = General, 
		meta = (
			UIMin = 512, ClampMin = 512, 
			UIMax = 16384, ClampMax = 16384,
			DisplayName = "Tracer databuffer size"))
	int32 SampleWindowSize;

	/** The frequency tracers will sample data. This scale is in frames, that is, a frequency of 1 means that each frame data is sampled, a frequency of 5 means every 5 frames ... */
	UPROPERTY(
		config,
		EditAnywhere,
		Category = General,
		meta = (
			UIMin = 1, ClampMin = 1, 
			UIMax = 120, ClampMax = 120,
			DisplayName = "Tracer sample frequency (Frames)"))
	int32 UpdateFrequency;

	/** The maximum number of sessions stored temporarily until the oldest session will be removed. Set this value to zero, if you do not want any session to be removed. (BE AWARE OF POSSIBLE OUT-OF-MEMORY SITUATIONS!) */
	UPROPERTY(
		config,
		EditAnywhere,
		Category = General,
		meta = (
			UIMin = 0, ClampMin = 0, 
			UIMax = 100, ClampMax = 100,
			DisplayName = "Maximum stored sessions"))
	int32 SessionCapacity;

	/** The maximum allowance of memory the StatsTracer plugin is permitted to use for storing session data. The unit scele is in megabytes. */
	UPROPERTY(
		config,
		EditAnywhere,
		Category = General,
		meta = (
			UIMin = 4, ClampMin = 4,
			DisplayName = "StatsTracer memory cap (Mbyte)"))
	int32 PhysicalMemoryLimit;


	///-------------------------------------------------------------------------------------------------
	/// Chart Visual Appearance
	///-------------------------------------------------------------------------------------------------
	 
	/** Enable this option to display a grid in data charts. */
	UPROPERTY(
		config,
		EditAnywhere,
		Category = ChartVisualAppearance,
		meta = (
			DisplayName = "Show grid"))
	bool ChartShowGrid;

	/** Enable this option to display reference lines at the current mouse coursor position when hovering a data plot area. */
	UPROPERTY(
		config,
		EditAnywhere,
		Category = ChartVisualAppearance,
		meta = (
			DisplayName = "Show reference pointer"))
	bool ChartShowIndicatorCross;

	/** Enable this option to display additional text label for multi-dimensional data source (Vector, Rotator, Transform). This make it easier to distinguish line-plots of the same source in a chart. */
	UPROPERTY(
		config,
		EditAnywhere,
		Category = ChartVisualAppearance,
		meta = (
			DisplayName = "Display additional labels"))
	bool ChartShowMultiStatsSubLabels;

	/** The X-axis grid line scale. */
	UPROPERTY(
		config,
		EditAnywhere,
		Category = ChartVisualAppearance,
		meta = (
			UIMin = 10, ClampMin = 10, 
			UIMax = 250, ClampMax = 250,
			FixedIncrement = 5,
			DisplayName = "X-axis grid scale (Pixel)"))
	int32 ChartXAxisGridSize;

	/** The Y-axis grid line scale. */
	UPROPERTY(
		config,
		EditAnywhere,
		Category = ChartVisualAppearance,
		meta = (
			UIMin = 10, ClampMin = 10, 
			UIMax = 250, ClampMax = 250,
			FixedIncrement = 5,
			DisplayName = "Y-axis grid scale (Pixel)"))
	int32 ChartYAxisGridSize;

	/** Sets the X-Axis mode for displaying points of references. A point of reference can be a frame or the elapsed time. */
	UPROPERTY(
		config,
		EditAnywhere,
		Category = ChartVisualAppearance,
		meta = (
			DisplayName = "X-axis timeline mode"))
	ETimelineMode ChartXAxisTimelineMode;

	/** Sets the reference pointer mode for displaying it's label. */
	UPROPERTY(
		config,
		EditAnywhere,
		Category = ChartVisualAppearance,
		meta = (
			DisplayName = "Reference pointer timeline mode"))
	ETimelineMode ChartReferencePointerTimelineMode;

	/** The root directory where all CSV files will be saved to. */
	UPROPERTY(
		config,
		EditAnywhere,
		Category = CsvSetting,
		meta = (
			RelativePath,
			DisplayName = "Csv file location"))
	FDirectoryPath CsvOutputDir;

	/** To reduce the result set of stats detected by 'Tracer' components you can specify filter here. Each entry resembles one filter. Filter can be regular expressions. */
	UPROPERTY(
		config,
		EditAnywhere,
		Category = TracerComponent,
		meta = (DisplayName = "Global stats filter"))
	TArray<FString> GlobalStatsFilter;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
