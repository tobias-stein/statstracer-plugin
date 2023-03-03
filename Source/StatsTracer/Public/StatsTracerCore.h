///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracer\Public\StatsTracerCore.h
///
/// Summary:	Declares the statistics tracer core class.
///-------------------------------------------------------------------------------------------------

#pragma once

#include "Engine.h"
#include "Containers/CircularBuffer.h"
#include "CSVStream.h"
#include "StatsTracerEditorSettings.h"

namespace StatsTracer {

	class CSVStream;

	class IDataSource;
	class FTracerSession;
	class FTracerDataRepository;

	/// Summary:	A list of all possible tracable properties. This list contains the CppTypeNames.
	static const FString TracablePropertiesCppTypeNames[] 
	{
		"bool",
		"int32",
		"float",
		"uint8",
		"FVector",
		"FRotator",
		"FTransform"
	};

	///-------------------------------------------------------------------------------------------------
	/// Fn:	static bool IsTracableProperty(const FString& InPropertyCppName)
	///
	/// Summary:	Returns true, if the given CppType name is one of the possible tracable properties.
	///
	/// Author:	Tobias Stein
	///
	/// Date:	7/03/2018
	///
	/// Parameters:
	/// InPropertyCppName - 	Name of the in property C++.
	///
	/// Returns:	True if tracable property, false if not.
	///-------------------------------------------------------------------------------------------------

	static bool IsTracableProperty(const FString& InPropertyCppName)
	{
		for (auto n : TracablePropertiesCppTypeNames)
		{
			if (n.Equals(InPropertyCppName) == true)
				return true;
		}

		return false;
	}

	///-------------------------------------------------------------------------------------------------
	/// Fn:	static bool IsMultidimensionalTracableProperty(const FString& InPropertyCppName)
	///
	/// Summary:	True, if the tracable type is multi-dimensional, that is a vector or transform.
	///
	/// Author:	Tobias Stein
	///
	/// Date:	7/03/2018
	///
	/// Parameters:
	/// InPropertyCppName - 	Name of the in property C++.
	///
	/// Returns:	True if multidimensional tracable property, false if not.
	///-------------------------------------------------------------------------------------------------

	static bool IsMultidimensionalTracableProperty(const FString& InPropertyCppName)
	{
		if (InPropertyCppName.Equals("FVector") || InPropertyCppName.Equals("FRotator") || InPropertyCppName.Equals("FTransform"))
			return true;

		return false;
	}

	enum STATSTRACER_API						EDataSourceType
	{
		Unknown = 0,

		// simple types
		Bool,
		Int,
		Float,
		Byte,

		// complex types
		Vector,
		Rotator,
		Transform
	};

	///-------------------------------------------------------------------------------------------------
	/// Fn:	static FLinearColor GetNextLabelColor(float InHueStartPosition, float* OutHuePosition)
	///
	/// Summary: Used to generate distinguashible colors when called multiple times.
	///
	/// Author:	Tobias Stein
	///
	/// Date:	31/03/2018
	///
	/// Parameters:
	/// InHueStartPosition - 	The in hue start position.
	/// OutHuePosition - 	 	[in,out] If non-null, the out hue position.
	///
	/// Returns:	The next label color.
	///-------------------------------------------------------------------------------------------------

	static FLinearColor GetNextLabelColor(float InHueStartPosition, float* OutHuePosition)
	{
		static const float STEP_SIZE { 33.33f };

		*OutHuePosition = FMath::Fmod(InHueStartPosition + (STEP_SIZE + (FMath::FRand() * 16.67f)), 360.0f);

		return FLinearColor::FGetHSV((uint8)FMath::Lerp(0.0f, 255.0f, *OutHuePosition / 360.0f), 0, 255);
	}





	///-------------------------------------------------------------------------------------------------
	/// Declaration of various smart pointer types.
	///-------------------------------------------------------------------------------------------------

	using TTracerSessionHandle					= TSharedPtr<FTracerSession>;
	using TWeakTracerSessionHandle				= TWeakPtr<FTracerSession>;


	using TDataSourceHandle						= TSharedPtr<IDataSource>;


