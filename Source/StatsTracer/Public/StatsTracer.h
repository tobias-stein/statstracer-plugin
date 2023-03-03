///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracer\Public\StatsTracer.h
///
/// Summary:	Declares the statistics tracer class.
///-------------------------------------------------------------------------------------------------

#pragma once

#include "Modules/ModuleManager.h"

class FStatsTracerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

	void RegisterSettings();
	void UnregisterSettings();
};


