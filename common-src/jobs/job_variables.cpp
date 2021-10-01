#include "job_variables.h"

#include <QObject>

//==============================================================================

const QString JobVariables::TOKEN_WIDTH = "{w}";
const QString JobVariables::TOKEN_HEIGHT = "{h}";
const QString JobVariables::TOKEN_FPS_NUMERATOR = "{fpsn}";
const QString JobVariables::TOKEN_FPS_DENOMINATOR = "{fpsd}";
const QString JobVariables::TOKEN_FPS = "{fps}";
const QString JobVariables::TOKEN_BITDEPTH = "{bits}";
const QString JobVariables::TOKEN_SCRIPT_DIRECTORY = "{sd}";
const QString JobVariables::TOKEN_SCRIPT_NAME = "{sn}";
const QString JobVariables::TOKEN_FRAMES_NUMBER = "{f}";
const QString JobVariables::TOKEN_SUBSAMPLING = "{ss}";

//==============================================================================

JobVariables::JobVariables()
{
	fillVariables();
}

// END OF JobVariables::JobVariables()
//==============================================================================

std::vector<vsedit::VariableToken> JobVariables::variables() const
{
	std::vector<vsedit::VariableToken> cutVariables;
	for(const vsedit::VariableToken & variable : m_variables)
	{
		vsedit::VariableToken cutVariable =
			{variable.token, variable.description, std::function<QString()>()};
		cutVariables.push_back(cutVariable);
	}
	return cutVariables;
}

// END OF std::vector<vsedit::VariableToken> JobVariables::variables() const
//==============================================================================

void JobVariables::fillVariables()
{
	m_variables =
	{
		{TOKEN_WIDTH, QObject::tr("video width"),
			std::function<QString()>()},
		{TOKEN_HEIGHT, QObject::tr("video height"),
			std::function<QString()>()},
		{TOKEN_FPS_NUMERATOR, QObject::tr("video framerate numerator"),
			std::function<QString()>()},
		{TOKEN_FPS_DENOMINATOR, QObject::tr("video framerate denominator"),
			std::function<QString()>()},
		{TOKEN_FPS, QObject::tr("video framerate as fraction"),
			std::function<QString()>()},
		{TOKEN_BITDEPTH, QObject::tr("video colour bitdepth"),
			std::function<QString()>()},
		{TOKEN_SCRIPT_DIRECTORY, QObject::tr("script directory"),
			std::function<QString()>()},
		{TOKEN_SCRIPT_NAME, QObject::tr("script name without extension"),
			std::function<QString()>()},
		{TOKEN_FRAMES_NUMBER, QObject::tr("total frames number"),
			std::function<QString()>()},
		{TOKEN_SUBSAMPLING, QObject::tr("subsampling string (like 420)"),
			std::function<QString()>()},
	};

	std::sort(m_variables.begin(), m_variables.end(),
		[&](const vsedit::VariableToken & a_first,
			const vsedit::VariableToken & a_second) -> bool
		{
			return (a_first.token.length() > a_second.token.length());
		});
}

// END OF void JobVariables::fillVariables()
//==============================================================================
