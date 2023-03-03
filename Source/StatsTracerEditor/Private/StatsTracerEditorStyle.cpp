///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Private\StatsTracerEditorStyle.cpp
///
/// Summary:	Implements the statistics tracer editor style class.
///-------------------------------------------------------------------------------------------------

#include "StatsTracerEditorStyle.h"
#include "StatsTracerEditorPCH.h"

#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

#include "Runtime/Launch/Resources/Version.h"

TSharedPtr< FSlateStyleSet > FStatsTracerEditorStyle::StyleInstance = NULL;

void FStatsTracerEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FStatsTracerEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FStatsTracerEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("StatsTracerEditorStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define TTF_CORE_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToCoreContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon24x24(24.0f, 24.0f);
const FVector2D Icon32x32(32.0f, 32.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef< FSlateStyleSet > FStatsTracerEditorStyle::Create()
{
	// create new sharable style
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));

	// use unreals Slates editor content as root content folder
	Style->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	Style->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	// This is actually necessary for setting the Button icon (Toolbar and menu)
	Style->Set("StatsTracerEditor.OpenPluginWindow", new FSlateImageBrush(IPluginManager::Get().FindPlugin("StatsTracer")->GetBaseDir() / TEXT("Resources/Icon128.png"), Icon40x40));

	// Fonts
	{
		// regular font
		Style->Set("HugeFont", TTF_CORE_FONT("Fonts/Roboto-Regular", 18));
		Style->Set("LargeFont", TTF_CORE_FONT("Fonts/Roboto-Regular", 14));
		Style->Set("BigFont", TTF_CORE_FONT("Fonts/Roboto-Regular", 12));
		Style->Set("NormalFont", TTF_CORE_FONT("Fonts/Roboto-Regular", 10));
		Style->Set("SmallFont", TTF_CORE_FONT("Fonts/Roboto-Regular", 7));


		if (FEngineVersion::Current().GetMajor() == 4 && FEngineVersion::Current().GetMinor() == 18)
		{
			// bold font
			Style->Set("HugeBoldFont", TTF_FONT("Fonts/Roboto-Black", 18));
			Style->Set("LargeBoldFont", TTF_FONT("Fonts/Roboto-Black", 14));
			Style->Set("BigBoldFont", TTF_FONT("Fonts/Roboto-Black", 12));
			Style->Set("NormalBoldFont", TTF_FONT("Fonts/Roboto-Black", 10));
			Style->Set("SmallBoldFont", TTF_FONT("Fonts/Roboto-Black", 7));

			// italic font
			Style->Set("HugeItalicFont", TTF_FONT("Fonts/Roboto-Italic", 18));
			Style->Set("LargeItalicFont", TTF_FONT("Fonts/Roboto-Italic", 14));
			Style->Set("BigItalicFont", TTF_FONT("Fonts/Roboto-Italic", 12));
			Style->Set("NormalItalicFont", TTF_FONT("Fonts/Roboto-Italic", 10));
			Style->Set("SmallItalicFont", TTF_FONT("Fonts/Roboto-Italic", 7));
		}
		// Seems that italic and black Roboto fonts have been moved since 4.18
		else if (FEngineVersion::Current().GetMajor() == 4 && FEngineVersion::Current().GetMinor() == 19)
		{
			// bold font
			Style->Set("HugeBoldFont", TTF_CORE_FONT("Fonts/Roboto-Black", 18));
			Style->Set("LargeBoldFont", TTF_CORE_FONT("Fonts/Roboto-Black", 14));
			Style->Set("BigBoldFont", TTF_CORE_FONT("Fonts/Roboto-Black", 12));
			Style->Set("NormalBoldFont", TTF_CORE_FONT("Fonts/Roboto-Black", 10));
			Style->Set("SmallBoldFont", TTF_CORE_FONT("Fonts/Roboto-Black", 7));

			// italic font
			Style->Set("HugeItalicFont", TTF_CORE_FONT("Fonts/Roboto-Italic", 18));
			Style->Set("LargeItalicFont", TTF_CORE_FONT("Fonts/Roboto-Italic", 14));
			Style->Set("BigItalicFont", TTF_CORE_FONT("Fonts/Roboto-Italic", 12));
			Style->Set("NormalItalicFont", TTF_CORE_FONT("Fonts/Roboto-Italic", 10));
			Style->Set("SmallItalicFont", TTF_CORE_FONT("Fonts/Roboto-Italic", 7));
		}
	} 

	// Brushes
	{
		Style->Set("PlayIcon", new IMAGE_BRUSH("Icons/icon_playInSelectedViewport_40x", Icon40x40));
		Style->Set("PauseIcon", new IMAGE_BRUSH("Icons/icon_pause_40x", Icon40x40));
		Style->Set("StopIcon", new IMAGE_BRUSH("Icons/icon_stop_40x", Icon40x40));
		Style->Set("DeleteIcon", new IMAGE_BRUSH("Launcher/Launcher_Delete", Icon32x32));
		Style->Set("ZoomIcon", new IMAGE_BRUSH("Common/SearchGlass", Icon16x16));
		Style->Set("ShowDataIcon", new IMAGE_BRUSH("Icons/Help/icon_Help_support_16x", Icon16x16));
		Style->Set("VisibleIcon", new IMAGE_BRUSH("Icons/icon_levels_visible_16px", Icon16x16));
		Style->Set("VisibleIcon.Highlighted", new IMAGE_BRUSH("Icons/icon_levels_visible_hi_16px", Icon16x16));
		Style->Set("NotVisibleIcon", new IMAGE_BRUSH("Icons/icon_levels_invisible_16px", Icon16x16));
		Style->Set("NotVisibleIcon.Highlighted", new IMAGE_BRUSH("Icons/icon_levels_invisible_hi_16px", Icon16x16));
		Style->Set("DataGraphIcon", new IMAGE_BRUSH("Icons/Profiler/profiler_ShowGraphData_32x", Icon32x32));
		Style->Set("StatsIcon", new IMAGE_BRUSH("Icons/icon_eject_40x", Icon40x40));
		Style->Set("RefreshIcon", new IMAGE_BRUSH("NewsFeed/ReloadButton", Icon24x24));
		Style->Set("FolderClosedIcon", new IMAGE_BRUSH("Icons/FolderClosed", Icon32x32));
		Style->Set("FolderOpenIcon", new IMAGE_BRUSH("Icons/FolderOpen", Icon32x32));
		Style->Set("PurgeIcon", new IMAGE_BRUSH("Icons/Edit/icon_Edit_Delete_40x", Icon32x32));
		Style->Set("TargetIcon", new IMAGE_BRUSH("Icons/icon_Animation_Retarget_Skeleton_16x", Icon32x32));

		Style->Set("CSVStreamOnIcon", new IMAGE_BRUSH("Icons/icon_levels_SaveModified_40x", Icon24x24));
		Style->Set("CSVStreamOffIcon", new IMAGE_BRUSH("Icons/icon_levels_SaveDisabled_40x", Icon24x24));

		Style->Set("TracerIcon.White", new IMAGE_BRUSH("Icons/icon_TextureEd_AlphaChannel_40x", Icon32x32));
		Style->Set("TracerIcon.Red", new IMAGE_BRUSH("Icons/icon_TextureEd_RedChannel_40x", Icon32x32));
		Style->Set("TracerIcon.Green", new IMAGE_BRUSH("Icons/icon_TextureEd_GreenChannel_40x", Icon32x32));
		Style->Set("TracerIcon.Blue", new IMAGE_BRUSH("Icons/icon_TextureEd_BlueChannel_40x", Icon32x32));
		Style->Set("TracerIcon.YellowGreen", new IMAGE_BRUSH("Icons/icon_TextureEd_Saturation_40x", Icon32x32));



	}

	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef TTF_CORE_FONT
#undef OTF_FONT

void FStatsTracerEditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FStatsTracerEditorStyle::Get()
{
	return *StyleInstance;
}
