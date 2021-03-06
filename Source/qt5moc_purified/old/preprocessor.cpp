#include "preprocessor.h"
#include "utils.h"

#define Q_FALLTHROUGH()

namespace header_tool
{

	bool Preprocessor::preprocessOnly = false;
}

#if 0
#include <std::vector<std::string>.h>
#include <qfile.h>
#include <qdir.h>
#include <qfileinfo.h>
#endif

namespace header_tool
{
	// generated headers
#include "ppkeywords.h"
#include "keywords.h"





// transform \r\n into \n
// \r into \n (os9 style)
// backslash-newlines into newlines
	static std::string cleaned(const std::string &input)
	{
		std::string result;
		result.resize(input.size());
		const char *data = input.data();
		const char *end = input.data() + input.size();
		char *output = const_cast<char*>(result.data());

		int newlines = 0;
		while (data != end)
		{
			while (data != end && is_space(*data))
				++data;
			bool takeLine = (*data == '#');
			if (*data == '%' && *(data + 1) == ':')
			{
				takeLine = true;
				++data;
			}
			if (takeLine)
			{
				*output = '#';
				++output;
				do ++data; while (data != end && is_space(*data));
			}
			while (data != end)
			{
				// handle \\\n, \\\r\n and \\\r
				if (*data == '\\')
				{
					if (*(data + 1) == '\r')
					{
						++data;
					}
					if (data != end && (*(data + 1) == '\n' || (*data) == '\r'))
					{
						++newlines;
						data += 1;
						if (data != end && *data != '\r')
							data += 1;
						continue;
					}
				}
				else if (*data == '\r' && *(data + 1) == '\n')
				{ // reduce \r\n to \n
					++data;
				}
				if (data == end)
					break;

				char ch = *data;
				if (ch == '\r') // os9: replace \r with \n
					ch = '\n';
				*output = ch;
				++output;

				if (*data == '\n')
				{
					// output additional newlines to keep the correct line-numbering
					// for the lines following the backslash-newline sequence(s)
					while (newlines)
					{
						*output = '\n';
						++output;
						--newlines;
					}
					++data;
					break;
				}
				++data;
			}
		}
		result.resize(output - result.data());
		return result;
	}

	void Preprocessor::skipUntilEndif()
	{
		while (index < symbols.size() - 1 && symbols.at(index).token != PP_ENDIF)
		{
			switch (symbols.at(index).token)
			{
				case PP_IF:
				case PP_IFDEF:
				case PP_IFNDEF:
					++index;
					skipUntilEndif();
					break;
				default:
					;
			}
			++index;
		}
	}

	bool Preprocessor::skipBranch()
	{
		while (index < symbols.size() - 1
			&& (symbols.at(index).token != PP_ENDIF
			&& symbols.at(index).token != PP_ELIF
			&& symbols.at(index).token != PP_ELSE)
			)
		{
			switch (symbols.at(index).token)
			{
				case PP_IF:
				case PP_IFDEF:
				case PP_IFNDEF:
					++index;
					skipUntilEndif();
					break;
				default:
					;
			}
			++index;
		}
		return (index < symbols.size() - 1);
	}