	using TDataSourceArray						= TArray<TDataSourceHandle>;


	using TDataGroupMap							= TMap<FString, TDataSourceArray>;

	using TTracerDataRepositoryHandle			= TSharedPtr<FTracerDataRepository>;
	using TWeakTracerDataRepositoryHandle		= TWeakPtr<FTracerDataRepository>;

	using TTracerDataRepositoryMap				= TMap<uint32, TTracerDataRepositoryHandle>;
	using TTracerDataRepositoryMapHandle		= TSharedPtr<TTracerDataRepositoryMap>;
	using TWeakTracerDataRepositoryMapHandle	= TWeakPtr<TTracerDataRepositoryMap>;

	///-------------------------------------------------------------------------------------------------
	/// Struct:	ZeroSample
	///
	/// Summary:	Utility struct which tells the desired zero value for a given T type.
	///
	/// Author:	Tobias Stein
	///
	/// Date:	1/04/2018
	///-------------------------------------------------------------------------------------------------

	template<class T>
	struct ZeroSample { static const T Value; };

	template<>
	struct ZeroSample<bool> { static const bool Value; };

	template<>
	struct ZeroSample<int32> { static const int32 Value; };

	template<>
	struct ZeroSample<float> { static const float Value; };

	template<>
	struct ZeroSample<uint8> { static const uint8 Value; };

	template<>
	struct ZeroSample<FVector> { static const FVector Value; };

	template<>
	struct ZeroSample<FRotator> { static const FRotator Value; };

	template<>
	struct ZeroSample<FTransform> { static const FTransform Value; };

	///-------------------------------------------------------------------------------------------------
	/// Class:	STATSTRACER_API
	///
	/// Summary:	Base sample class containing a frame and time when sampled.
	///
	/// Author:	Tobias Stein
	///
	/// Date:	1/04/2018
	///-------------------------------------------------------------------------------------------------

	class STATSTRACER_API IDataSample
	{
	public:
		
		uint64							Frame;
		float							ElapsedTime; // in seconds

										IDataSample();
		virtual							~IDataSample();
	};

	///-------------------------------------------------------------------------------------------------
	/// Class:	STATSTRACER_API
	///
	/// Summary:	A typed data sample holding data.
	///
	/// Author:	Tobias Stein
	///
	/// Date:	1/04/2018
	///
	/// Typeparams:
	/// T - 	Generic type parameter.
	///-------------------------------------------------------------------------------------------------

	template<class T>
	class STATSTRACER_API FDataSample : public IDataSample
	{
	public:

		T								Value;		

		FDataSample() :
			Value(ZeroSample<T>::Value)
		{}

		virtual ~FDataSample()
		{}
	};

	

	///-------------------------------------------------------------------------------------------------
	/// Class:	IDataSource
	///
	/// Summary:	A data source.
	///
	/// Author:	Tobias Stein
	///
	/// Date:	8/03/2018
	///-------------------------------------------------------------------------------------------------

	class STATSTRACER_API IDataSource
	{
	private:

		const FString					m_Name;
		const FString					m_Group;
		const FText						m_Description;

		// color is changable through STracerDataChart widget
		FColor							m_Color;

		const bool						m_StreamToCsv;

	public:

										IDataSource(const FString& name, const FString& group, const FString& description, const FColor& color, const bool streamToCsv = true);

		virtual							~IDataSource();

		virtual void					SampleData(uint64 frame, float ElapsedTime, CSVStream* stream = nullptr) = 0;
		virtual inline uint32			GetSampleWindowSize() const = 0;
		virtual inline uint32			GetSampleCount() const = 0;
		virtual inline void				Clear() = 0;

		virtual inline const void*		GetRawDataPtr() const = 0;
		virtual inline uint64			GetFrameNumber(uint32 index) const = 0;
		virtual inline float			GetElapsedTime(uint32 index) const = 0;

		virtual inline uint64			GetDataSourcePhysicalMemorySize() = 0;

		virtual inline EDataSourceType	GetDataSourceType() const = 0;

