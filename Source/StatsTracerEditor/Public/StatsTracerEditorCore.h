///-------------------------------------------------------------------------------------------------
///  Copyright (C) Tobias Stein - All Rights Reserved.
///
/// Unauthorized copying of this file, via any medium is strictly prohibited
/// Proprietary and confidential.
///
/// Written by Tobias Stein <tobiasstein40@gmail.com>, 2017 - 2018.
///
/// File:	StatsTracerEditor\Public\StatsTracerEditorCore.h
///
/// Summary:	Declares the statistics tracer editor core class.
///-------------------------------------------------------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "StatsTracerCore.h"

namespace StatsTracer {

	///-------------------------------------------------------------------------------------------------
	/// Concrete DataSource view classes.
	///-------------------------------------------------------------------------------------------------
	
	class STATSTRACEREDITOR_API IDataSourceView
	{
	public:

									IDataSourceView();
		virtual						~IDataSourceView();

		virtual void				Update() = 0;
	};

	template<class T>
	class STATSTRACEREDITOR_API FDataSourceView : public IDataSourceView
	{
	protected:

		const FDataSource<T>*		m_DataSource;

	public:

		FDataSourceView(const FDataSource<T>* dataSource) :
			m_DataSource(dataSource)
		{}

		virtual ~FDataSourceView()
		{}

		virtual void Update() = 0;
	};

	///-------------------------------------------------------------------------------------------------
	/// Class:	FSimpleDataSourceView
	///
	/// Summary:	General view for data sources with simple types, e.g. int32, bool, float ...
	///
	/// Author:	Tobias Stein
	///
	/// Date:	3/01/2018
	///
	/// Typeparams:
	/// T - 	Generic type parameter.
	///-------------------------------------------------------------------------------------------------
	
	template<class T>
	class STATSTRACEREDITOR_API FSimpleDataSourceView : public FDataSourceView<T>
	{
	private:

		T							m_MinValue;
		T							m_MaxValue;
		T							m_AvgValue;

	public:

		FSimpleDataSourceView(const FDataSource<T>* dataSource) :
			FDataSourceView<T>(dataSource)
		{
			this->Update();
		}

		virtual	~FSimpleDataSourceView()
		{}

		virtual void Update() override
		{
			if (this->m_DataSource == nullptr || this->m_DataSource->GetSampleCount() == 0)
			{
				this->m_MinValue = 0;
				this->m_MaxValue = 0;
				this->m_AvgValue = 0;
				return;
			}

			this->m_MinValue = TNumericLimits<T>::Max();
			this->m_MaxValue = TNumericLimits<T>::Min();
			T sum = 0;

			for (uint32 i = 0; i < this->m_DataSource->GetSampleCount(); ++i)
			{
				if (FMath::IsWithinInclusive<T>((*this->m_DataSource)[i].Value, TNumericLimits<T>::Min(), TNumericLimits<T>::Max()) == false)
					continue;

				// update min
				if ((*this->m_DataSource)[i].Value < this->m_MinValue)
					this->m_MinValue = (*this->m_DataSource)[i].Value;

				// update max
				if ((*this->m_DataSource)[i].Value > this->m_MaxValue)
					this->m_MaxValue = (*this->m_DataSource)[i].Value;

				// acc sum
				sum += (*this->m_DataSource)[i].Value;
			}

			this->m_AvgValue = (T)(sum / FMath::Max<int32>(1, this->m_DataSource->GetSampleCount()));
		}
		
		inline T					GetMinValue() const { return this->m_MinValue; }
		inline T					GetMaxValue() const { return this->m_MaxValue; }
		inline T					GetAvgValue() const { return this->m_AvgValue; }
		
		const T&					operator[](int32 i) const { return (*this->m_DataSource)[i].Value; }
	};

	///-------------------------------------------------------------------------------------------------
	/// Class:	FSimpleDataSourceView<bool>
	///
	/// Summary:	Template specialization for boolean data source view. 
	///
	/// Author:	Tobias Stein
	///
	/// Date:	3/01/2018
	///-------------------------------------------------------------------------------------------------

	template<>
	class STATSTRACEREDITOR_API FSimpleDataSourceView<bool> : public FDataSourceView<bool>
	{
	private:

