///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracer\Private\StatsTracerCore.cpp
///
/// Summary:	Implements the statistics tracer core class.
///-------------------------------------------------------------------------------------------------

#include "StatsTracerCore.h"
#include "StatsTracerPCH.h"

namespace StatsTracer {


	template<class T>
	const T ZeroSample<T>::Value { T() };

	const bool ZeroSample<bool>::Value{ false };
	const int32 ZeroSample<int32>::Value{ 0 };
	const float ZeroSample<float>::Value{ 0.0f };
	const uint8 ZeroSample<uint8>::Value{ 0 };
	const FVector ZeroSample<FVector>::Value { FVector(0.0f) }; 
	const FRotator ZeroSample<FRotator>::Value { FRotator(0.0f) }; 
	const FTransform ZeroSample<FTransform>::Value { FTransform::Identity }; 



	IDataSample::IDataSample() :
		Frame(0),
		ElapsedTime(0.0f)
	{}

	IDataSample::~IDataSample()
	{}


	///-------------------------------------------------------------------------------------------------
	/// Class:	IDataSource
	///-------------------------------------------------------------------------------------------------

	IDataSource::IDataSource(
		const FString& name, 
		const FString& group, 
		const FString& description, 
		const FColor& color, 
		const bool streamToCsv) :
		m_Name(name),
		m_Group(group),
		m_Description(FText::FromString(description)),
		m_Color(color),
		m_StreamToCsv(streamToCsv)
	{}

	IDataSource::~IDataSource()
	{}


	///-------------------------------------------------------------------------------------------------
	/// Class:	FTracerDataRepository
	///-------------------------------------------------------------------------------------------------

	FTracerDataRepository::FTracerDataRepository(const FString& name, const FString& repositoryDescription, AActor* tracedActor, TSharedPtr<FTracerSession> session, const bool streamToCsv, const bool autostart) :
		m_RepositoryId(tracedActor->GetUniqueID()),
		m_RepositoryName(name),
		m_RepositoryDescription(FText::FromString(repositoryDescription)),
		m_TracedActor(tracedActor),
		m_TracedActorFName(tracedActor->GetFName()),
		m_Session(session),
		m_State(INITIALIZED),
		m_StreamToCsv(streamToCsv),
		m_AutoStartOnBeginPlay(autostart),
		m_CSVStream(nullptr),
		m_NextColorStartHue(0.0f)
	{}

	FTracerDataRepository::~FTracerDataRepository()
	{
		if (this->m_CSVStream != nullptr)
		{
			delete this->m_CSVStream;
			this->m_CSVStream = nullptr;
		}

		this->m_TracedActor = nullptr;
		this->m_Session = nullptr;
		this->m_DataGroups.Empty();
	}