	std::vector<Symbol> Preprocessor::tokenize(const std::string& input, int lineNum, Preprocessor::TokenizeMode mode)
	{
		std::vector<Symbol> symbols;
		// Preallocate some space to speed up the code below.
		// The magic divisor value was found by calculating the average ratio between
		// input size and the final size of symbols.
		// This yielded a value of 16.x when compiling Qt Base.
		symbols.reserve(input.size() / 16);
		const char *begin = input.data();
		const char *data = begin;
		while (*data)
		{
			if (mode == TokenizeCpp || mode == TokenizeDefine)
			{
				int column = 0;

				const char *lexem = data;
				int state = 0;
				Token token = NOTOKEN;
				for (;;)
				{
					if (static_cast<signed char>(*data) < 0)
					{
						++data;
						continue;
					}
					int nextindex = keywords[state].next;
					int next = 0;
					if (*data == keywords[state].defchar)
						next = keywords[state].defnext;
					else if (!state || nextindex)
						next = keyword_trans[nextindex][(int)*data];
					if (!next)
						break;
					state = next;
					token = keywords[state].token;
					++data;
				}

				// suboptimal, is_ident_char  should use a table
				if (keywords[state].ident && is_ident_char(*data))
					token = keywords[state].ident;

				if (token == NOTOKEN)
				{
					if (*data)
						++data;
					// an error really, but let's ignore this input
					// to not confuse moc later. However in pre-processor
					// only mode let's continue.
					if (!Preprocessor::preprocessOnly)
						continue;
				}

				++column;

				if (token > SPECIAL_TREATMENT_MARK)
				{
					switch (token)
					{
						case QUOTE:
							data = skipQuote(data);
							token = STRING_LITERAL;
							// concatenate multi-line strings for easier
							// STRING_LITERAL handling in moc
							if (!Preprocessor::preprocessOnly
								&& !symbols.empty()
								&& symbols.back().token == STRING_LITERAL)
							{

								const std::string newString
									= '\"'
									+ symbols.back().unquotedLexem()
									+ sub(input, lexem - begin + 1, data - lexem - 2)
									+ '\"';
								symbols.back() = Symbol(symbols.back().lineNum, STRING_LITERAL, newString);
								continue;
							}
							break;
						case SINGLEQUOTE:
							while (*data && (*data != '\''
								|| (*(data - 1) == '\\'
								&& *(data - 2) != '\\')))
								++data;
							if (*data)
								++data;
							token = CHARACTER_LITERAL;
							break;
						case LANGLE_SCOPE:
							// split <:: into two tokens, < and ::
							token = LANGLE;
							data -= 2;
							break;
						case DIGIT:
							while (is_digit_char(*data) || *data == '\'')
								++data;
							if (!*data || *data != '.')
							{
								token = INTEGER_LITERAL;
								if (data - lexem == 1 &&
									(*data == 'x' || *data == 'X')
									&& *lexem == '0')
								{
									++data;
									while (is_hex_char(*data) || *data == '\'')
										++data;
								}
								break;
							}
							token = FLOATING_LITERAL;
							++data;
							Q_FALLTHROUGH();
						case FLOATING_LITERAL:
							while (is_digit_char(*data) || *data == '\'')
								++data;
							if (*data == '+' || *data == '-')
								++data;
							if (*data == 'e' || *data == 'E')
							{
								++data;
								while (is_digit_char(*data) || *data == '\'')
									++data;
							}
							if (*data == 'f' || *data == 'F'
								|| *data == 'l' || *data == 'L')
								++data;
							break;
						case HASH:
							if (column == 1 && mode == TokenizeCpp)
							{
								mode = PreparePreprocessorStatement;
								while (*data && (*data == ' ' || *data == '\t'))
									++data;
								if (is_ident_char(*data))
									mode = TokenizePreprocessorStatement;
								continue;
							}
							break;
						case PP_HASHHASH:
							if (mode == TokenizeCpp)
								continue;
							break;
						case NEWLINE:
							++lineNum;
							if (mode == TokenizeDefine)
							{
								mode = TokenizeCpp;
								// emit the newline token
								break;
							}
							continue;
						case BACKSLASH:
							{
								const char *rewind = data;
								while (*data && (*data == ' ' || *data == '\t'))
									++data;
								if (*data && *data == '\n')
								{
									++data;
									continue;
								}
								data = rewind;
							} break;
						case CHARACTER:
							while (is_ident_char(*data))
								++data;
							token = IDENTIFIER;
							break;
						case C_COMMENT:
							if (*data)
							{
								if (*data == '\n')
									++lineNum;
								++data;
								if (*data)
								{
									if (*data == '\n')
										++lineNum;
									++data;
								}
							}
							while (*data && (*(data - 1) != '/' || *(data - 2) != '*'))
							{
								if (*data == '\n')
									++lineNum;
								++data;
							}
							token = WHITESPACE; // one comment, one whitespace
							Q_FALLTHROUGH();
						case WHITESPACE:
							if (column == 1)
								column = 0;
							while (*data && (*data == ' ' || *data == '\t'))
								++data;
							if (Preprocessor::preprocessOnly) // tokenize whitespace
								break;
							continue;
						case CPP_COMMENT:
							while (*data && *data != '\n')
								++data;
							continue; // ignore safely, the newline is a separator
						default:
							continue; //ignore
					}
				}
#ifdef USE_LEXEM_STORE
				if (!Preprocessor::preprocessOnly
					&& token != IDENTIFIER
					&& token != STRING_LITERAL
					&& token != FLOATING_LITERAL
					&& token != INTEGER_LITERAL)
					symbols += Symbol(lineNum, token);
				else
#endif
					symbols.emplace_back(lineNum, token, input, lexem - begin, data - lexem);

			}
			else
			{ //   Preprocessor

				const char *lexem = data;
				int state = 0;
				Token token = NOTOKEN;
				if (mode == TokenizePreprocessorStatement)
				{
					state = pp_keyword_trans[0][(int)'#'];
					mode = TokenizePreprocessor;
				}
				for (;;)
				{
					if (static_cast<signed char>(*data) < 0)
					{
						++data;
						continue;
					}
					int nextindex = pp_keywords[state].next;
					int next = 0;
					if (*data == pp_keywords[state].defchar)
						next = pp_keywords[state].defnext;
					else if (!state || nextindex)
						next = pp_keyword_trans[nextindex][(int)*data];
					if (!next)
						break;
					state = next;
					token = pp_keywords[state].token;
					++data;
				}
				// suboptimal, is_ident_char  should use a table
				if (pp_keywords[state].ident && is_ident_char(*data))
					token = pp_keywords[state].ident;

				switch (token)
				{
					case NOTOKEN:
						if (*data)
							++data;
						break;
					case PP_DEFINE:
						mode = PrepareDefine;
						break;
					case PP_IFDEF:
						symbols.emplace_back(lineNum, PP_IF);
						symbols.emplace_back(lineNum, PP_DEFINED);
						continue;
					case PP_IFNDEF:
						symbols.emplace_back(lineNum, PP_IF);
						symbols.emplace_back(lineNum, PP_NOT);
						symbols.emplace_back(lineNum, PP_DEFINED);
						continue;
					case PP_INCLUDE:
						mode = TokenizeInclude;
						break;
					case PP_QUOTE:
						data = skipQuote(data);
						token = PP_STRING_LITERAL;
						break;
					case PP_SINGLEQUOTE:
						while (*data && (*data != '\''
							|| (*(data - 1) == '\\'
							&& *(data - 2) != '\\')))
							++data;
						if (*data)
							++data;
						token = PP_CHARACTER_LITERAL;
						break;
					case PP_DIGIT:
						while (is_digit_char(*data) || *data == '\'')
							++data;
						if (!*data || *data != '.')
						{
							token = PP_INTEGER_LITERAL;
							if (data - lexem == 1 &&
								(*data == 'x' || *data == 'X')
								&& *lexem == '0')
							{
								++data;
								while (is_hex_char(*data) || *data == '\'')
									++data;
							}
							break;
						}
						token = PP_FLOATING_LITERAL;
						++data;
						Q_FALLTHROUGH();
					case PP_FLOATING_LITERAL:
						while (is_digit_char(*data) || *data == '\'')
							++data;
						if (*data == '+' || *data == '-')
							++data;
						if (*data == 'e' || *data == 'E')
						{
							++data;
							while (is_digit_char(*data) || *data == '\'')
								++data;
						}
						if (*data == 'f' || *data == 'F'
							|| *data == 'l' || *data == 'L')
							++data;
						break;
					case PP_CHARACTER:
						if (mode == PreparePreprocessorStatement)
						{
							// rewind entire token to begin
							data = lexem;
							mode = TokenizePreprocessorStatement;
							continue;
						}
						while (is_ident_char(*data))
							++data;
						token = PP_IDENTIFIER;

						if (mode == PrepareDefine)
						{
							symbols.emplace_back(lineNum, token, input, lexem - begin, data - lexem);
							// make sure we explicitly add the whitespace here if the next char
							// is not an opening brace, so we can distinguish correctly between
							// regular and function std::unordered_map<std::string, Macro>
							if (*data != '(')
								symbols.emplace_back(lineNum, WHITESPACE);
							mode = TokenizeDefine;
							continue;
						}
						break;
					case PP_C_COMMENT:
						if (*data)
						{
							if (*data == '\n')
								++lineNum;
							++data;
							if (*data)
							{
								if (*data == '\n')
									++lineNum;
								++data;
							}
						}
						while (*data && (*(data - 1) != '/' || *(data - 2) != '*'))
						{
							if (*data == '\n')
								++lineNum;
							++data;
						}
						token = PP_WHITESPACE; // one comment, one whitespace
						Q_FALLTHROUGH();
					case PP_WHITESPACE:
						while (*data && (*data == ' ' || *data == '\t'))
							++data;
						continue; // the preprocessor needs no whitespace
					case PP_CPP_COMMENT:
						while (*data && *data != '\n')
							++data;
						continue; // ignore safely, the newline is a separator
					case PP_NEWLINE:
						++lineNum;
						mode = TokenizeCpp;
						break;
					case PP_BACKSLASH:
						{
							const char *rewind = data;
							while (*data && (*data == ' ' || *data == '\t'))
								++data;
							if (*data && *data == '\n')
							{
								++data;
								continue;
							}
							data = rewind;
						} break;
					case PP_LANGLE:
						if (mode != TokenizeInclude)
							break;
						token = PP_STRING_LITERAL;
						while (*data && *data != '\n' && *(data - 1) != '>')
							++data;
						break;
					default:
						break;
				}
				if (mode == PreparePreprocessorStatement)
					continue;
#ifdef USE_LEXEM_STORE
				if (token != PP_IDENTIFIER
					&& token != PP_STRING_LITERAL
					&& token != PP_FLOATING_LITERAL
					&& token != PP_INTEGER_LITERAL)
					symbols += Symbol(lineNum, token);
				else
#endif
					symbols.emplace_back(lineNum, token, input, lexem - begin, data - lexem);
			}
		}
		symbols.emplace_back(); // eof symbol
		return symbols;
	}

