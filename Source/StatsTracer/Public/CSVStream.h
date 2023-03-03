///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracer\Public\CSVStream.h
///
/// Summary:	Declares the CSV stream class.
///-------------------------------------------------------------------------------------------------

#pragma once

#include "CoreMinimal.h"



namespace StatsTracer {

	///-------------------------------------------------------------------------------------------------
	/// Class:	CSVStream
	///
	/// Summary:	A CSV stream.
	///
	/// Author:	Tobias Stein
	///
	/// Date:	8/03/2018
	///-------------------------------------------------------------------------------------------------

	class CSVStream
	{
		static const TCHAR SEPERATOR { ',' };

	public:

		enum Manipulator
		{
			endl = 0
		};

	private:

		IFileHandle*	m_FileHandle;

		FString			m_Buffer;

	public:

		CSVStream(const FString& sessionName, const FString& tracerName);
		~CSVStream();

		void Flush();
		void Close();

		bool IsValid() const;

		friend CSVStream& operator<<(CSVStream& stream, const CSVStream::Manipulator& mutator);

		friend CSVStream& operator<<(CSVStream& stream, const FString& value);
		friend CSVStream& operator<<(CSVStream& stream, const bool& value);
		friend CSVStream& operator<<(CSVStream& stream, const uint8& value);
		friend CSVStream& operator<<(CSVStream& stream, const int32& value);
		friend CSVStream& operator<<(CSVStream& stream, const float& value);
		friend CSVStream& operator<<(CSVStream& stream, const FVector& value);
		friend CSVStream& operator<<(CSVStream& stream, const FRotator& value);
		friend CSVStream& operator<<(CSVStream& stream, const FTransform& value);

	private:

		void Write();
	};

} // namespace StatsTracer