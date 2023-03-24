#pragma once

#include <stdint.h>
#include <float.h>
#include <cmath>
#include <algorithm>
#include <immintrin.h>

#define is16Aligned(mem) ( reinterpret_cast<uint64_t>(mem) & 15 == 0 ) 

#define _mm_castpd_si128(m128) ( *reinterpret_cast<__m128i*>(&m128) )

#define UMTX { { { 1.0f, 0.0f, 0.0f, 0.0f },\
				 { 0.0f, 1.0f, 0.0f, 0.0f },\
				 { 0.0f, 0.0f, 1.0f, 0.0f },\
				 { 0.0f, 0.0f, 0.0f, 1.0f } } }

struct alignas(16) ViewMatrix { float mtx[4][4]; };

struct m128Matrix { __m128 r[4]; };

const extern m128Matrix umtx;
extern m128Matrix vmtx;

template <typename T> inline constexpr T sign(const T val)
{
	return static_cast<T>((static_cast<T>(0) < val)) - static_cast<T>((val < static_cast<T>(0)));
}

class V4D
{
public:
	__m128 V4;

	enum CoordinateAxis : int
	{
		X = 0,
		Z = 1,
		Y = 2,
		W = 3
	};

	V4D() {}

	V4D(const V4D& v)
	{
		this->V4 = v.V4;
	}

	V4D(__m128 v)
	{
		this->V4 = v;
	}

	V4D(const float v[4])
	{
		this->V4 = v != nullptr ? _mm_loadu_ps(v) : _mm_setzero_ps();
	}

	V4D(const float f0, const float f1, const float f2, const float f3)
	{
		this->V4 = _mm_set_ps(f3, f2, f1, f0);
	}

	V4D(const float f0, const float f1, const float f2)
	{
		__m128 v = _mm_set_ps(f2, f2, f1, f0);
		this->V4 = _mm_insert_ps(v, _mm_setzero_ps(), 0b00110000);
	}

	V4D(const float f)
	{
		this->V4 = _mm_set1_ps(f);
	}

	V4D(const V4D axis, const float angle)
	{
		const float halfa = angle * 0.5f;

		__m128 v = _mm_mul_ps(axis, _mm_set1_ps(sinf(halfa)));
		this->V4 = _mm_insert_ps(v, _mm_set_ss(cosf(halfa)), 0b00110000);
	}

	V4D(const V4D v1, const V4D v2)
	{
		V4D v = v1.cross(v2);
		v[3] = v1 * v2 + sqrtf(v1.length2() * v1.length2());
		*this = v.normalize();
	}

	~V4D() {}

	operator __m128() const
	{
		return V4;
	}

	float& operator [] (const int index)
	{
		return reinterpret_cast<float*>(&this->V4)[index];
	}

	float operator [] (const int index) const
	{
		return reinterpret_cast<const float*>(&this->V4)[index];
	}

	volatile float& operator [] (const int index) volatile
	{
		return reinterpret_cast<volatile float*>(&this->V4)[index];
	}

	V4D& operator = (const V4D v)
	{
		this->V4 = v;
		return *this;
	}

	V4D& operator = (const float v[4])
	{
		this->V4 = _mm_loadu_ps(v);
		return *this;
	}

	V4D operator + (const V4D v) const
	{
		return _mm_add_ps(*this, v);
	}

	void operator += (const V4D v)
	{
		this->V4 = _mm_add_ps(*this, v);
	}

	V4D operator - (const V4D v) const
	{
		return _mm_sub_ps(*this, v);
	}

	void operator -= (const V4D v)
	{
		this->V4 = _mm_sub_ps(*this, v);
	}

	V4D operator * (const float s) const
	{
		return _mm_mul_ps(*this, _mm_set1_ps(s));
	}

	void operator *= (const float s)
	{
		this->V4 = _mm_mul_ps(*this, _mm_set1_ps(s));
	}

	V4D operator / (const float s) const
	{
		return _mm_div_ps(*this, _mm_set1_ps(s));
	}

	void operator /= (const float s)
	{
		this->V4 = _mm_div_ps(*this, _mm_set1_ps(s));
	}

	__m128 hadd() const // https://stackoverflow.com/questions/6996764/fastest-way-to-do-horizontal-sse-vector-sum-or-other-reduction#:~:text=for%20the%20optimizer.-,SSE3,-float%20hsum_ps_sse3(__m128
	{
		__m128 sumsh = _mm_movehdup_ps(this->V4);
		__m128 shsum = _mm_add_ps(this->V4, sumsh);
		sumsh = _mm_movehl_ps(sumsh, shsum);
		shsum = _mm_add_ss(shsum, sumsh);

		return shsum;
	}

