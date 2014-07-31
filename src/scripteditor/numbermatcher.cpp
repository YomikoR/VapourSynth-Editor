#include "numbermatcher.h"

NumberMatcher::NumberMatcher() : m_state(Initial), m_lastValidLength(0)
{

}

bool NumberMatcher::beginsWithNumber(const QString & a_string, int a_matchFrom)
{
	m_state = Initial;
	m_lastValidLength = 0;
	int stringLength = a_string.length();

	for(int i = a_matchFrom; i < stringLength; ++i)
	{
		if(m_state == Initial)
		{
			if(a_string[i] == '0')
			{
				m_state = FirstZero;
				m_lastValidLength = i - a_matchFrom + 1;
			}
			else if(QString("123456789").contains(a_string[i]))
			{
				m_state = Integer;
				m_lastValidLength = i - a_matchFrom + 1;
			}
			else if(a_string[i] == '.')
				m_state = FirstDot;
			else
				break;
		}
		else if(m_state == FirstZero)
		{
			if(a_string[i].toLower() == 'b')
				m_state = BinLiteral;
			else if(a_string[i].toLower() == 'x')
				m_state = HexLiteral;
			else if(a_string[i].toLower() == 'o')
				m_state = OctLiteral;
			else if(QString("0123456789").contains(a_string[i]))
			{
				m_state = Integer;
				m_lastValidLength = i - a_matchFrom + 1;
			}
			else if(a_string[i] == '.')
			{
				m_state = DotAfterInteger;
				m_lastValidLength = i - a_matchFrom + 1;
			}
			else if(a_string[i].toLower() == 'j')
			{
				m_lastValidLength = i - a_matchFrom + 1;
				return true;
			}
			else
				break;
		}
		else if(m_state == BinLiteral)
		{
			if(QString("01").contains(a_string[i]))
			{
				m_state = BinNumber;
				m_lastValidLength = i - a_matchFrom + 1;
			}
			else
				break;
		}
		else if(m_state == BinNumber)
		{
			if(QString("01").contains(a_string[i]))
				m_lastValidLength = i - a_matchFrom + 1;
			else
				break;
		}
		else if(m_state == OctLiteral)
		{
			if(QString("01234567").contains(a_string[i]))
			{
				m_state = OctNumber;
				m_lastValidLength = i - a_matchFrom + 1;
			}
			else
				break;
		}
		else if(m_state == OctNumber)
		{
			if(QString("01234567").contains(a_string[i]))
				m_lastValidLength = i - a_matchFrom + 1;
			else
				break;
		}
		else if(m_state == HexLiteral)
		{
			if(QString("0123456789abcdef").contains(a_string[i].toLower()))
			{
				m_state = HexNumber;
				m_lastValidLength = i - a_matchFrom + 1;
			}
			else
				break;
		}
		else if(m_state == HexNumber)
		{
			if(QString("0123456789abcdef").contains(a_string[i].toLower()))
				m_lastValidLength = i - a_matchFrom + 1;
			else
				break;
		}
		else if(m_state == Integer)
		{
			if(QString("0123456789").contains(a_string[i]))
				m_lastValidLength = i - a_matchFrom + 1;
			else if(a_string[i] == '.')
			{
				m_state = DotAfterInteger;
				m_lastValidLength = i - a_matchFrom + 1;
			}
			else if(a_string[i].toLower() == 'e')
				m_state = ExpLiteral;
			else if(a_string[i].toLower() == 'j')
			{
				m_lastValidLength = i - a_matchFrom + 1;
				return true;
			}
			else
				break;
		}
		else if(m_state == DotAfterInteger)
		{
			if(QString("0123456789").contains(a_string[i]))
			{
				m_state = Fraction;
				m_lastValidLength = i - a_matchFrom + 1;
			}
			else if(a_string[i].toLower() == 'e')
				m_state = ExpLiteral;
			else if(a_string[i].toLower() == 'j')
			{
				m_lastValidLength = i - a_matchFrom + 1;
				return true;
			}
			else
				break;
		}
		else if(m_state == FirstDot)
		{
			if(QString("0123456789").contains(a_string[i]))
			{
				m_state = Fraction;
				m_lastValidLength = i - a_matchFrom + 1;
			}
			else
				break;
		}
		else if(m_state == Fraction)
		{
			if(QString("0123456789").contains(a_string[i]))
				m_lastValidLength = i - a_matchFrom + 1;
			else if(a_string[i].toLower() == 'e')
				m_state = ExpLiteral;
			else if(a_string[i].toLower() == 'j')
			{
				m_lastValidLength = i - a_matchFrom + 1;
				return true;
			}
			else
				break;
		}
		else if(m_state == ExpLiteral)
		{
			if(QString("+-").contains(a_string[i]))
				m_state = ExpSign;
			else if(QString("0123456789").contains(a_string[i]))
			{
				m_state = ExpComplete;
				m_lastValidLength = i - a_matchFrom + 1;
			}
			else
				break;
		}
		else if(m_state == ExpSign)
		{
			if(QString("0123456789").contains(a_string[i]))
			{
				m_state = ExpComplete;
				m_lastValidLength = i - a_matchFrom + 1;
			}
			else
				break;
		}
		else if(m_state == ExpComplete)
		{
			if(QString("0123456789").contains(a_string[i]))
				m_lastValidLength = i - a_matchFrom + 1;
			else if(a_string[i].toLower() == 'j')
			{
				m_lastValidLength = i - a_matchFrom + 1;
				return true;
			}
		}
		else
			break;
	}

	return (m_lastValidLength != 0);
}

int NumberMatcher::matchedLength() const
{
	return m_lastValidLength;
}
