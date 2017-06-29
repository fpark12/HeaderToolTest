#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "parser.h"
#include <list>
#include <set>
#include <string>
#include <unordered_map>
#include <stdio.h>

namespace header_tool {

struct Macro
{
    Macro() : isFunction(false), isVariadic(false) {}
    bool isFunction;
    bool isVariadic;
    Symbols arguments;
    Symbols symbols;
};

#ifdef USE_LEXEM_STORE
typedef std::vector<uint8> MacroName;
#else
typedef std::string MacroName;
//typedef SubArray MacroName;
#endif
typedef std::unordered_map<MacroName, Macro> Macros;

class QFile;

class Preprocessor : public Parser
{
public:
    Preprocessor(){}
    static bool preprocessOnly;
    std::list<std::vector<uint8>> frameworks;
    std::set<std::vector<uint8>> preprocessedIncludes;
	std::unordered_map<std::string, std::vector<uint8>> nonlocalIncludePathResolutionCache;
	//std::unordered_map<std::vector<uint8>, std::vector<uint8>> nonlocalIncludePathResolutionCache;
    Macros macros;
    std::vector<uint8> resolveInclude(const std::vector<uint8> &filename, const std::vector<uint8> &relativeTo);
    Symbols preprocessed(const std::vector<uint8> &filename, QFile *device);

    void parseDefineArguments(Macro *m);

    void skipUntilEndif();
    bool skipBranch();

    void substituteUntilNewline(Symbols &substituted);
    static Symbols macroExpandIdentifier(Preprocessor *that, SymbolStack &symbols, int lineNum, std::vector<uint8> *macroName);
    static void macroExpand(Symbols *into, Preprocessor *that, const Symbols &toExpand, int &index, int lineNum, bool one,
                               const std::set<std::vector<uint8>> &excludeSymbols = std::set<std::vector<uint8>>());

    int evaluateCondition();

    enum TokenizeMode { TokenizeCpp, TokenizePreprocessor, PreparePreprocessorStatement, TokenizePreprocessorStatement, TokenizeInclude, PrepareDefine, TokenizeDefine };
    static Symbols tokenize(const std::string &input, int lineNum = 1, TokenizeMode mode = TokenizeCpp);

private:
    void until(Token);

    void preprocess(const std::vector<uint8> &filename, Symbols &preprocessed);
};

}

#endif // PREPROCESSOR_H
