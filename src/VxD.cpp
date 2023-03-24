#include "VxD.h"

const extern m128Matrix umtx = UMTX;

V4D V4D::qTransform(const V4D v) const
{
	const float v0 = v[0];
	const float v1 = v[1];
	const float v2 = v[2];
	const float v3 = v[3];

	const float v_0 = (*this)[0];
	const float v_1 = (*this)[1];
	const float v_2 = (*this)[2];

	const float v3v3_ = v3 * v3 - 0.5f;

	const float v0v1 = v[0] * v[1];
	const float v0v2 = v[0] * v[2];
	const float v0v3 = v[0] * v[3];

	const float v1v2 = v[1] * v[2];
	const float v1v3 = v[1] * v[3];

	const float v2v3 = v[2] * v[3];

	return V4D(2.0f * (v_0 * (v0 * v0 + v3v3_) + v_1 * (v0v1 - v2v3) + v_2 * (v0v2 + v1v3)),
			   2.0f * (v_0 * (v0v1 + v2v3) + v_1 * (v1 * v1 + v3v3_) + v_2 * (v1v2 - v0v3)),
			   2.0f * (v_0 * (v0v2 - v1v3) + v_1 * (v1v2 + v0v3) + v_2 * (v2 * v2 + v3v3_)));
}

V4D V4D::qMul(const V4D v) const
{
	const float v0 = (*this)[0];
	const float v1 = (*this)[1];
	const float v2 = (*this)[2];
	const float v3 = (*this)[3];

	const float v_0 = v[0];
	const float v_1 = v[1];
	const float v_2 = v[2];
	const float v_3 = v[3];

	return V4D(v3 * v_0 + v0 * v_3 + v1 * v_2 - v2 * v_1,
			   v3 * v_1 - v0 * v_2 + v1 * v_3 + v2 * v_0,
			   v3 * v_2 + v0 * v_1 - v1 * v_0 + v2 * v_3,
			   v3 * v_3 - v0 * v_0 - v1 * v_1 - v2 * v_2);
}

V4D V4D::qDiv(const V4D v) const
{
	const float v0 = (*this)[0];
	const float v1 = (*this)[1];
	const float v2 = (*this)[2];
	const float v3 = (*this)[3];

	const float v_0 = v[0];
	const float v_1 = v[1];
	const float v_2 = v[2];
	const float v_3 = v[3];

	return V4D(v_3 * v0 - v_0 * v3 + v_1 * v2 - v_2 * v1,
			   v_3 * v1 - v_0 * v2 - v_1 * v3 + v_2 * v0,
			   v_3 * v2 + v_0 * v1 - v_1 * v0 - v_2 * v3,
			   v_3 * v3 + v_0 * v0 + v_1 * v1 + v_2 * v2);
}

V4D V4D::qPow(const float pow) const
{
	const float logFV = this->dot3(*this);
	const float logRV = sqrtf(logFV);
	const float logFR = (*this)[3];

	const float logS = logRV > FLT_EPSILON ? atan2f(logRV, logFR) / logRV : 0.0f;
	V4D logQ = *this * logS;
	logQ[3] = logf(logFV + logFR * logFR) / 2.0f;
	logQ *= pow;

	const float expFV = logQ.dot3(logQ);
	const float expRV = sqrtf(expFV);
	const float expRR = expf(logQ[3]);

	const float expS = expRV > FLT_EPSILON ? expRR * sinf(expRV) / expRV : 0.0f;
	logQ *= expS;
	logQ[3] = expRR * cosf(expRV);

	return *this;
}

V4D V4D::qSlerp(const V4D v, const float t) const // http://number-none.com/product/Understanding%20Slerp,%20Then%20Not%20Using%20It/
{
	float dot = *this * v;

	if (dot > 0.9995f)
	{
		V4D v_ = *this + (v - *this) * t;
		return v_.normalize();
	}

	dot = std::clamp(dot, -1.0f, 1.0f);
	float theta_0 = acosf(dot);
	float theta = theta_0 * t;

	V4D v_ = v - *this * dot;

	return *this * cosf(theta) + v_.normalize() * sinf(theta);
}