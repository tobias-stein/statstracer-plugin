///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Private\StatsTracerEditorCore.cpp
///
/// Summary:	Implements the statistics tracer editor core class.
///-------------------------------------------------------------------------------------------------

#include "StatsTracerEditorCore.h"
#include "StatsTracerEditorPCH.h"


namespace StatsTracer {

	///-------------------------------------------------------------------------------------------------
	/// Class:	IDataSourceView
	///-------------------------------------------------------------------------------------------------
	
	IDataSourceView::IDataSourceView()
	{}

	IDataSourceView::~IDataSourceView()
	{}

	///-------------------------------------------------------------------------------------------------
	/// explicit template instantiation
	///-------------------------------------------------------------------------------------------------
	 
	template class FSimpleDataSourceView<bool>;
	template class FSimpleDataSourceView<int32>;
	template class FSimpleDataSourceView<float>;
	template class FSimpleDataSourceView<uint8>;
	template class FSimpleDataSourceView<FVector>;
	template class FSimpleDataSourceView<FRotator>;
	template class FSimpleDataSourceView<FTransform>;

} // namespace StatsTracer