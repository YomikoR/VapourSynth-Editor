#ifndef SCRIPT_LEXER_H
#define SCRIPT_LEXER_H

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

struct Token
{
	QString text;
	int start;
	int length;
	TokenType type;
};

#endif // SCRIPT_LEXER_H