	void FTracerDataRepository::AddDataSource(IDataSource* dataSourcePtr)
	{
		SCOPE_CYCLE_COUNTER(STAT_AddDatasource);

		if (this->m_State > INITIALIZED)
		{
			UE_LOG(LogTemp, Warning, TEXT("Tracer data-repository is already active, no more data-sources can be added."));
			return;
		}

		/* DO NOT TRACE THE SAME STAT TWICE! */
		for (auto GROUP : this->m_DataGroups)
		{
			for (auto SOURCE : GROUP.Value)
			{
				if (dataSourcePtr->GetRawDataPtr() == SOURCE->GetRawDataPtr())
				{
					UE_LOG(LogTemp, Warning, TEXT("DataSources '%s' and '%s' are tracing the same stat. Ignoring duplicate data source."), *SOURCE->GetName(), *dataSourcePtr->GetName());
					return;
				}
			}
		}

		// create smart pointer for memory management
		TDataSourceHandle dataSource { dataSourcePtr };

		// If check if there is still enough memory to store this datasource
		{
			uint32 dataSourceTotalMemorySize = dataSource->GetDataSourcePhysicalMemorySize();
			if (StatsTracer::TDRM->GetAvailablePhysicalMemory() < dataSourceTotalMemorySize)
			{
				UE_LOG(LogTemp, Warning, TEXT("StatsTracer plugin reached memory limitation. '%s' stat will not be traced."), *dataSource->GetName());
				StatsTracer::TDRM->ConsumePhysicalMemory(0);
				return;
			}
			else
			{
				StatsTracer::TDRM->ConsumePhysicalMemory(dataSourceTotalMemorySize);
			}
		}


		FString group = dataSource->GetGroup();
		group = group.TrimStartAndEnd().Equals("") == true ? FString("Default") : dataSource->GetGroup();

		TArray<TDataSourceHandle>* dataSourceGroup = this->m_DataGroups.Find(*group);
		if(dataSourceGroup != nullptr)
		{
			// duplicate check
			bool result = dataSourceGroup->ContainsByPredicate([&](TDataSourceHandle& other) { return dataSource->GetName().Equals(other->GetName()); });
			if (result == false)
			{
				dataSourceGroup->Add(dataSource);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("DataSource \'%s\' already exists in group \'%s\'. DataSource will be ignored."), *dataSource->GetName(), *group);
			}
		}
		// group does not exist yet, create it.
		else
		{
			this->m_DataGroups.Add(group).Add(dataSource);
		}
	}

	void FTracerDataRepository::Update(uint64 frame, float ElapsedTime, bool forceUpdate)
	{
		SCOPE_CYCLE_COUNTER(STAT_UpdateRepository);

		if (this->m_Session.IsValid() == false)
			return;

		if (this->m_TracedActor == nullptr || this->m_TracedActor->IsValidLowLevel() == false || this->m_TracedActor->IsPendingKillOrUnreachable() == true)
		{
			this->Stop();
			return;
		}



		if (forceUpdate == true || (this->m_State == TRACING && (this->m_Session.Pin()->GetSessionState() == FTracerSession::TRACING)))
		{
			for (auto& dataGroup : this->m_DataGroups)
			{
				for (int i = 0; i < dataGroup.Value.Num(); ++i)
				{
					dataGroup.Value[i]->SampleData(frame, ElapsedTime, this->m_CSVStream);
				}
			}

			if (this->ShouldStreamToCsv() && this->m_CSVStream != nullptr)
			{
				// line break	
				(*this->m_CSVStream) << CSVStream::endl;
			}
		}
	}

	void FTracerDataRepository::Start(const FDateTime& sessionStart)
	{
		this->m_State = TRACING;

		// open csv file stream if used and there are data sources to be sampled from
		if (this->m_DataGroups.Num() > 0 && this->m_StreamToCsv == true)
		{
			const FString sessionAllias = FString::Printf(TEXT("Session-%02d%02d%04d-%02d%02d%02d%03d"),
				sessionStart.GetDay(),
				sessionStart.GetMonth(),
				sessionStart.GetYear(),
				sessionStart.GetHour(),
				sessionStart.GetMinute(),
				sessionStart.GetSecond(),
				sessionStart.GetMillisecond());

			this->m_CSVStream = new CSVStream(sessionAllias, FString::Printf(TEXT("%s-%u"), *this->m_RepositoryName, this->m_RepositoryId));
		}
		else
		{
			this->m_CSVStream = nullptr;
		}

		// write csv header
		if (this->m_CSVStream != nullptr)
		{
			CSVStream& stream = *this->m_CSVStream;

			for (auto& dataGroup : this->m_DataGroups)
			{
				for (int i = 0; i < dataGroup.Value.Num(); ++i)
				{
					if (dataGroup.Value[i]->ShouldStreamtoCsv() == true)
					{
						static const FString REPLACE_PATTERN(" ");
						static const FString REPLACEMENT("_");

						FString name = dataGroup.Value[i]->GetName(); 
						name = name.ConvertTabsToSpaces(1).TrimStartAndEnd().Replace(*REPLACE_PATTERN, *REPLACEMENT);

						switch (dataGroup.Value[i]->GetDataSourceType())
						{
							case Bool:
							case Int:
							case Float:
							case Byte:
							{
								stream << name;
								break;
							}

							case Vector:
							{
								stream << FString::Printf(TEXT("%s.X"), *name);
								stream << FString::Printf(TEXT("%s.Y"), *name);
								stream << FString::Printf(TEXT("%s.Z"), *name);
								break;
							}

							case Rotator:
							{
								stream << FString::Printf(TEXT("%s.Roll"), *name);
								stream << FString::Printf(TEXT("%s.Pitch"), *name);
								stream << FString::Printf(TEXT("%s.Yaw"), *name);
								break;
							}

							case Transform:
							{
								stream << FString::Printf(TEXT("%s.Location.X"), *name);
								stream << FString::Printf(TEXT("%s.Location.Y"), *name);
								stream << FString::Printf(TEXT("%s.Location.Z"), *name);
								stream << FString::Printf(TEXT("%s.Rotation.Roll"), *name);
								stream << FString::Printf(TEXT("%s.Rotation.Pitch"), *name);
								stream << FString::Printf(TEXT("%s.Rotation.Yaw"), *name);
								stream << FString::Printf(TEXT("%s.Scale.X"), *name);
								stream << FString::Printf(TEXT("%s.Scale.Y"), *name);
								stream << FString::Printf(TEXT("%s.Scale.Z"), *name);
								break;
							}
						}				
					}
				}
			}

			stream << CSVStream::endl;
		}
	}

	void FTracerDataRepository::Pause()
	{
		if (this->m_State >= PAUSED)
			return;

		this->m_State = PAUSED;
	}

	void FTracerDataRepository::Resume()
	{
		if (this->m_State != PAUSED)
			return;

		this->m_State = TRACING;
	}

	void FTracerDataRepository::Stop()
	{
		if (this->m_State >= STOPPED)
			return;

		this->m_TracedActor = nullptr;

		// close csv stream, iff open
		if (this->m_CSVStream != nullptr && this->m_CSVStream->IsValid() == true)
		{
			this->m_CSVStream->Close();
		}

		this->m_State = STOPPED;
	}

	void FTracerDataRepository::Complete()
	{
		if (this->m_State == COMPLETE)
			return;

		this->m_TracedActor = nullptr;

#if WITH_EDITOR									

		/*
			This is kind of a hack to retrieve the actor we just traced from the editor. The original traced
			actor pointer belongs to the PIE world instance which got cloned from the editor's.
		*/
		{
			UWorld* editorWorld = GEditor->GetEditorWorldContext().World();
			if (editorWorld != nullptr)
			{
				for (TActorIterator<AActor> It(editorWorld); It; ++It)
				{
					if (m_TracedActorFName.IsEqual(It->GetFName()) == true)
					{
						this->m_TracedActor = *It;
						break;
					}
				}
			}
		}
#endif
		this->m_State = COMPLETE;
	}
	
	bool FTracerDataRepository::HasTracedActor() const 
	{ 
		return (this->m_TracedActor != nullptr && this->m_TracedActor->IsValidLowLevel()); 
	}

	void FTracerDataRepository::SelectTracedActor()
	{
#if WITH_EDITOR
		if (this->m_TracedActor != nullptr && this->m_TracedActor->IsValidLowLevel() == true)
		{
			GEditor->SelectNone(false, false, false);
			GEditor->SelectActor(this->m_TracedActor, true, true, true, true);
			GEditor->MoveViewportCamerasToActor({ this->m_TracedActor }, true);
		}
#endif
	}

	FColor FTracerDataRepository::GetNextDefaultDataSourceColor()
	{
		return GetNextLabelColor(this->m_NextColorStartHue, &(this->m_NextColorStartHue)).ToFColor(true);
	}

	uint64 FTracerDataRepository::GetRepositoryPhysicalMemorySize()
	{
		uint64 result = 0;

		for (auto kvp : this->m_DataGroups)
		{
			for (auto it : kvp.Value)
			{
				if (it.IsValid() == true)
				{
					result += it->GetDataSourcePhysicalMemorySize();
				}
			}
		}

		return result;
	}

	///-------------------------------------------------------------------------------------------------
	/// Class:	FTracerSession
	///-------------------------------------------------------------------------------------------------

	uint32 FTracerSession::m_NextSessionId { 0 };

	FTracerSession::FTracerSession() :
		m_Id(m_NextSessionId++),
		m_SampleFrequency(UStatsTracerEditorSettings::GetInstance()->UpdateFrequency),
		m_State(TRACING),
		m_SessionStart(FDateTime::UtcNow()),
		m_RepositoryMap(new TTracerDataRepositoryMap()),
		m_FrameCounter(0),
		m_ElapsedTime(0.0f)
	{
		this->m_AlliasName = FString::Printf(TEXT("Session #%u"), this->m_Id);
	}

	FTracerSession::~FTracerSession()
	{
		this->m_RepositoryMap->Empty();
	}

	TTracerDataRepositoryHandle FTracerSession::CreateTracerRepository(const FString& repositoryName, const FString& repositoryDescription, AActor* tracedActor, const bool streamToCsv, const bool autostart)
	{
		TTracerDataRepositoryHandle* repoHandle = this->m_RepositoryMap->Find(tracedActor->GetUniqueID());
		if (repoHandle != nullptr)
			return *repoHandle;
		
		return this->m_RepositoryMap->Add(tracedActor->GetUniqueID(), TTracerDataRepositoryHandle(new FTracerDataRepository(repositoryName, repositoryDescription, tracedActor, this->AsShared(), streamToCsv, autostart)));
	}

	void FTracerSession::DeleteTracerRepository(const uint32 repositoryId)
	{
		SCOPE_CYCLE_COUNTER(STAT_RemoveRepository);

		TTracerDataRepositoryHandle* repoHandle = this->m_RepositoryMap->Find(repositoryId);

		if (repoHandle != nullptr)
		{
			this->m_RepositoryMap->Remove(repositoryId);

			StatsTracer::TDRM->UpdatePhysicalMemoryUsage();
		}
	}

	void FTracerSession::StartSession()
	{
		this->m_SessionStart = FDateTime::UtcNow();

		if (this->m_RepositoryMap.IsValid() == true)
		{
			for (auto& KVP : *this->m_RepositoryMap)
			{
				if (KVP.Value.IsValid() == true)
				{
					if(KVP.Value->ShouldAutostartOnBeginPlay() == true)
						KVP.Value->Start(this->m_SessionStart);
				}
			}
		}
	}

	void FTracerSession::PauseSession()
	{
		if (this->m_State >= PAUSED)
			return;

		this->m_State = PAUSED;
	}

	void FTracerSession::ResumeSession()
	{
		if (this->m_State != PAUSED)
			return;

		this->m_State = TRACING;
	}

	void FTracerSession::UpdateSession(float DeltaTime, bool forceUpdate)
	{
		if (this->m_RepositoryMap.IsValid() == true)
		{
			// increase elpased session time
			this->m_ElapsedTime += DeltaTime;

			if ((this->m_FrameCounter % this->m_SampleFrequency) == 0)
			{
				for (auto& KVP : *this->m_RepositoryMap)
				{
					if (KVP.Value.IsValid() == true)
					{
						KVP.Value->Update(this->m_FrameCounter, this->m_ElapsedTime, forceUpdate);
					}
				}
			}

			// increase frame counter
			this->m_FrameCounter++;
		}
	}

	void FTracerSession::StopSession()
	{
		if (this->m_State >= STOPPED)
			return;

		this->m_SessionEnd = FDateTime::UtcNow();

		if (this->m_RepositoryMap.IsValid() == true)
		{
			for (auto& KVP : *this->m_RepositoryMap)
			{
				if (KVP.Value.IsValid() == true)
				{
					KVP.Value->Stop();
				}
			}
		}

		this->m_State = STOPPED;
	}

	void FTracerSession::EndSession()
	{
		if (this->m_State == COMPLETE)
			return;

		StopSession();
	

		if (this->m_RepositoryMap.IsValid() == true)
		{
			for (auto& KVP : *this->m_RepositoryMap)
			{
				if (KVP.Value.IsValid() == true)
				{
					KVP.Value->Complete();
				}
			}
		}

		this->m_State = COMPLETE;
	}

	uint64 FTracerSession::GetSessionPhysicalMemorySize()
	{
		uint64 result = 0;

		if (this->m_RepositoryMap.IsValid())
		{
			for (auto& KVP : *this->m_RepositoryMap)
			{
				if (KVP.Value.IsValid())
				{
					result += KVP.Value->GetRepositoryPhysicalMemorySize();
				}
			}
		}

		return result;
	}


	///-------------------------------------------------------------------------------------------------
	/// Class:	FTracerDataRepositoryManager
	///-------------------------------------------------------------------------------------------------

	FTracerDataRepositoryManager::FTracerDataRepositoryManager() :
		m_TotalPhysicalMemorySize(0),
		m_RatioTotalLimit(0.0f),
		m_AvailalbeMemory(0)
	{
		RegisterEditorDelegates();
	}


	FTracerDataRepositoryManager::~FTracerDataRepositoryManager()
	{
		UnregisterEditorDelegates();

		// clear all sessions
		this->m_Sessions.Empty();
	}

	void FTracerDataRepositoryManager::InitializeNewTracerSession()
	{
		SCOPE_CYCLE_COUNTER(STAT_CreateSession);

		UpdatePhysicalMemoryUsage();

		this->m_Sessions.Add(TTracerSessionHandle(new FTracerSession()));
	}

	void FTracerDataRepositoryManager::StartActiveTracerSession()
	{
		if (this->m_Sessions.Num() == 0)
			return;

		auto& sessionHandle = this->m_Sessions.Last();
		check(sessionHandle.IsValid() && "Invalid tracer session!");

		// start active session
		sessionHandle->StartSession();
	}

	void FTracerDataRepositoryManager::PauseActiveTracerSession()
	{
		if (this->m_Sessions.Num() == 0)
			return;

		auto& sessionHandle = this->m_Sessions.Last();
		check(sessionHandle.IsValid() && "Invalid tracer session!");

		// pause active session
		sessionHandle->PauseSession();
	}

	void FTracerDataRepositoryManager::ResumeActiveTracerSession()
	{
		if (this->m_Sessions.Num() == 0)
			return;

		auto& sessionHandle = this->m_Sessions.Last();
		check(sessionHandle.IsValid() && "Invalid tracer session!");

		// resume active session
		sessionHandle->ResumeSession();
	}

	void FTracerDataRepositoryManager::UpdateActiveTracerSession(float DeltaTime, bool forceUpdate)
	{
		if (this->m_Sessions.Num() == 0)
			return;

		auto& sessionHandle = this->m_Sessions.Last();
		check(sessionHandle.IsValid() && "Invalid tracer session!");

		// update active session
		sessionHandle->UpdateSession(DeltaTime, forceUpdate);
	}

	void FTracerDataRepositoryManager::StopActiveTracerSession()
	{
		if (this->m_Sessions.Num() == 0)
			return;

		auto& sessionHandle = this->m_Sessions.Last();
		check(sessionHandle.IsValid() && "Invalid tracer session!");

		// stop active session
		sessionHandle->StopSession();
	}

	void FTracerDataRepositoryManager::EndActiveTracerSession()
	{
		if (this->m_Sessions.Num() == 0)
			return;

		auto& sessionHandle = this->m_Sessions.Last();
		check(sessionHandle.IsValid() && "Invalid tracer session!");

		// end active session
		sessionHandle->EndSession();
	}

	TTracerDataRepositoryHandle FTracerDataRepositoryManager::CreateTracerRepository(const FString& repositoryName, const FString& repositoryDescription, AActor* tracedActor, const bool streamToCsv, const bool autostart)
	{		
		SCOPE_CYCLE_COUNTER(STAT_CreateRepository);

		auto& sessionHandle = this->m_Sessions.Last();
		check(sessionHandle.IsValid() && "Invalid tracer session!");

		if (sessionHandle->IsActiveSession() == false)
			return nullptr;

		return sessionHandle->CreateTracerRepository(repositoryName, repositoryDescription, tracedActor, streamToCsv, autostart);
	}

	void FTracerDataRepositoryManager::RemoveSession(const uint32 sessionId)
	{
		SCOPE_CYCLE_COUNTER(STAT_RemoveSession);

		bool found = false;
		int32 i = 0;
		for (i; i < this->m_Sessions.Num(); ++i)
		{
			if (this->m_Sessions[i]->GetSessionId() == sessionId)
			{
				found = true;
				break;
			}
		}

		if (found == true)
		{
			this->m_Sessions[i].Reset();
			this->m_Sessions.RemoveAt(i);

			UpdatePhysicalMemoryUsage();
		}
	}
	
	void FTracerDataRepositoryManager::ConsumePhysicalMemory(uint64 memorySize)
	{ 
		const uint64 LIMIT_BYTES = (uint64)UStatsTracerEditorSettings::GetInstance()->PhysicalMemoryLimit * 1048576; // convert Mbytes to bytes

		this->m_TotalPhysicalMemorySize += memorySize; 

		this->m_AvailalbeMemory = LIMIT_BYTES - this->m_TotalPhysicalMemorySize;		
		this->m_RatioTotalLimit = FMath::Clamp((float)this->m_TotalPhysicalMemorySize / FMath::Max<float>(1.0f, (float)LIMIT_BYTES), 0.0f, 1.0f);
	}

	TArray<TWeakTracerSessionHandle> FTracerDataRepositoryManager::GetSessionArray()
	{
		TArray<TWeakTracerSessionHandle> weakSessionArray;
		weakSessionArray.Append(this->m_Sessions);

		return weakSessionArray;
	}

	void FTracerDataRepositoryManager::UpdatePhysicalMemoryUsage()
	{	
		const uint64 LIMIT_BYTES = (uint64)UStatsTracerEditorSettings::GetInstance()->PhysicalMemoryLimit * 1048576; // convert Mbytes to bytes

		SCOPE_CYCLE_COUNTER(STAT_UpdatePhysicalMemoryUsage);

		this->m_TotalPhysicalMemorySize		= 0;
		this->m_AvailalbeMemory				= LIMIT_BYTES;

		// loop over all stored sessions
		for (auto& S : this->m_Sessions)
		{
			// make sure session is valid
			if (S.IsValid() == true)
			{
				this->m_TotalPhysicalMemorySize += S->GetSessionPhysicalMemorySize();
			}
		}
		
		this->m_AvailalbeMemory = LIMIT_BYTES - this->m_TotalPhysicalMemorySize;
		this->m_RatioTotalLimit = FMath::Clamp((float)this->m_TotalPhysicalMemorySize / FMath::Max<float>(1.0f, (float)LIMIT_BYTES), 0.0f, 1.0f);
	}

	/** Register Editor delegates. */
	void FTracerDataRepositoryManager::RegisterEditorDelegates()
	{
		FEditorDelegates::PreBeginPIE.AddRaw(this, &FTracerDataRepositoryManager::HandleEditorPreBeginPIE);
		FEditorDelegates::PostPIEStarted.AddRaw(this, &FTracerDataRepositoryManager::HandleEditorPostStartedPIE);
		FEditorDelegates::EndPIE.AddRaw(this, &FTracerDataRepositoryManager::HandleEditorEndPIE);
		FEditorDelegates::PausePIE.AddRaw(this, &FTracerDataRepositoryManager::HandleEditorPausePIE);
		FEditorDelegates::ResumePIE.AddRaw(this, &FTracerDataRepositoryManager::HandleEditorResumePIE);
		FEditorDelegates::SingleStepPIE.AddRaw(this, &FTracerDataRepositoryManager::HandleEditorSingleStepPIE);
	}

	/** Unregister Editor delegates. */
	void FTracerDataRepositoryManager::UnregisterEditorDelegates()
	{
		FEditorDelegates::PreBeginPIE.RemoveAll(this);
		FEditorDelegates::BeginPIE.RemoveAll(this);
		FEditorDelegates::PostPIEStarted.RemoveAll(this);
		FEditorDelegates::EndPIE.RemoveAll(this);
		FEditorDelegates::PausePIE.RemoveAll(this);
		FEditorDelegates::ResumePIE.RemoveAll(this);
		FEditorDelegates::SingleStepPIE.RemoveAll(this);
	}

	void FTracerDataRepositoryManager::HandleEditorPreBeginPIE(bool bIsSimulating)
	{
		// create a new session
		InitializeNewTracerSession();

		// check session capacity
		if (UStatsTracerEditorSettings::GetInstance()->SessionCapacity > 0 && GetSessionCount() > UStatsTracerEditorSettings::GetInstance()->SessionCapacity)
		{
			auto oldestSession = GetOldestSession();
			if (oldestSession.IsValid() == true)
				RemoveSession(oldestSession.Pin()->GetSessionId());
		}
	}

	void FTracerDataRepositoryManager::HandleEditorPostStartedPIE(bool bIsSimulating)
	{
		if (GEditor->GetPIEWorldContext() != nullptr && GEditor->GetPIEWorldContext()->World() != nullptr)
		{
			FActorSpawnParameters ASP;
			ASP.ObjectFlags = EObjectFlags::RF_Standalone | EObjectFlags::RF_Transient;

			// create stats tracer session runner
			GEditor->GetPIEWorldContext()->World()->SpawnActor<AStatsTracerSessionRunner>(ASP);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("StatsTracer Plugin: Unable to get PIE world instance, cannot start tracer session."));
			EndActiveTracerSession();
			return;
		}

		StartActiveTracerSession();
	}

	void FTracerDataRepositoryManager::HandleEditorEndPIE(bool bIsSimulating)
	{
		EndActiveTracerSession();
	}

	void FTracerDataRepositoryManager::HandleEditorPausePIE(bool bIsSimulating)
	{
		PauseActiveTracerSession();
	}

	void FTracerDataRepositoryManager::HandleEditorResumePIE(bool bIsSimulating)
	{
		ResumeActiveTracerSession();
	}

	void FTracerDataRepositoryManager::HandleEditorSingleStepPIE(bool bIsSimulating)
	{	
		UpdateActiveTracerSession(GEditor->GetPIEWorldContext()->World()->GetDeltaSeconds(), true);
	}

	
	///-------------------------------------------------------------------------------------------------
	/// explicit template instantiation
	///-------------------------------------------------------------------------------------------------
 
	template class FDataSample<bool>;
	template class FDataSample<int32>;
	template class FDataSample<float>;
	template class FDataSample<uint8>;
	template class FDataSample<FVector>;
	template class FDataSample<FRotator>;
	template class FDataSample<FTransform>;

	template class FDataSource<bool>;
	template class FDataSource<int32>;
	template class FDataSource<float>;
	template class FDataSource<uint8>;
	template class FDataSource<FVector>;
	template class FDataSource<FRotator>;
	template class FDataSource<FTransform>;


	///-------------------------------------------------------------------------------------------------
	/// GLOBAL Tracer-Data Repository pointer
	///-------------------------------------------------------------------------------------------------

	FTracerDataRepositoryManager* TDRM = new FTracerDataRepositoryManager();

} // namespace StatsTracer