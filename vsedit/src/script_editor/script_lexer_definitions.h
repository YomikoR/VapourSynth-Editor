#ifndef SCRIPT_LEXER_DEFINITIONS_H
#define SCRIPT_LEXER_DEFINITIONS_H

#include <QString>
#include <list>
#include <vector>
#include <memory>

enum class ScriptContextType
{
	Code,
	ModuleImport,
	Function,
	FunctionHeader,
	FunctionArguments,
	FunctionCallArguments,
	Expression,
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
	int start{0};
	int end{-1};
	ScriptContext * pParent{nullptr};
	std::list<std::unique_ptr<ScriptContext> > children;
};

struct Token
{
	QString text;
	int start;
	TokenType type{TokenType::Undecided};

	Token(QString a_text, int a_start, TokenType a_type);
	Token(int a_start);
	int length() const;
	bool operator<(const Token & a_other) const;
};

typedef std::vector<Token> TokenVector;
typedef TokenVector::iterator TokenIterator;

#endif // SCRIPT_LEXER_DEFINITIONS_H