		inline const FString&			GetName() const { return this->m_Name; }
		inline const FText&				GetDescription() const { return this->m_Description; }
		inline const FString&			GetGroup() const { return this->m_Group; }

		inline const bool				ShouldStreamtoCsv() const { return this->m_StreamToCsv; }

		inline FColor&					GetColor() { return this->m_Color; }

		template<class T>
		inline T*						GetAs() { return static_cast<T*>(this); }
	};

	///-------------------------------------------------------------------------------------------------
	/// Class:	FDataSource
	///
	/// Summary:	A data source.
	///
	/// Author:	Tobias Stein
	///
	/// Date:	8/03/2018
	///
	/// Typeparams:
	/// T - 	Generic type parameter.
	///-------------------------------------------------------------------------------------------------

	template<class T>
	class STATSTRACER_API FDataSource : public IDataSource
	{

	private:		

		const T*						m_Source;

		TCircularBuffer<FDataSample<T>>	m_Samples;
		const uint32					m_SampleWindowSize;

		uint32							m_BufferIndex;
		uint32							m_SampleCount;

	protected:

		FDataSource(
			const T* source,
			const FString& name,
			const FString& group,
			const FString& description,
			const FColor& color,
			const bool streamToCsv) :
			IDataSource(name, group, description, color, streamToCsv),
			m_Source(source),
			m_Samples(UStatsTracerEditorSettings::GetInstance()->SampleWindowSize),
			m_SampleWindowSize(UStatsTracerEditorSettings::GetInstance()->SampleWindowSize),
			m_BufferIndex(0),
			m_SampleCount(0)
		{}

	public:

		virtual	~FDataSource()
		{
			this->m_Source = nullptr;
		}

		virtual inline EDataSourceType	GetDataSourceType() const = 0;

		virtual void SampleData(uint64 frame, float ElapsedTime, CSVStream* stream = nullptr) override
		{
			//SCOPE_CYCLE_COUNTER(STAT_SampleDatasource);

			// check if source is still valid
			if (this->m_Source == nullptr)
			{
				if (this->ShouldStreamtoCsv())
					(*stream) << "";

				return;
			}

			// update buffer
			{
				this->m_Samples[this->m_BufferIndex].Value = *this->m_Source;
				this->m_Samples[this->m_BufferIndex].Frame = frame;
				this->m_Samples[this->m_BufferIndex].ElapsedTime = ElapsedTime;
			}

			// strema value to csv file
			if (this->ShouldStreamtoCsv() == true && stream != nullptr)
			{
				(*stream) << (*this->m_Source);
			}

			// update buffer index
			this->m_BufferIndex = this->m_Samples.GetNextIndex(this->m_BufferIndex);

			// update sample count
			this->m_SampleCount = FMath::Min<uint32>(this->m_SampleCount + 1, this->m_SampleWindowSize);
		}

		const FDataSample<T>& operator[](uint32 i) const
		{
			return this->m_Samples[((this->m_SampleCount < this->m_SampleWindowSize - 1) ? 0 : this->m_BufferIndex) + i];
		}

		virtual inline const void*		GetRawDataPtr() const override { return m_Source; }

		virtual inline uint32			GetSampleCount() const override { return this->m_SampleCount; }

		virtual inline uint64			GetFrameNumber(uint32 index) const override { return this->m_Samples[((this->m_SampleCount < this->m_SampleWindowSize - 1) ? 0 : this->m_BufferIndex) + index].Frame; }
		virtual inline float			GetElapsedTime(uint32 index) const override { return this->m_Samples[((this->m_SampleCount < this->m_SampleWindowSize - 1) ? 0 : this->m_BufferIndex) + index].ElapsedTime; }

		virtual inline uint32			GetSampleWindowSize() const override { return this->m_SampleWindowSize; }

		virtual inline void				Clear() override { this->m_BufferIndex = 0; this->m_SampleCount = 0; }

		virtual inline uint64			GetDataSourcePhysicalMemorySize() override { return sizeof(T) * this->m_SampleWindowSize; }
	};

	///-------------------------------------------------------------------------------------------------
	/// Concrete DataSource classes.
	///-------------------------------------------------------------------------------------------------