		float	m_AvgValue;

	public:

		FSimpleDataSourceView(const FBoolDataSource* dataSource) :
			FDataSourceView<bool>(dataSource)
		{
			this->Update();
		}

		virtual ~FSimpleDataSourceView()
		{}


		virtual void Update() override
		{
			int32 sum = 0;

			if (this->m_DataSource == nullptr)
				return;

			for (uint32 i = 0; i < this->m_DataSource->GetSampleCount(); ++i)
			{
				// acc sum
				sum += ((*this->m_DataSource)[i].Value == true ? 1 : 0);
			}

			this->m_AvgValue = (float)(sum / FMath::Min<int32>(1, this->m_DataSource->GetSampleCount()));
		}

		inline float GetAvgTrueStateValue() const { return this->m_AvgValue; }

		const bool	operator[](int32 i) const { return (*this->m_DataSource)[i].Value; }
	};
	
	template<>
	class STATSTRACEREDITOR_API FSimpleDataSourceView<FVector> : public FDataSourceView<FVector>
	{
		enum Element { X = 0, Y = 1, Z = 2, MAX_ELEMENTS };

	private:

		float	m_MinValue[MAX_ELEMENTS];
		float	m_MaxValue[MAX_ELEMENTS];

	public:

		FSimpleDataSourceView(const FVectorDataSource* dataSource) :
			FDataSourceView<FVector>(dataSource)
		{
			this->Update();
		}

		virtual ~FSimpleDataSourceView()
		{}


		virtual void Update() override
		{
			if (this->m_DataSource == nullptr || this->m_DataSource->GetSampleCount() == 0)
			{
				this->m_MinValue[X] = this->m_MinValue[Y] = this->m_MinValue[Z] = 0.0f;
				this->m_MaxValue[X] = this->m_MaxValue[Y] = this->m_MaxValue[Z] = 0.0f;
				return;
			}

			this->m_MinValue[X] = this->m_MinValue[Y] = this->m_MinValue[Z] = TNumericLimits<float>::Max();
			this->m_MaxValue[X] = this->m_MaxValue[Y] = this->m_MaxValue[Z] = TNumericLimits<float>::Min();
			
			for (uint32 i = 0; i < this->m_DataSource->GetSampleCount(); ++i)
			{
				const float x = (*this->m_DataSource)[i].Value.X;
				const float y = (*this->m_DataSource)[i].Value.Y;
				const float z = (*this->m_DataSource)[i].Value.Z;

				if (FMath::IsNaN(x) == false && FMath::IsFinite(x))
				{
					if (x < this->m_MinValue[X])
						this->m_MinValue[X] = x;

					if (x > this->m_MaxValue[X])
						this->m_MaxValue[X] = x;
				}

				if (FMath::IsNaN(y) == false && FMath::IsFinite(y))
				{
					if (y < this->m_MinValue[Y])
						this->m_MinValue[Y] = y;

					if (y > this->m_MaxValue[Y])
						this->m_MaxValue[Y] = y;
				}

				if (FMath::IsNaN(z) == false && FMath::IsFinite(z))
				{
					if (z < this->m_MinValue[Z])
						this->m_MinValue[Z] = z;

					if (z > this->m_MaxValue[Z])
						this->m_MaxValue[Z] = z;
				}
			}
		}


		inline const float GetMinValueX() const { return this->m_MinValue[X]; }
		inline const float GetMaxValueX() const { return this->m_MaxValue[X]; }
		inline const float GetMinValueY() const { return this->m_MinValue[Y]; }
		inline const float GetMaxValueY() const { return this->m_MaxValue[Y]; }
		inline const float GetMinValueZ() const { return this->m_MinValue[Z]; }
		inline const float GetMaxValueZ() const { return this->m_MaxValue[Z]; }

		inline const FVector operator[](int32 i) const { return (*this->m_DataSource)[i].Value; }
	};

	template<>
	class STATSTRACEREDITOR_API FSimpleDataSourceView<FRotator> : public FDataSourceView<FRotator>
	{
		enum Element { X = 0, Y = 1, Z = 2, MAX_ELEMENTS };

	private:

		float	m_MinValue[MAX_ELEMENTS];
		float	m_MaxValue[MAX_ELEMENTS];

	public:

		FSimpleDataSourceView(const FRotatorDataSource* dataSource) :
			FDataSourceView<FRotator>(dataSource)
		{
			this->Update();
		}

		virtual ~FSimpleDataSourceView()
		{}


		virtual void Update() override
		{
			if (this->m_DataSource == nullptr || this->m_DataSource->GetSampleCount() == 0)
			{
				this->m_MinValue[X] = this->m_MinValue[Y] = this->m_MinValue[Z] = 0.0f;
				this->m_MaxValue[X] = this->m_MaxValue[Y] = this->m_MaxValue[Z] = 0.0f;
				return;
			}

			this->m_MinValue[X] = this->m_MinValue[Y] = this->m_MinValue[Z] = TNumericLimits<float>::Max();
			this->m_MaxValue[X] = this->m_MaxValue[Y] = this->m_MaxValue[Z] = TNumericLimits<float>::Min();

			for (uint32 i = 0; i < this->m_DataSource->GetSampleCount(); ++i)
			{
				const float x = (*this->m_DataSource)[i].Value.Roll;
				const float y = (*this->m_DataSource)[i].Value.Pitch;
				const float z = (*this->m_DataSource)[i].Value.Yaw;

				if (FMath::IsNaN(x) == false && FMath::IsFinite(x))
				{
					if (x < this->m_MinValue[X])
						this->m_MinValue[X] = x;

					if (x > this->m_MaxValue[X])
						this->m_MaxValue[X] = x;
				}

				if (FMath::IsNaN(y) == false && FMath::IsFinite(y))
				{
					if (y < this->m_MinValue[Y])
						this->m_MinValue[Y] = y;

					if (y > this->m_MaxValue[Y])
						this->m_MaxValue[Y] = y;
				}

				if (FMath::IsNaN(z) == false && FMath::IsFinite(z))
				{
					if (z < this->m_MinValue[Z])
						this->m_MinValue[Z] = z;

					if (z > this->m_MaxValue[Z])
						this->m_MaxValue[Z] = z;
				}
			}
		}

		inline const float GetMinValueX() const { return this->m_MinValue[X]; }
		inline const float GetMaxValueX() const { return this->m_MaxValue[X]; }
		inline const float GetMinValueY() const { return this->m_MinValue[Y]; }
		inline const float GetMaxValueY() const { return this->m_MaxValue[Y]; }
		inline const float GetMinValueZ() const { return this->m_MinValue[Z]; }
		inline const float GetMaxValueZ() const { return this->m_MaxValue[Z]; }

		inline const FRotator operator[](int32 i) const { return (*this->m_DataSource)[i].Value; }
	};

	template<>
	class STATSTRACEREDITOR_API FSimpleDataSourceView<FTransform> : public FDataSourceView<FTransform>
	{
		enum Element 
		{ 
			PX = 0, PY = 1, PZ = 2, 
			RX = 3, RY = 4, RZ = 5,
			SX = 6, SY = 7, SZ = 8,
			MAX_ELEMENTS 
		};

	private:

		float	m_MinValue[MAX_ELEMENTS];
		float	m_MaxValue[MAX_ELEMENTS];

	public:

		FSimpleDataSourceView(const FTransformDataSource* dataSource) :
			FDataSourceView<FTransform>(dataSource)
		{
			this->Update();
		}

		virtual ~FSimpleDataSourceView()
		{}