	void Preprocessor::macroExpand(std::vector<Symbol> *into, Preprocessor *that, const std::vector<Symbol> &toExpand, int &index,
		int lineNum, bool one, const std::set<std::string> &excludeSymbols)
	{
		SymbolStack symbols;
		SafeSymbols sf;
		sf.symbols = toExpand;
		sf.index = index;
		sf.excludedSymbols = excludeSymbols;
		symbols.push(sf);

		if (toExpand.empty())
			return;

		for (;;)
		{
			std::string macro;
			std::vector<Symbol> newSyms = macroExpandIdentifier(that, symbols, lineNum, &macro);

			if (macro.empty())
			{
				// not a macro
				Symbol s = symbols.symbol();
				s.lineNum = lineNum;
				(*into).push_back(s);
			}
			else
			{
				SafeSymbols sf;
				sf.symbols = newSyms;
				sf.index = 0;
				sf.expandedMacro = macro;
				symbols.push(sf);
			}
			if (!symbols.hasNext() || (one && symbols.size() == 1))
				break;
			symbols.next();
		}

		if (symbols.size())
			index = symbols.top().index;
		else
			index = toExpand.size();
	}

	std::vector<Symbol> Preprocessor::macroExpandIdentifier(Preprocessor *that, SymbolStack &symbols, int lineNum, std::string* macroName)
	{
		Symbol s = symbols.symbol();

		auto& macro_itr = that->macros.find(s.lex);

		// not a macro
		if (s.token != PP_IDENTIFIER || macro_itr == that->macros.end() || symbols.dontReplaceSymbol(s.lexem()))
		{
			return std::vector<Symbol>();
		}

		const Macro &macro = (*macro_itr).second;
		*macroName = s.lexem();

		std::vector<Symbol> expansion;
		if (!macro.isFunction)
		{
			expansion = macro.symbols;
		}
		else
		{
			bool haveSpace = false;
			while (symbols.test(PP_WHITESPACE))
			{
				haveSpace = true;
			}
			if (!symbols.test(PP_LPAREN))
			{
				*macroName = std::string();
				std::vector<Symbol> syms;
				if (haveSpace)
					syms.push_back(Symbol(lineNum, PP_WHITESPACE));
				syms.push_back(s);
				syms.back().lineNum = lineNum;
				return syms;
			}
			std::vector<std::vector<Symbol>> arguments;
			arguments.reserve(5);
			while (symbols.hasNext())
			{
				std::vector<Symbol> argument;
				// strip leading space
				while (symbols.test(PP_WHITESPACE))
				{
				}
				int nesting = 0;
				bool vararg = macro.isVariadic && (arguments.size() == macro.arguments.size() - 1);
				while (symbols.hasNext())
				{
					Token t = symbols.next();
					if (t == PP_LPAREN)
					{
						++nesting;
					}
					else if (t == PP_RPAREN)
					{
						--nesting;
						if (nesting < 0)
							break;
					}
					else if (t == PP_COMMA && nesting == 0)
					{
						if (!vararg)
							break;
					}
					argument.push_back(symbols.symbol());
				}
				arguments.push_back(argument);

				if (nesting < 0)
					break;
				else if (!symbols.hasNext())
					that->error("missing ')' in macro usage");
			}

			// empty VA_ARGS
			if (macro.isVariadic && arguments.size() == macro.arguments.size() - 1)
			{
				// todo:
				// arguments += std::vector<Symbol>();
			}

			// now replace the macro arguments with the expanded arguments
			enum Mode
			{
				Normal,
				Hash,
				HashHash
			} mode = Normal;

			for (int i = 0; i < macro.symbols.size(); ++i)
			{
				const Symbol &s = macro.symbols[i];
				if (s.token == HASH || s.token == PP_HASHHASH)
				{
					mode = (s.token == HASH ? Hash : HashHash);
					continue;
				}
				int index = std::distance(macro.arguments.begin(), std::find(macro.arguments.begin(), macro.arguments.end(), s));
				if (mode == Normal)
				{
					if (index >= 0 && index < arguments.size())
					{
						// each argument undoergoes macro expansion if it's not used as part of a # or ##
						if (i == macro.symbols.size() - 1 || macro.symbols.at(i + 1).token != PP_HASHHASH)
						{
							std::vector<Symbol> arg = arguments.at(index);
							int idx = 1;
							macroExpand(&expansion, that, arg, idx, lineNum, false, symbols.excludeSymbols());
						}
						else
						{
							expansion.insert(expansion.end(), arguments[index].begin(), arguments[index].end());
						}
					}
					else
					{
						expansion.push_back(s);
					}
				}
				else if (mode == Hash)
				{
					if (index < 0)
					{
						that->error("'#' is not followed by a macro parameter");
						continue;
					}
					else if (index >= arguments.size())
					{
						that->error("Macro invoked with too few parameters for a use of '#'");
						continue;
					}

					const std::vector<Symbol> &arg = arguments.at(index);
					std::string stringified;
					for (int i = 0; i < arg.size(); ++i)
					{
						stringified += arg.at(i).lexem();
					}
					replace_all(stringified, R"(")", R"(\")");
					stringified.insert(0, 1, '"');
					stringified.insert(stringified.end(), 1, '"');
					expansion.push_back(Symbol(lineNum, STRING_LITERAL, stringified));
				}
				else if (mode == HashHash)
				{
					if (s.token == WHITESPACE)
						continue;

					while (expansion.size() && expansion.back().token == PP_WHITESPACE)
						expansion.pop_back();

					Symbol next = s;
					if (index >= 0 && index < arguments.size())
					{
						const std::vector<Symbol> &arg = arguments.at(index);
						if (arg.size() == 0)
						{
							mode = Normal;
							continue;
						}
						next = arg.at(0);
					}

					if (!expansion.empty() && expansion.back().token == s.token
						&& expansion.back().token != STRING_LITERAL)
					{
						Symbol last = expansion.back();
						expansion.pop_back();

						std::string lexem = last.lexem() + next.lexem();
						expansion.push_back(Symbol(lineNum, last.token, lexem));
					}
					else
					{
						expansion.push_back(next);
					}

					if (index >= 0 && index < arguments.size())
					{
						const std::vector<Symbol> &arg = arguments.at(index);
						for (int i = 1; i < arg.size(); ++i)
							expansion.push_back(arg[i]);
					}
				}
				mode = Normal;
			}
			if (mode != Normal)
				that->error("'#' or '##' found at the end of a macro argument");

		}

		return expansion;
	}