	class STATSTRACER_API FBoolDataSource : public FDataSource<bool>
	{ 
	public: 
		
		FBoolDataSource(const bool* source, const FString& name, const FString& group = "", const FString& description = "", const FColor& color = FColor(0, 0, 0, 0), const bool streamToCsv = true) :
			FDataSource(source, name, group, description, color, streamToCsv)
		{}
		
		virtual ~FBoolDataSource()
		{}

		virtual inline EDataSourceType GetDataSourceType() const override { return EDataSourceType::Bool; }
	};

	class STATSTRACER_API FIntDataSource : public FDataSource<int32>
	{ 
	public:

		FIntDataSource(const int32* source, const FString& name, const FString& group = "", const FString& description = "", const FColor& color = FColor(0, 0, 0, 0), const bool streamToCsv = true) :
			FDataSource(source, name, group, description, color, streamToCsv)
		{}

		virtual ~FIntDataSource()
		{}

		virtual inline EDataSourceType GetDataSourceType() const override { return EDataSourceType::Int; }
	};

	class STATSTRACER_API FFloatDataSource : public FDataSource<float>
	{ 
	public: 

		FFloatDataSource(const float* source, const FString& name, const FString& group = "", const FString& description = "", const FColor& color = FColor(0, 0, 0, 0), const bool streamToCsv = true) :
			FDataSource(source, name, group, description, color, streamToCsv)
		{}

		virtual ~FFloatDataSource()
		{}

		virtual inline EDataSourceType GetDataSourceType() const override { return EDataSourceType::Float; }
	};

	class STATSTRACER_API FByteDataSource : public FDataSource<uint8>
	{ 
	public: 

		FByteDataSource(const uint8* source, const FString& name, const FString& group = "", const FString& description = "", const FColor& color = FColor(0, 0, 0, 0), const bool streamToCsv = true) :
			FDataSource(source, name, group, description, color, streamToCsv)
		{}

		virtual ~FByteDataSource()
		{}

		virtual inline EDataSourceType GetDataSourceType() const override { return EDataSourceType::Byte; }
	};
	
	class STATSTRACER_API FVectorDataSource : public FDataSource<FVector> 
	{ 
	public: 
		
		FVectorDataSource(const FVector* source, const FString& name, const FString& group = "", const FString& description = "", const FColor& color = FColor(0, 0, 0, 0), const bool streamToCsv = true) :
			FDataSource(source, name, group, description, color, streamToCsv)
		{}

		virtual ~FVectorDataSource()
		{}

		virtual inline EDataSourceType GetDataSourceType() const override { return EDataSourceType::Vector; }
	};

	class STATSTRACER_API FRotatorDataSource : public FDataSource<FRotator>
	{ 
	public: 
		
		FRotatorDataSource(const FRotator* source, const FString& name, const FString& group = "", const FString& description = "", const FColor& color = FColor(0, 0, 0, 0), const bool streamToCsv = true) :
			FDataSource(source, name, group, description, color, streamToCsv)
		{}

		virtual ~FRotatorDataSource()
		{}

		virtual inline EDataSourceType GetDataSourceType() const override { return EDataSourceType::Rotator; }
	};
	
	class STATSTRACER_API FTransformDataSource : public FDataSource<FTransform>
	{ 
	public: 
		
		FTransformDataSource(const FTransform* source, const FString& name, const FString& group = "", const FString& description = "", const FColor& color = FColor(0, 0, 0, 0), const bool streamToCsv = true) :
			FDataSource(source, name, group, description, color, streamToCsv)
		{}

		virtual ~FTransformDataSource()
		{}

		virtual inline EDataSourceType GetDataSourceType() const override { return EDataSourceType::Transform; }
	};

	///-------------------------------------------------------------------------------------------------
	/// Class:	FTracerDataRepository
	///
	/// Summary:	A tracer data repository.
	///
	/// Author:	Tobias Stein
	///
	/// Date:	8/03/2018
	///-------------------------------------------------------------------------------------------------

	class STATSTRACER_API FTracerDataRepository
	{
		friend class FTracerDataRepositoryManager;