		virtual void Update() override
		{
			if (this->m_DataSource == nullptr || this->m_DataSource->GetSampleCount() == 0)
			{
				this->m_MinValue[PX] = this->m_MinValue[PY] =this->m_MinValue[PZ] = 
				this->m_MinValue[RX] = this->m_MinValue[RY] =this->m_MinValue[RZ] = 
				this->m_MinValue[SX] = this->m_MinValue[SY] =this->m_MinValue[SZ] = 0.0f;

				this->m_MaxValue[PX] = this->m_MaxValue[PY] = this->m_MaxValue[PZ] =
				this->m_MaxValue[RX] = this->m_MaxValue[RY] = this->m_MaxValue[RZ] =
				this->m_MaxValue[SX] = this->m_MaxValue[SY] = this->m_MaxValue[SZ] = 0.0f;
				return;
			}

			this->m_MinValue[PX] = this->m_MinValue[PY] = this->m_MinValue[PZ] =
			this->m_MinValue[RX] = this->m_MinValue[RY] = this->m_MinValue[RZ] =
			this->m_MinValue[SX] = this->m_MinValue[SY] = this->m_MinValue[SZ] = TNumericLimits<float>::Max();

			this->m_MaxValue[PX] = this->m_MaxValue[PY] = this->m_MaxValue[PZ] =
			this->m_MaxValue[RX] = this->m_MaxValue[RY] = this->m_MaxValue[RZ] =
			this->m_MaxValue[SX] = this->m_MaxValue[SY] = this->m_MaxValue[SZ] = TNumericLimits<float>::Min();

			for (uint32 i = 0; i < this->m_DataSource->GetSampleCount(); ++i)
			{
				const FVector	P = (*this->m_DataSource)[i].Value.GetLocation();
				const FRotator	R = (*this->m_DataSource)[i].Value.GetRotation().Rotator();
				const FVector	S = (*this->m_DataSource)[i].Value.GetScale3D();

				// Position
				if (FMath::IsNaN(P.X) == false && FMath::IsFinite(P.X))
				{
					if (P.X < this->m_MinValue[PX])
						this->m_MinValue[PX] = P.X;

					if (P.X > this->m_MaxValue[PX])
						this->m_MaxValue[PX] = P.X;
				}

				if (FMath::IsNaN(P.Y) == false && FMath::IsFinite(P.Y))
				{
					if (P.Y < this->m_MinValue[PY])
						this->m_MinValue[PY] = P.Y;

					if (P.Y > this->m_MaxValue[PY])
						this->m_MaxValue[PY] = P.Y;
				}

				if (FMath::IsNaN(P.Z) == false && FMath::IsFinite(P.Z))
				{
					if (P.Z < this->m_MinValue[PZ])
						this->m_MinValue[PZ] = P.Z;

					if (P.Z > this->m_MaxValue[PZ])
						this->m_MaxValue[PZ] = P.Z;
				}

				// Rotation
				if (FMath::IsNaN(R.Roll) == false && FMath::IsFinite(R.Roll))
				{
					if (R.Roll < this->m_MinValue[RX])
						this->m_MinValue[RX] = R.Roll;

					if (R.Roll > this->m_MaxValue[RX])
						this->m_MaxValue[RX] = R.Roll;
				}

				if (FMath::IsNaN(R.Pitch) == false && FMath::IsFinite(R.Pitch))
				{
					if (R.Pitch < this->m_MinValue[RY])
						this->m_MinValue[RY] = R.Pitch;

					if (R.Pitch > this->m_MaxValue[RY])
						this->m_MaxValue[RY] = R.Pitch;
				}

				if (FMath::IsNaN(R.Yaw) == false && FMath::IsFinite(R.Yaw))
				{
					if (R.Yaw < this->m_MinValue[RZ])
						this->m_MinValue[RZ] = R.Yaw;

					if (R.Yaw > this->m_MaxValue[RZ])
						this->m_MaxValue[RZ] = R.Yaw;
				}

				// Scale
				// Position
				if (FMath::IsNaN(S.X) == false && FMath::IsFinite(S.X))
				{
					if (S.X < this->m_MinValue[SX])
						this->m_MinValue[SX] = S.X;

					if (S.X > this->m_MaxValue[SX])
						this->m_MaxValue[SX] = S.X;
				}

				if (FMath::IsNaN(S.Y) == false && FMath::IsFinite(S.Y))
				{
					if (S.Y < this->m_MinValue[SY])
						this->m_MinValue[SY] = S.Y;

					if (S.Y > this->m_MaxValue[SY])
						this->m_MaxValue[SY] = S.Y;
				}

				if (FMath::IsNaN(S.Z) == false && FMath::IsFinite(S.Z))
				{
					if (S.Z < this->m_MinValue[SZ])
						this->m_MinValue[SZ] = S.Z;

					if (S.Z > this->m_MaxValue[SZ])
						this->m_MaxValue[SZ] = S.Z;
				}
			}
		}

