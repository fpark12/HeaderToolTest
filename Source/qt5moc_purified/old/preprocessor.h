#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "parser.h"
#include <list>
#include <set>
#include <string>
#include <unordered_map>
#include <stdio.h>

namespace header_tool
{

	struct Macro
	{
		Macro() : isFunction(false), isVariadic(false)
		{}
		bool isFunction;
		bool isVariadic;
		Symbols arguments;
		Symbols symbols;
	};

#ifdef USE_LEXEM_STORE
	typedef std::string std::string;
#else
	//typedef std::string std::string;
	//typedef SubArray std::string;
#endif
//typedef std::unordered_map<std::string, Macro> std::unordered_map<std::string, Macro>;

	class QFile;

	class Preprocessor : public Parser
	{
	public:
		Preprocessor()
		{}

		static bool preprocessOnly;
		std::vector<std::string> frameworks;
		std::set<std::string> preprocessedIncludes;
		std::unordered_map<std::string, std::string> nonlocalIncludePathResolutionCache;
		//std::unordered_map<std::string, std::string> nonlocalIncludePathResolutionCache;
		std::unordered_map<std::string, Macro> macros;
		std::string resolveInclude(const std::string &filename, const std::string &relativeTo);
		Symbols preprocessed(const std::string &filename, FILE*& device);

		void parseDefineArguments(Macro *m);

		void skipUntilEndif();
		bool skipBranch();

		void substituteUntilNewline(Symbols &substituted);
		static Symbols macroExpandIdentifier(Preprocessor *that, SymbolStack &symbols, int lineNum, std::string *macro);
		static void macroExpand(Symbols *into, Preprocessor *that, const Symbols &toExpand, int &index, int lineNum, bool one,
			const std::set<std::string> &excludeSymbols = std::set<std::string>());

		int evaluateCondition();

		enum TokenizeMode
		{
			TokenizeCpp, TokenizePreprocessor, PreparePreprocessorStatement, TokenizePreprocessorStatement, TokenizeInclude, PrepareDefine, TokenizeDefine
		};
		static Symbols tokenize(const std::string &input, int lineNum = 1, TokenizeMode mode = TokenizeCpp);

	private:
		void until(Token);

		void preprocess(const std::string &filename, Symbols &preprocessed);
	};

}

#endif // PREPROCESSOR_H