	public:

		enum State
		{
			INITIALIZED = 0,
			TRACING,
			PAUSED,
			STOPPED,
			COMPLETE
		};

	private:

		const uint32							m_RepositoryId;

		const FString							m_RepositoryName;
		const FText								m_RepositoryDescription;

		AActor*									m_TracedActor;
		const FName								m_TracedActorFName;

		TWeakPtr<FTracerSession>				m_Session;
		TDataGroupMap							m_DataGroups;
		
		State									m_State;

		const bool								m_StreamToCsv;
		const bool								m_AutoStartOnBeginPlay;

		CSVStream*								m_CSVStream;

		float									m_NextColorStartHue;

	public:

												FTracerDataRepository(const FString& repositoryName, const FString& repositoryDescription, AActor* tracedActor, TSharedPtr<FTracerSession> session, const bool streamToCsv = false, const bool autostart = true);
												~FTracerDataRepository();

		void									AddDataSource(IDataSource* dataSourcePtr);

		void									Update(uint64 frame, float DeltaTime, bool forceUpdate = false);

		void									Start(const FDateTime& sessionStart);
		void									Pause();
		void									Resume();
		void									Stop();
		void									Complete();

		FColor									GetNextDefaultDataSourceColor();

		uint64									GetRepositoryPhysicalMemorySize();

		inline const uint32						GetRepositoryId() const { return this->m_RepositoryId; }
		inline const FString&					GetRepositoryName() const { return this->m_RepositoryName; }
		inline const TDataGroupMap&				GetRepositoryData() const { return this->m_DataGroups; }
		inline const FText&						GetRepositoryDescription() const { return this->m_RepositoryDescription; }
		
		bool									HasTracedActor() const;

		void									SelectTracedActor();


		inline TWeakPtr<FTracerSession>			GetSession() const { return this->m_Session; }

		

		inline const State						GetRepositoryState() const { return this->m_State; }
		inline bool								IsActive() const { return this->m_State < STOPPED; }

		inline bool								ShouldStreamToCsv() const { return this->m_StreamToCsv; }
		inline bool								ShouldAutostartOnBeginPlay() const { return this->m_AutoStartOnBeginPlay; }
	};

	///-------------------------------------------------------------------------------------------------
	/// Class:	FTracerSession
	///
	/// Summary:	A tracer session.
	///
	/// Author:	Tobias Stein
	///
	/// Date:	8/03/2018
	///-------------------------------------------------------------------------------------------------

	class STATSTRACER_API FTracerSession : public TSharedFromThis<FTracerSession>
	{
	public:

		enum State
		{
			TRACING = 0,
			PAUSED,
			STOPPED,
			COMPLETE
		};

	private:

		static uint32								m_NextSessionId;

		uint32										m_Id;
		FString										m_AlliasName;

		int32										m_SampleFrequency;

		State										m_State;
		FDateTime									m_SessionStart;
		FDateTime									m_SessionEnd;

		TTracerDataRepositoryMapHandle				m_RepositoryMap;

		uint64										m_FrameCounter;
		float										m_ElapsedTime;

	public:

													FTracerSession();
													~FTracerSession();

		TTracerDataRepositoryHandle					CreateTracerRepository(const FString& repositoryName, const FString& repositoryDescription, AActor* tracedActor, const bool streamToCsv = false, const bool autostart = true);
		void										DeleteTracerRepository(const uint32 repositoryId);
		
		void										StartSession();
		void										PauseSession();
		void										ResumeSession();
		void										UpdateSession(float DeltaTime, bool forceUpdate = false);
		void										StopSession();
		void										EndSession();


		uint64										GetSessionPhysicalMemorySize();

		inline const State							GetSessionState() const { return this->m_State; }

		inline const FDateTime&						GetSessionStart() const { return this->m_SessionStart; }
		inline const FDateTime&						GetSessionEnd() const { return this->m_SessionEnd; }

		inline bool									IsActiveSession() const { return this->m_State < STOPPED; }
		
		inline const int32							GetSessionSampleFrequency() const { return this->m_SampleFrequency; }