	void Preprocessor::substituteUntilNewline(std::vector<Symbol> &substituted)
	{
		while (hasNext())
		{
			Token token = next();
			if (token == PP_IDENTIFIER)
			{
				macroExpand(&substituted, this, symbols, index, symbol().lineNum, true);
			}
			else if (token == PP_DEFINED)
			{
				bool braces = test(PP_LPAREN);
				next(PP_IDENTIFIER);
				Symbol definedOrNotDefined = symbol();
				definedOrNotDefined.token = macros.find(definedOrNotDefined.lex) != macros.end() ? PP_MOC_TRUE : PP_MOC_FALSE;
				substituted.push_back(definedOrNotDefined);
				if (braces)
					test(PP_RPAREN);
				continue;
			}
			else if (token == PP_NEWLINE)
			{
				substituted.emplace_back();
				break;
			}
			else
			{
				substituted.emplace_back();
			}
		}
	}

	class PP_Expression : public Parser
	{
	public:
		int value()
		{
			index = 0; return unary_expression_lookup() ? conditional_expression() : 0;
		}

		int conditional_expression();
		int logical_OR_expression();
		int logical_AND_expression();
		int inclusive_OR_expression();
		int exclusive_OR_expression();
		int AND_expression();
		int equality_expression();
		int relational_expression();
		int shift_expression();
		int additive_expression();
		int multiplicative_expression();
		int unary_expression();
		bool unary_expression_lookup();
		int primary_expression();
		bool primary_expression_lookup();
	};

