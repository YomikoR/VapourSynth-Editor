#ifndef NUMBERMATCHER_H_INCLUDED
#define NUMBERMATCHER_H_INCLUDED

#include <QString>

/// Finite state machine Python number matcher for QString.
class NumberMatcher
{
	public:

		NumberMatcher();

		bool beginsWithNumber(const QString & a_string, int a_matchFrom);

		int matchedLength() const;

	private:

		enum State
		{
			Initial,
			FirstZero, // Valid final state
			BinLiteral,
			BinNumber, // Valid final state
			OctLiteral,
			OctNumber, // Valid final state
			HexLiteral,
			HexNumber, // Valid final state
			Integer, // Valid final state
			DotAfterInteger, // Valid final state
			FirstDot,
			Fraction, // Valid final state
			ExpLiteral,
			ExpSign,
			ExpComplete, // Valid final state
		};

		State m_state;

		int m_lastValidLength;
};

#endif // NUMBERMATCHER_H_INCLUDED
