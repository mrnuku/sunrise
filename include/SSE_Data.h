#ifndef SSE_DATA_H
#define SSE_DATA_H

#ifdef _M_X64
#define PREF_X64_X86( inst_x64, inst_x86 ) inst_x64
#endif
#ifdef _M_AMD64
#define PREF_AMD64_X64_X86( inst_amd64, inst_x64, inst_x86 ) inst_amd64
#define PREF_AMD64_X86( inst_amd64, inst_x86 ) inst_amd64
#elif _M_X64
#define PREF_AMD64_X64_X86( inst_amd64, inst_x64, inst_x86 ) inst_x64
#define PREF_AMD64_X86( inst_amd64, inst_x86 ) inst_x86
#else
#define PREF_X64_X86( inst_x64, inst_x86 ) inst_x86
#define PREF_AMD64_X64_X86( inst_amd64, inst_x64, inst_x86 ) inst_x86
#define PREF_AMD64_X86( inst_amd64, inst_x86 ) inst_x86
#endif

class SSE_Data {

private:

	typedef union {

		__m128						real;
		__m128i						integral;
		__m128d						realDbl;

	} memTypes;

	memTypes					d_memory;

public:

	SSE_Data &					operator =( const __m128 & other )						{ d_memory.real = other; }
	SSE_Data &					operator =( const __m128i & other )						{ d_memory.integral = other; }
	SSE_Data &					operator =( const __m128d & other )						{ d_memory.realDbl = other; }
	SSE_Data &					operator =( const int & val )							{ __stosd( ( dword * )this, *( dword * )&val, 4 ); }
	SSE_Data &					operator =( const float & val )							{ __stosd( ( dword * )this, *( dword * )&val, 4 ); }

								operator __m128 &( )									{ return d_memory.real; }
								operator __m128i &( )									{ return d_memory.integral; }
								operator __m128d &( )									{ return d_memory.realDbl; }

	byte &						operator[ ]( int index )								{ return *( &( ( byte * )this )[ index ] ); }

	__m128 &					GetRealType( )											{ return d_memory.real; }
	__m128i &					GetIntegralType( )										{ return d_memory.integral; }
	__m128d &					GetDoubleType( )										{ return d_memory.realDbl; }

	float &						GetFloatRef( int index )								{ return *( &( ( float * )this )[ index ] ); }
	int &						GetIntRef( int index )									{ return *( &( ( int * )this )[ index ] ); }
	short &						GetShortRef( int index )								{ return *( &( ( short * )this )[ index ] ); }
	byte &						GetByteRef( int index )									{ return *( &( ( byte * )this )[ index ] ); }
	double &					GetDoubleRef( int index )								{ return *( &( ( double * )this )[ index ] ); }

	void						SetFloat( const float & val )							{ __movsd( ( dword * )this, ( dword * )&val, 1 ); }

	void						WriteBytes( int offset, const byte * ptr, int size )	{ __movsb( ( ( byte * )this ) + offset, ptr, size ); }
	void						Clear( )												{ PREF_AMD64_X86( __stosq( ( qword * )this, 0, 2 ), __stosd( ( dword * )this, 0, 4 ) ); }

								SSE_Data( )												{ assert( !( ( int )this % 16 ) ); }
								SSE_Data( const __m128 & other )						{ d_memory.real = other; }
								SSE_Data( const __m128i & other )						{ d_memory.integral = other; }
								SSE_Data( const __m128d & other )						{ d_memory.realDbl = other; }

	void						LoadIntegralType( const void * data )					{ d_memory.integral = _mm_loadu_si128( ( __m128i * )data ); }
	void						StoreIntegralType( void * data )						{ _mm_storeu_si128( ( __m128i * )data, d_memory.integral ); }

	void						LoadUnaligned( const void * data )						{ d_memory.real = _mm_loadu_ps( ( float * )data ); }
	void						StoreUnaligned( void * data )							{ _mm_storeu_ps( ( float * )data, d_memory.real ); }