	int PP_Expression::conditional_expression()
	{
		int value = logical_OR_expression();
		if (test(PP_QUESTION))
		{
			int alt1 = conditional_expression();
			int alt2 = test(PP_COLON) ? conditional_expression() : 0;
			return value ? alt1 : alt2;
		}
		return value;
	}

	int PP_Expression::logical_OR_expression()
	{
		int value = logical_AND_expression();
		if (test(PP_OROR))
			return logical_OR_expression() || value;
		return value;
	}

	int PP_Expression::logical_AND_expression()
	{
		int value = inclusive_OR_expression();
		if (test(PP_ANDAND))
			return logical_AND_expression() && value;
		return value;
	}

	int PP_Expression::inclusive_OR_expression()
	{
		int value = exclusive_OR_expression();
		if (test(PP_OR))
			return value | inclusive_OR_expression();
		return value;
	}

	int PP_Expression::exclusive_OR_expression()
	{
		int value = AND_expression();
		if (test(PP_HAT))
			return value ^ exclusive_OR_expression();
		return value;
	}

	int PP_Expression::AND_expression()
	{
		int value = equality_expression();
		if (test(PP_AND))
			return value & AND_expression();
		return value;
	}

	int PP_Expression::equality_expression()
	{
		int value = relational_expression();
		switch (next())
		{
			case PP_EQEQ:
				return value == equality_expression();
			case PP_NE:
				return value != equality_expression();
			default:
				prev();
				return value;
		}
	}

	int PP_Expression::relational_expression()
	{
		int value = shift_expression();
		switch (next())
		{
			case PP_LANGLE:
				return value < relational_expression();
			case PP_RANGLE:
				return value > relational_expression();
			case PP_LE:
				return value <= relational_expression();
			case PP_GE:
				return value >= relational_expression();
			default:
				prev();
				return value;
		}
	}

	int PP_Expression::shift_expression()
	{
		int value = additive_expression();
		switch (next())
		{
			case PP_LTLT:
				return value << shift_expression();
			case PP_GTGT:
				return value >> shift_expression();
			default:
				prev();
				return value;
		}
	}

