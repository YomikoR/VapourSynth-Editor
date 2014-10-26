#ifndef RESAMPLEFILTERS_H_INCLUDED
#define RESAMPLEFILTERS_H_INCLUDED

namespace vsedit
{

	class AbstractResampleLinearFilter
	{
		public:

			virtual ~AbstractResampleLinearFilter();

			virtual size_t taps() const = 0;
			virtual double weight(double a_distance) const = 0;
	};

	class LinearResampleFilter : public AbstractResampleLinearFilter
	{
		public:

			virtual size_t taps() const override;
			virtual double weight(double a_distance) const override;
	};

	class CubicResampleFilter : public AbstractResampleLinearFilter
	{
		public:

			CubicResampleFilter(double a_b, double a_c);

			virtual size_t taps() const override;
			virtual double weight(double a_distance) const override;

		private:

		double m_p0;
		double m_p2;
		double m_p3;
		double m_q0;
		double m_q1;
		double m_q2;
		double m_q3;
	};

}

#endif // RESAMPLEFILTERS_H_INCLUDED