		inline const float GetMinValuePX() const { return this->m_MinValue[PX]; }
		inline const float GetMaxValuePX() const { return this->m_MaxValue[PX]; }
		inline const float GetMinValuePY() const { return this->m_MinValue[PY]; }
		inline const float GetMaxValuePY() const { return this->m_MaxValue[PY]; }
		inline const float GetMinValuePZ() const { return this->m_MinValue[PZ]; }
		inline const float GetMaxValuePZ() const { return this->m_MaxValue[PZ]; }

		inline const float GetMinValueRX() const { return this->m_MinValue[RX]; }
		inline const float GetMaxValueRX() const { return this->m_MaxValue[RX]; }
		inline const float GetMinValueRY() const { return this->m_MinValue[RY]; }
		inline const float GetMaxValueRY() const { return this->m_MaxValue[RY]; }
		inline const float GetMinValueRZ() const { return this->m_MinValue[RZ]; }
		inline const float GetMaxValueRZ() const { return this->m_MaxValue[RZ]; }

		inline const float GetMinValueSX() const { return this->m_MinValue[SX]; }
		inline const float GetMaxValueSX() const { return this->m_MaxValue[SX]; }
		inline const float GetMinValueSY() const { return this->m_MinValue[SY]; }
		inline const float GetMaxValueSY() const { return this->m_MaxValue[SY]; }
		inline const float GetMinValueSZ() const { return this->m_MinValue[SZ]; }
		inline const float GetMaxValueSZ() const { return this->m_MaxValue[SZ]; }

		inline const FTransform operator[](int32 i) const { return (*this->m_DataSource)[i].Value; }
	};

	///-------------------------------------------------------------------------------------------------
	/// Concrete DataSource view classes.
	///-------------------------------------------------------------------------------------------------

	class STATSTRACEREDITOR_API FBoolDataSourceView : public FSimpleDataSourceView<bool>
	{
	public:
    
		FBoolDataSourceView(const FBoolDataSource* InDataSource) :
			FSimpleDataSourceView<bool>(InDataSource)
		{}

		virtual ~FBoolDataSourceView()
		{}
	};

	class STATSTRACEREDITOR_API FIntDataSourceView : public FSimpleDataSourceView<int32>
	{
	public:

		FIntDataSourceView(const FIntDataSource* InDataSource) :
			FSimpleDataSourceView<int32>(InDataSource)
		{}

		virtual ~FIntDataSourceView()
		{}
	};

	class STATSTRACEREDITOR_API FFloatDataSourceView : public FSimpleDataSourceView<float>
	{
	public:

		FFloatDataSourceView(const FFloatDataSource* InDataSource) :
			FSimpleDataSourceView<float>(InDataSource)
		{}

		virtual ~FFloatDataSourceView()
		{}
	};

	class STATSTRACEREDITOR_API FByteDataSourceView : public FSimpleDataSourceView<uint8>
	{
	public:

		FByteDataSourceView(const FByteDataSource* InDataSource) :
			FSimpleDataSourceView<uint8>(InDataSource)
		{}

		virtual ~FByteDataSourceView()
		{}
	};

	class STATSTRACEREDITOR_API FVectorDataSourceView : public FSimpleDataSourceView<FVector>
	{
	public:

		FVectorDataSourceView(const FVectorDataSource* InDataSource) :
			FSimpleDataSourceView<FVector>(InDataSource)
		{}

		virtual ~FVectorDataSourceView()
		{}
	};

	class STATSTRACEREDITOR_API FRotatorDataSourceView : public FSimpleDataSourceView<FRotator>
	{
	public:

		FRotatorDataSourceView(const FRotatorDataSource* InDataSource) :
			FSimpleDataSourceView<FRotator>(InDataSource)
		{}

		virtual ~FRotatorDataSourceView()
		{}
	};

	class STATSTRACEREDITOR_API FTransformDataSourceView : public FSimpleDataSourceView<FTransform>
	{
	public:

		FTransformDataSourceView(const FTransformDataSource* InDataSource) :
			FSimpleDataSourceView<FTransform>(InDataSource)
		{}

		virtual ~FTransformDataSourceView()
		{}
	};

} // namespace StatsTracer