		inline const uint32							GetSessionId() const { return this->m_Id; }

		inline TWeakTracerDataRepositoryMapHandle	GetTracerDataRepositoryMap() const { return this->m_RepositoryMap; }

		inline void									SetAllias(const FString& allias) { this->m_AlliasName = allias; }
		inline const FString&						GetAllias() const { return this->m_AlliasName; }
	};

	///-------------------------------------------------------------------------------------------------
	/// Class:	FTracerDataRepositoryManager
	///
	/// Summary:	Manager for tracer data repositories.
	///
	/// Author:	Tobias Stein
	///
	/// Date:	8/03/2018
	///-------------------------------------------------------------------------------------------------

	class STATSTRACER_API FTracerDataRepositoryManager
	{
		using Sessions = TArray<TTracerSessionHandle>;

	private:

												FTracerDataRepositoryManager(const FTracerDataRepositoryManager&);
												FTracerDataRepositoryManager& operator=(const FTracerDataRepositoryManager&);

		Sessions								m_Sessions;

		uint64									m_TotalPhysicalMemorySize;
		float									m_RatioTotalLimit;
		uint64									m_AvailalbeMemory;

	public:

												FTracerDataRepositoryManager();
												~FTracerDataRepositoryManager();

		void									InitializeNewTracerSession();

		void									StartActiveTracerSession();
		void									PauseActiveTracerSession();
		void									ResumeActiveTracerSession();
		void									UpdateActiveTracerSession(float DeltaTime, bool forceUpdate = false);
		void									StopActiveTracerSession();
		void									EndActiveTracerSession();

		TTracerDataRepositoryHandle				CreateTracerRepository(const FString& repositoryName, const FString& repositoryDescription, AActor* tracedActor, const bool streamToCsv = false, const bool autostart = true);

		void									RemoveSession(const uint32 sessionId);

		void									ConsumePhysicalMemory(uint64 memorySize);

		inline void								ClearAllSessions() { this->m_Sessions.Empty(); UpdatePhysicalMemoryUsage(); }

		inline int32							GetSessionCount() const { return this->m_Sessions.Num(); }

		inline TWeakTracerSessionHandle			GetOldestSession() const { return this->m_Sessions[0]; }
		inline TWeakTracerSessionHandle			GetLatestSession() const { return this->m_Sessions.Last(); }

		TArray<TWeakTracerSessionHandle>		GetSessionArray();


		inline uint64							GetTotalPhysicalMemory() const { return this->m_TotalPhysicalMemorySize; }
		inline uint64							GetAvailablePhysicalMemory() const { return this->m_AvailalbeMemory; }
		
		inline float							GetTotalLimitRatio() const { return this->m_RatioTotalLimit; }

		///-------------------------------------------------------------------------------------------------
		/// Fn:	void FTracerDataRepositoryManager::UpdatePhysicalMemoryUsage();
		///
		/// Summary:	Updates the physical memory usage states.
		///
		/// Author:	Tobias Stein
		///
		/// Date:	4/03/2018
		///-------------------------------------------------------------------------------------------------

		void									UpdatePhysicalMemoryUsage();

	private:

		void									RegisterEditorDelegates();
		void									UnregisterEditorDelegates();

		void									HandleEditorPreBeginPIE(bool bIsSimulating);
		void									HandleEditorPostStartedPIE(bool bIsSimulating);
		void									HandleEditorEndPIE(bool bIsSimulating);
		void									HandleEditorPausePIE(bool bIsSimulating);
		void									HandleEditorResumePIE(bool bIsSimulating);
		void									HandleEditorSingleStepPIE(bool bIsSimulating);
	};

	///-------------------------------------------------------------------------------------------------
	/// Property:	extern STATSTRACER_API FTracerDataRepositoryManager* TDRM
	///
	/// Summary:	Gets the global instance of the TracerDataRepositoryManager.
	///
	/// Returns:	The global instance of the TracerDataRepositoryManager.
	///-------------------------------------------------------------------------------------------------

	extern STATSTRACER_API	FTracerDataRepositoryManager* TDRM;

} // namespace StatsTracer