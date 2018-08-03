#ifndef SCRIPT_LEXER_DEFINITIONS_H
#define SCRIPT_LEXER_DEFINITIONS_H

#include <list>
#include <QString>

enum class ScriptContextType
{
	Code,
	ModuleImport,
	Function,
	FunctionHeader,
	FunctionArguments,
	FunctionCallArguments,
	Expression,
	Comment,
};

enum class TokenType
{
	Undecided,
	Module,
	Keyword,
	Operator,
	String,
	Number,
	Comment,
	Variable,
	Function,
	FunctionArgument,
	VSModule,
	VSCore,
	VSNamespace,
	VSFunction,
	VSConstant,
	VSClip,
};

struct ScriptContext
{
	ScriptContextType type;
	int start;
	int end;
};

struct Token
{
	QString text;
	int start;
	TokenType type;
	std::list<ScriptContext> contextStack;

	int length() const;
	bool operator<(const Token & a_other) const;
};

#endif // SCRIPT_LEXER_DEFINITIONS_H