	int PP_Expression::additive_expression()
	{
		int value = multiplicative_expression();
		switch (next())
		{
			case PP_PLUS:
				return value + additive_expression();
			case PP_MINUS:
				return value - additive_expression();
			default:
				prev();
				return value;
		}
	}

	int PP_Expression::multiplicative_expression()
	{
		int value = unary_expression();
		switch (next())
		{
			case PP_STAR:
				return value * multiplicative_expression();
			case PP_PERCENT:
				{
					int remainder = multiplicative_expression();
					return remainder ? value % remainder : 0;
				}
			case PP_SLASH:
				{
					int div = multiplicative_expression();
					return div ? value / div : 0;
				}
			default:
				prev();
				return value;
		};
	}

	int PP_Expression::unary_expression()
	{
		switch (next())
		{
			case PP_PLUS:
				return unary_expression();
			case PP_MINUS:
				return -unary_expression();
			case PP_NOT:
				return !unary_expression();
			case PP_TILDE:
				return ~unary_expression();
			case PP_MOC_TRUE:
				return 1;
			case PP_MOC_FALSE:
				return 0;
			default:
				prev();
				return primary_expression();
		}
	}

	bool PP_Expression::unary_expression_lookup()
	{
		Token t = lookup();
		return (primary_expression_lookup()
			|| t == PP_PLUS
			|| t == PP_MINUS
			|| t == PP_NOT
			|| t == PP_TILDE
			|| t == PP_DEFINED);
	}

	int PP_Expression::primary_expression()
	{
		int value;
		if (test(PP_LPAREN))
		{
			value = conditional_expression();
			test(PP_RPAREN);
		}
		else
		{
			next();
			value = std::stoi(lexem());
			// TODO
			//value = lexem().toInt(0, 0);
		}
		return value;
	}

	bool PP_Expression::primary_expression_lookup()
	{
		Token t = lookup();
		return (t == PP_IDENTIFIER
			|| t == PP_INTEGER_LITERAL
			|| t == PP_FLOATING_LITERAL
			|| t == PP_MOC_TRUE
			|| t == PP_MOC_FALSE
			|| t == PP_LPAREN);
	}

	int Preprocessor::evaluateCondition()
	{
		PP_Expression expression;
		expression.currentFilenames = currentFilenames;

		substituteUntilNewline(expression.symbols);

		return expression.value();
	}

	static std::string readOrMapFile(FILE* file)
	{
		std::ifstream ifs(file);
		std::string content;
		if (file)
		{
			if (ifs.is_open())
			{
				ifs.seekg(0, ifs.end);
				int length = ifs.tellg();
				ifs.seekg(0, ifs.beg);

				if (length == 0)
				{
					return std::string();
				}

				content.resize(length);

				if (content.capacity() == length)
				{
					ifs.read(&content.front(), length);
				}
			}
		}

		return std::move(content);
	}

	static void mergeStringLiterals(std::vector<Symbol> *_symbols)
	{
		std::vector<Symbol>& symbols = *_symbols;
		for (std::vector<Symbol>::iterator i = symbols.begin(); i != symbols.end(); ++i)
		{
			if (i->token == STRING_LITERAL)
			{
				std::vector<Symbol>::iterator mergeSymbol = i;
				int literalsLength = mergeSymbol->len;
				while (++i != symbols.end() && i->token == STRING_LITERAL)
					literalsLength += i->len - 2; // no quotes

				if (literalsLength != mergeSymbol->len)
				{
					std::string mergeSymbolOriginalLexem = mergeSymbol->unquotedLexem();
					std::string &mergeSymbolLexem = mergeSymbol->lex;
					mergeSymbolLexem.resize(0);
					mergeSymbolLexem.reserve(literalsLength);
					mergeSymbolLexem.push_back('"');
					mergeSymbolLexem.append(mergeSymbolOriginalLexem);
					for (std::vector<Symbol>::const_iterator j = mergeSymbol + 1; j != i; ++j)
						mergeSymbolLexem.append(j->lex.data() + j->from + 1, j->len - 2); // append j->unquotedLexem()
					mergeSymbolLexem.push_back('"');
					mergeSymbol->len = mergeSymbol->lex.length();
					mergeSymbol->from = 0;
					i = symbols.erase(mergeSymbol + 1, i);
				}
				if (i == symbols.end())
					break;
			}
		}
	}

