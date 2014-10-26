#include <cmath>

#include "resamplefilters.h"

//==============================================================================

vsedit::AbstractResampleLinearFilter::~AbstractResampleLinearFilter()
{
}

//==============================================================================

size_t vsedit::LinearResampleFilter::taps() const
{
	return 2;
}

double vsedit::LinearResampleFilter::weight(double a_distance) const
{
	double t = std::fabs(a_distance);

	if(t < 1.0)
		return (1.0 - t);

	return 0.0;
}

//==============================================================================

vsedit::CubicResampleFilter::CubicResampleFilter(double a_b, double a_c) :
	m_p0((6.0 - 2.0 * a_b) / 6.0),
	m_p2((-18.0 + 12.0 * a_b + 6.0 * a_c) / 6.0),
	m_p3((12.0 - 9.0 * a_b - 6.0 * a_c) / 6.0),
	m_q0((8.0 * a_b + 24.0 * a_c) / 6.0),
	m_q1((-12.0 * a_b - 48.0 * a_c) / 6.0),
	m_q2((6.0 * a_b + 30.0 * a_c) / 6.0),
	m_q3((-a_b - 6.0 * a_c) / 6.0)
{
}

size_t vsedit::CubicResampleFilter::taps() const
{
	return 4;
}

double vsedit::CubicResampleFilter::weight(double a_distance) const
{
	double t = std::abs(a_distance);

	if(t < 1.0)
		return (m_p0 + t * t * (m_p2 + t * m_p3));
	else if(t < 2.0)
		return (m_q0 + t * (m_q1 + t * (m_q2 + t * m_q3)));

	return 0.0;
}

//==============================================================================
