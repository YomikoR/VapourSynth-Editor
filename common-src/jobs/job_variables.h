#ifndef JOB_VARIABLES_H_INCLUDED
#define JOB_VARIABLES_H_INCLUDED

#include "../helpers.h"

#include <functional>
#include <vector>

class JobVariables
{
public:

	JobVariables();

	virtual std::vector<vsedit::VariableToken> variables() const;

protected:

	static const QString TOKEN_WIDTH;
	static const QString TOKEN_HEIGHT;
	static const QString TOKEN_FPS_NUMERATOR;
	static const QString TOKEN_FPS_DENOMINATOR;
	static const QString TOKEN_FPS;
	static const QString TOKEN_BITDEPTH;
	static const QString TOKEN_SCRIPT_DIRECTORY;
	static const QString TOKEN_SCRIPT_NAME;
	static const QString TOKEN_FRAMES_NUMBER;
	static const QString TOKEN_SUBSAMPLING;

	virtual void fillVariables();

	std::vector<vsedit::VariableToken> m_variables;
};

#endif // JOB_VARIABLES_H_INCLUDED