	static std::string searchIncludePaths(const std::vector<Parser::IncludePath> &includepaths,
		const std::string &include)
	{
		std::filesystem::path fi;
		for (int j = 0; j < includepaths.size() && !std::filesystem::exists(fi); ++j)
		{
			const Parser::IncludePath &p = includepaths.at(j);
			if (p.isFrameworkPath)
			{
				const int slashPos = include.find('/');
				if (slashPos == -1)
					continue;
				fi = p.path + '/' + sub(include, 0, slashPos) + ".framework/Headers/";
				fi += sub(include, slashPos + 1);
				// fi.setFile(std::string::fromLocal8Bit(p.path + '/' + include.left(slashPos) + ".framework/Headers/"),
				// std::string::fromLocal8Bit(include.mid(slashPos + 1)));
			}
			else
			{
				fi = p.path + '/' + include;
			}
			// try again, maybe there's a file later in the include paths with the same name
			// (186067)
			if (std::filesystem::is_directory(fi))
			{
				fi.clear();
				continue;
			}
		}

		if (!std::filesystem::exists(fi) || std::filesystem::is_directory(fi))
			return std::string();

		return fi.string();
	}

	std::string Preprocessor::resolveInclude(const std::string &include, const std::string &relativeTo)
	{
		if (!relativeTo.empty())
		{
			std::filesystem::path fi;
			fi = relativeTo;
			fi /= include;
			if (std::filesystem::exists(fi) && std::filesystem::is_directory(fi))
				return fi.string();
		}

		auto it = nonlocalIncludePathResolutionCache.find(include);
		if (it == nonlocalIncludePathResolutionCache.end())
		{
			it = nonlocalIncludePathResolutionCache.insert_or_assign(it, include, searchIncludePaths(includes, include));
			//it = nonlocalIncludePathResolutionCache.insert(include, searchIncludePaths(includes, include));
		}
		return it->second;
	}

	void Preprocessor::preprocess(const std::string &filename, std::vector<Symbol> &preprocessed)
	{
		currentFilenames.push(filename);
		preprocessed.reserve(preprocessed.size() + symbols.size());
		while (hasNext())
		{
			Token token = next();

			switch (token)
			{
				case PP_INCLUDE:
					{
						int lineNum = symbol().lineNum;
						std::string include;
						bool local = false;
						if (test(PP_STRING_LITERAL))
						{
							// TODO
							local = lexem()[0] == '\"'; // same as '"'
							include = unquotedLexem();
						}
						else
							continue;
						until(PP_NEWLINE);

						include = resolveInclude(include, local ? filename : std::string());
						if (include.empty())
							continue;

						if (Preprocessor::preprocessedIncludes.find(include) != Preprocessor::preprocessedIncludes.end())
							continue;

						Preprocessor::preprocessedIncludes.insert(include);

						FILE* file = fopen(include.c_str(), "r");
						if (!file)
						{
							continue;
						}

						std::string input = readOrMapFile(file);

						fclose(file);
						if (input.empty())
							continue;

						std::vector<Symbol> saveSymbols = symbols;
						int saveIndex = index;

						// phase 1: get rid of backslash-newlines
						input = cleaned(input);

						// phase 2: tokenize for the preprocessor
						symbols = tokenize(input);
						input.clear();

						index = 0;

						// phase 3: preprocess conditions and substitute std::unordered_map<std::string, Macro>
						preprocessed.push_back(Symbol(0, MOC_INCLUDE_BEGIN, include));
						preprocess(include, preprocessed);
						preprocessed.push_back(Symbol(lineNum, MOC_INCLUDE_END, include));

						symbols = saveSymbols;
						index = saveIndex;
						continue;
					}
				case PP_DEFINE:
					{
						next();
						std::string name = lexem();
						if (name.empty() || !is_ident_start(name[0]))
							error();
						Macro macro;
						macro.isVariadic = false;
						if (test(LPAREN))
						{
							// we have a function macro
							macro.isFunction = true;
							parseDefineArguments(&macro);
						}
						else
						{
							macro.isFunction = false;
						}
						int start = index;
						until(PP_NEWLINE);
						macro.symbols.reserve(index - start - 1);

						// remove whitespace where there shouldn't be any:
						// Before and after the macro, after a # and around ##
						Token lastToken = HASH; // skip shitespace at the beginning
						for (int i = start; i < index - 1; ++i)
						{
							Token token = symbols.at(i).token;
							if (token == PP_WHITESPACE || token == WHITESPACE)
							{
								if (lastToken == PP_HASH || lastToken == HASH ||
									lastToken == PP_HASHHASH ||
									lastToken == PP_WHITESPACE || lastToken == WHITESPACE)
									continue;
							}
							else if (token == PP_HASHHASH)
							{
								if (!macro.symbols.empty() &&
									(lastToken == PP_WHITESPACE || lastToken == WHITESPACE))
									macro.symbols.pop_back();
							}
							macro.symbols.push_back(symbols.at(i));
							lastToken = token;
						}
						// remove trailing whitespace
						while (!macro.symbols.empty() &&
							(macro.symbols.back().token == PP_WHITESPACE || macro.symbols.back().token == WHITESPACE))
							macro.symbols.pop_back();

						if (!macro.symbols.empty())
						{
							if (macro.symbols.front().token == PP_HASHHASH ||
								macro.symbols.back().token == PP_HASHHASH)
							{
								error("'##' cannot appear at either end of a macro expansion");
							}
						}
						macros.insert_or_assign(name, macro);
						continue;
					}
				case PP_UNDEF:
					{
						next();
						std::string name = lexem();
						until(PP_NEWLINE);
						macros.erase(name);
						continue;
					}
				case PP_IDENTIFIER:
					{
						// substitute std::unordered_map<std::string, Macro>
						macroExpand(&preprocessed, this, symbols, index, symbol().lineNum, true);
						continue;
					}
				case PP_HASH:
					until(PP_NEWLINE);
					continue; // skip unknown preprocessor statement
				case PP_IFDEF:
				case PP_IFNDEF:
				case PP_IF:
					while (!evaluateCondition())
					{
						if (!skipBranch())
							break;
						if (test(PP_ELIF))
						{
						}
						else
						{
							until(PP_NEWLINE);
							break;
						}
					}
					continue;
				case PP_ELIF:
				case PP_ELSE:
					skipUntilEndif();
					Q_FALLTHROUGH();
				case PP_ENDIF:
					until(PP_NEWLINE);
					continue;
				case PP_NEWLINE:
					continue;
				case SIGNALS:
				case SLOTS:
					{
						Symbol sym = symbol();
						if (macros.find("QT_NO_KEYWORDS") != macros.end())
							sym.token = IDENTIFIER;
						else
							sym.token = (token == SIGNALS ? Q_SIGNALS_TOKEN : Q_SLOTS_TOKEN);
						preprocessed.push_back(sym);
					} continue;
				default:
					break;
			}
			preprocessed.push_back(symbol());
		}

		currentFilenames.pop();
	}

