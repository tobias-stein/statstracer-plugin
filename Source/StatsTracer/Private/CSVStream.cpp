///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracer\Private\CSVStream.cpp
///
/// Summary:	Implements the CSV stream class.
///-------------------------------------------------------------------------------------------------

#include "CSVStream.h"
#include "StatsTracerPCH.h"


#include "HAL/PlatformFilemanager.h"
#include "GenericPlatform/GenericPlatformFile.h"

namespace StatsTracer {

	CSVStream::CSVStream(const FString& sessionName, const FString& tracerName) :
		m_Buffer(FString())
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		
		auto fileDir = FPaths::Combine(UStatsTracerEditorSettings::GetInstance()->CsvOutputDir.Path, sessionName);

		if (PlatformFile.CreateDirectoryTree(*fileDir) == true)
		{			
			FString fullFileName = FPaths::Combine(fileDir, FString::Printf(TEXT("%s.csv"), *tracerName));

			// if there is already a file with the same name add an suffix to name
			if (PlatformFile.FileExists(*fullFileName) == true)
			{
				int32 suffix = 1;
				while (true)
				{
					fullFileName = FPaths::Combine(fileDir, FString::Printf(TEXT("%s-%d.csv"), *tracerName, suffix++));

					if (PlatformFile.FileExists(*fullFileName) == true)
						continue;

					break;
				}
			}

			this->m_FileHandle = PlatformFile.OpenWrite(*fullFileName);
			if (this->m_FileHandle == nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to open csv file '%s'!"), *fullFileName);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Unable to access csv output directory '%s'!"), *fileDir);
		}
	}

	CSVStream::~CSVStream()
	{
		if (this->m_FileHandle != nullptr)
			Close();
	}

	void CSVStream::Flush()
	{
		SCOPE_CYCLE_COUNTER(STAT_CSVFlush);

		// write to file
		Write();

		// clear buffer
		this->m_Buffer.Empty();

		// flush file handle
		this->m_FileHandle->Flush();
	}

	void CSVStream::Close()
	{
		Flush();

		delete this->m_FileHandle;
		this->m_FileHandle = nullptr;
	}
	 
	bool CSVStream::IsValid() const
	{
		if (this->m_FileHandle != nullptr)
			return true;

		return false;
	}

	void CSVStream::Write()
	{
		if (this->m_Buffer.Len() > 0)
		{
			this->m_FileHandle->Write((const uint8*)TCHAR_TO_ANSI(*this->m_Buffer), this->m_Buffer.Len());
		}
	}

	CSVStream& operator<<(CSVStream& stream, const CSVStream::Manipulator& mutator)
	{
		SCOPE_CYCLE_COUNTER(STAT_CSVSteamOperator);

		switch (mutator)
		{
			case CSVStream::endl:
			{
//#if PLATFORM_WINDOWS
//				static const FString NEWLINE = FString(TEXT("\r\n"));
//#elif PLATFORM_LINUX
//				static const FString NEWLINE = FString(TEXT("\n"));
//#elif PLATFORM_MAC
//				static const FString NEWLINE = FString(TEXT("\r"));
//#else
//				static const FString NEWLINE = FString(TEXT("\r\n"));
//#endif

				if (stream.m_Buffer.Len() > 0)
				{
					// overwrite last ','
					stream.m_Buffer.RemoveAt(stream.m_Buffer.Len() - 1);
				}

				stream.m_Buffer.Append(NEWLINE);
				stream.Flush();
				break;
			}
		}

		return stream;
	}

	CSVStream& operator<<(CSVStream& stream, const FString& value)
	{
		SCOPE_CYCLE_COUNTER(STAT_CSVSteamOperator);

		stream.m_Buffer.Append(value).AppendChar(CSVStream::SEPERATOR);
		return stream;
	}

	CSVStream& operator<<(CSVStream& stream, const bool& value)
	{
		SCOPE_CYCLE_COUNTER(STAT_CSVSteamOperator);

		stream.m_Buffer.AppendChar(value == true ? '1' : '0').AppendChar(CSVStream::SEPERATOR);
		return stream;
	}

	CSVStream& operator<<(CSVStream& stream, const uint8& value)
	{
		SCOPE_CYCLE_COUNTER(STAT_CSVSteamOperator);

		stream.m_Buffer.AppendChar(value).AppendChar(CSVStream::SEPERATOR);
		return stream;
	}

	CSVStream& operator<<(CSVStream& stream, const int32& value)
	{
		SCOPE_CYCLE_COUNTER(STAT_CSVSteamOperator);

		stream.m_Buffer.Append(FString::FromInt(value)).AppendChar(CSVStream::SEPERATOR);
		return stream;
	}

	CSVStream& operator<<(CSVStream& stream, const float& value)
	{
		SCOPE_CYCLE_COUNTER(STAT_CSVSteamOperator);

		stream.m_Buffer.Append(FString::SanitizeFloat(value)).AppendChar(CSVStream::SEPERATOR);
		return stream;
	}

	CSVStream& operator<<(CSVStream& stream, const FVector& value)
	{
		stream << value.X << value.Y << value.Z;
		return stream;
	}

	CSVStream& operator<<(CSVStream& stream, const FRotator& value)
	{
		stream << value.Roll << value.Pitch << value.Yaw;
		return stream;
	}

	CSVStream& operator<<(CSVStream& stream, const FTransform& value)
	{
		stream << value.GetLocation() << value.GetRotation().Rotator() << value.GetScale3D();
		return stream;
	}

} // namespace StatsTracer