	float length() const
	{
		V4D v = _mm_mul_ps(*this, *this);

		return _mm_cvtss_f32(_mm_sqrt_ss(v.hadd()));
	}

	float length2() const
	{
		V4D v = _mm_mul_ps(*this, *this);

		return _mm_cvtss_f32(v.hadd());
	}

	float inRange(const V4D v, const float range) const
	{
		V4D r = v - *this;
		r = _mm_mul_ps(r, r);
		r = r.hadd();

		if (_mm_comigt_ss(r, _mm_setzero_ps()))
		{
			if (_mm_comile_ss(_mm_mul_ps(r, _mm_rsqrt_ss(r)), _mm_set_ss(range)))
			{
				return _mm_cvtss_f32(_mm_sqrt_ss(r));
			}
			else
			{
				return 0.0f;
			}
		}
		else
		{
			return range;
		}
	}

	V4D normalize() const
	{
		__m128 l = _mm_set1_ps(this->length());
		__m128 z = _mm_setzero_ps();

		if (_mm_testc_si128(_mm_castps_si128(z), _mm_castps_si128(l)))
		{
			return z;
		}
		else
		{
			return _mm_div_ps(this->V4, l);
		}
	}

	V4D scaleTo(const float s) const
	{
		return _mm_mul_ps(this->normalize(), _mm_set1_ps(s));
	}

	template <CoordinateAxis A = Z> V4D flatten(const bool doNormalize = false) const
	{
		V4D v = _mm_insert_ps(*this, _mm_setzero_ps(), A << 4);

		if (doNormalize)
		{
			return v.normalize();
		}
		else
		{
			return v;
		}
	}

	float operator * (const V4D v) const
	{
		return _mm_cvtss_f32(V4D(_mm_mul_ps(*this, v)).hadd());
	}

	float dot3(const V4D v) const
	{
		return _mm_cvtss_f32(V4D(_mm_mul_ps(this->flatten<CoordinateAxis::W>(), v.flatten<CoordinateAxis::W>())).hadd());
	}

	V4D cross(const V4D v) const // https://geometrian.com/programming/tutorials/cross-product/index.php
	{
		__m128 tmp0 = _mm_shuffle_ps(*this, *this, _MM_SHUFFLE(3, 0, 2, 1));
		__m128 tmp1 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 1, 0, 2));
		__m128 tmp2 = _mm_mul_ps(tmp0, v);
		__m128 tmp3 = _mm_mul_ps(tmp0, tmp1);
		__m128 tmp4 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3, 0, 2, 1));

		return _mm_sub_ps(tmp3, tmp4);
	}

	V4D projectOnto(const V4D v) const
	{
		return _mm_mul_ps(v, _mm_set1_ps(v * *this));
	}

	float sign2v(const V4D v) const
	{
		const float k = -1.0f;

		if (!_mm_testz_si128(_mm_castps_si128(_mm_mul_ps(*this, v)), _mm_set1_epi32(0x80000000)))
		{
			return k;
		}
		else
		{
			return k * k;
		}
	}

	V4D qConjugate() const
	{
		return _mm_or_ps(*this, _mm_set_ps(0.0f, -0.0f, -0.0f, -0.0f));
	}

	V4D qTransform(const V4D v) const;

	V4D qMul(const V4D v) const;

	V4D qDiv(const V4D v) const;

	V4D qPow(const float pow) const;

	V4D qSlerp(const V4D v, const float t) const;
};

namespace VxD
{
	inline float* write(const V4D v, float* mem)
	{
		_mm_store_ps(mem, v);

		return mem;
	}

	inline ViewMatrix* write_vmtx(const V4D v0, const V4D v1, const V4D v2, ViewMatrix* vmtx)
	{
		_mm_store_ps(vmtx->mtx[0], v0);
		_mm_store_ps(vmtx->mtx[1], v1);
		_mm_store_ps(vmtx->mtx[2], v2);
		vmtx->mtx[3][3] = 1.0f;

		return vmtx;
	}

	inline ViewMatrix* write_vmtx(const V4D v0, const V4D v1, const V4D v2, const V4D v3, ViewMatrix* vmtx)
	{
		_mm_store_ps(vmtx->mtx[0], v0);
		_mm_store_ps(vmtx->mtx[1], v1);
		_mm_store_ps(vmtx->mtx[2], v2);
		_mm_store_ps(vmtx->mtx[3], v3);

		return vmtx;
	}

	inline ViewMatrix* write_umtx(ViewMatrix* vmtx)
	{
		_mm_store_ps(vmtx->mtx[0], umtx.r[0]);
		_mm_store_ps(vmtx->mtx[1], umtx.r[1]);
		_mm_store_ps(vmtx->mtx[2], umtx.r[2]);
		_mm_store_ps(vmtx->mtx[3], umtx.r[3]);

		return vmtx;
	}
}