	std::vector<Symbol> Preprocessor::preprocessed(const std::string &filename, FILE*& file)
	{
		std::string input = readOrMapFile(file);

		if (input.empty())
			return symbols;

		// phase 1: get rid of backslash-newlines
		input = cleaned(input);

		// phase 2: tokenize for the preprocessor
		index = 0;
		symbols = tokenize(input);

#if 0
		for (int j = 0; j < symbols.size(); ++j)
			fprintf(stderr, "line %d: %s(%s)\n",
				symbols[j].lineNum,
				symbols[j].lexem().constData(),
				tokenTypeName(symbols[j].token));
#endif

		// phase 3: preprocess conditions and substitute std::unordered_map<std::string, Macro>
		std::vector<Symbol> result;
		// Preallocate some space to speed up the code below.
		// The magic value was found by logging the final size
		// and calculating an average when running moc over FOSS projects.
		std::ifstream ifs(file);
		ifs.seekg(0, ifs.end);
		int length = ifs.tellg();
		ifs.seekg(0, ifs.beg);
		result.reserve(length / 300000);
		preprocess(filename, result);
		mergeStringLiterals(&result);

#if 0
		for (int j = 0; j < result.size(); ++j)
			fprintf(stderr, "line %d: %s(%s)\n",
				result[j].lineNum,
				result[j].lexem().constData(),
				tokenTypeName(result[j].token));
#endif

		return result;
	}

	void Preprocessor::parseDefineArguments(Macro *m)
	{
		std::vector<Symbol> arguments;
		while (hasNext())
		{
			while (test(PP_WHITESPACE))
			{
			}
			Token t = next();
			if (t == PP_RPAREN)
				break;
			if (t != PP_IDENTIFIER)
			{
				std::string l = lexem();
				if (l == "...")
				{
					m->isVariadic = true;
					arguments.push_back(Symbol(symbol().lineNum, PP_IDENTIFIER, "__VA_ARGS__"));
					while (test(PP_WHITESPACE))
					{
					}
					if (!test(PP_RPAREN))
						error("missing ')' in macro argument list");
					break;
				}
				else if (!is_identifier(l.data(), l.length()))
				{
					error("Unexpected character in macro argument list.");
				}
			}

			Symbol arg = symbol();
			if (std::find(arguments.begin(), arguments.end(), arg) != arguments.end())
				error("Duplicate macro parameter.");
			arguments.push_back(symbol());

			while (test(PP_WHITESPACE))
			{
			}
			t = next();
			if (t == PP_RPAREN)
				break;
			if (t == PP_COMMA)
				continue;
			if (lexem() == "...")
			{
				//GCC extension:    #define FOO(x, y...) x(y)
				// The last argument was already parsed. Just mark the macro as variadic.
				m->isVariadic = true;
				while (test(PP_WHITESPACE))
				{
				}
				if (!test(PP_RPAREN))
					error("missing ')' in macro argument list");
				break;
			}
			error("Unexpected character in macro argument list.");
		}
		m->arguments = arguments;
		while (test(PP_WHITESPACE))
		{
		}
	}

	void Preprocessor::until(Token t)
	{
		while (hasNext() && next() != t)
			;
	}
}