	/// Computes the absolute difference of the 16 unsigned 8-bit integers from a and the 16 unsigned 8-bit integers from b.
	///
	/// opcode: PSADBW
	/// s0 = abs(a0 - b0) + abs(a1 - b1) +...+ abs(a7 - b7), r4 = abs(a8 - b8) + abs(a9 - b9) +...+ abs(a15 - b15)
	/// s1 = 0x0, s2 = 0x0, s3 = 0x0, r5 = 0x0, r6 = 0x0, r7 = 0x0
	void						AbsoluteDifference( SSE_Data & a, SSE_Data & b )		{ d_memory.integral = _mm_sad_epu8( a.d_memory.integral, b.d_memory.integral ); }

	/// Conversion functions - ConvertXYZ X is the length (P= packed), Y the source type, Z the dest type

	void						ConvertPFD( )											{ d_memory.realDbl = _mm_cvtps_pd( d_memory.real ); }
	void						ConvertPDF( )											{ d_memory.real = _mm_cvtpd_ps( d_memory.realDbl ); }

	void						ConvertPID( )											{ d_memory.realDbl = _mm_cvtepi32_pd( d_memory.integral ); }
	void						ConvertPDI( )											{ d_memory.integral = _mm_cvtpd_epi32( d_memory.realDbl ); }

	void						ConvertPFI( )											{ d_memory.integral = _mm_cvtps_epi32( d_memory.real ); }
	void						ConvertPIF( )											{ d_memory.real = _mm_cvtepi32_ps( d_memory.integral ); }
	
	template<int d0,int d1,int d2,int d3>
	void						Shuffle( )												{ d_memory.real = _mm_shuffle_ps( d_memory.real, d_memory.real, _MM_SHUFFLE(d0,d1,d2,d3) ); }
	template<int d0,int d1,int d2,int d3>
	void						Shuffle( SSE_Data & a )									{ d_memory.real = _mm_shuffle_ps( d_memory.real, a.d_memory.real, _MM_SHUFFLE(d0,d1,d2,d3) ); }
	template<int d0,int d1,int d2,int d3>
	void						Shuffle( SSE_Data & a, SSE_Data & b )					{ d_memory.real = _mm_shuffle_ps( b.d_memory.real, a.d_memory.real, _MM_SHUFFLE(d0,d1,d2,d3) ); }

} ALIGN( 16 );

class Float16 {

private:

	word						d_data;

public:

								Float16( ) { }
								Float16( const float & f32 );

	operator					float( ) const;
	void						operator( )( float & f32 ) const;
};

INLINE Float16::Float16( const float & f32 ) {

	dword exp = ((*(dword*)&f32>>IEEE_FLT_MANTISSA_BITS)&0xFF);
	d_data = exp?((*(dword*)&f32>>16)&0x8000) | (((exp-(IEEE_FLT_EXPONENT_BIAS-15))<<10)&0x7C00) | ((*(dword*)&f32>>13)&0x3FF):0;
}

INLINE Float16::operator float( ) const {

	float f32;
	dword exp = ((d_data>>10)&0x1F);
	*(dword*)&f32 = exp?((d_data<<16)&0x80000000) | (((exp+(IEEE_FLT_EXPONENT_BIAS-15))<<IEEE_FLT_MANTISSA_BITS)&0x7F800000) | ((d_data&0x3FF)<<13):0;
	return f32;
}

INLINE void Float16::operator( )( float & f32 ) const {

	dword exp = ((d_data>>10)&0x1F);
	*(dword*)&f32 = exp?((d_data<<16)&0x80000000) | (((exp+(IEEE_FLT_EXPONENT_BIAS-15))<<IEEE_FLT_MANTISSA_BITS)&0x7F800000) | ((d_data&0x3FF)<<13):0;
}

#endif
