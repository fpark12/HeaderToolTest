#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "token.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <stack>
//#include <qdebug.h>

namespace header_tool
{

	//#define USE_LEXEM_STORE

	struct SubArray
	{
		inline SubArray() :from(0), len(-1)
		{}
		inline SubArray(const std::vector<uint8> &a) : array(a), from(0), len(a.size())
		{}
		//inline SubArray(const char *s):array(s),from(0) { len = array.size(); }
		inline SubArray(const std::vector<uint8> &a, int from, int len) : array(a), from(from), len(len)
		{}
		std::vector<uint8> array;
		int from, len;
		inline bool operator==(const SubArray &other) const
		{
			if (len != other.len)
				return false;
			for (int i = 0; i < len; ++i)
				if (array.at(from + i) != other.array.at(other.from + i))
					return false;
			return true;
		}
	};

	static uint32 hash(const uint8 *p, int n)
	{
		uint32 h = 0;

		while (n--)
		{
			h = (h << 4) + *p++;
			h ^= (h & 0xf0000000) >> 23;
			h &= 0x0fffffff;
		}
		return h;
	}

	/*
#define QChar uint8

	static uint32 hash(const QChar *p, int n)
	{
		uint32 h = 0;

		while (n--)
		{
			h = (h << 4) + (*p++).unicode();
			h ^= (h & 0xf0000000) >> 23;
			h &= 0x0fffffff;
		}
		return h;
	}
	*/

	inline uint32 qHash(const SubArray &key)
	{
		return hash(key.array.data() + key.from, key.len);
		//return qHash( QLatin1String( key.array.data() + key.from, key.len ) );
	}

	std::vector<uint8> sub(const std::vector<uint8>& vec, int pos, int len)
	{
		if (pos >= vec.size())
		{
			return std::vector<uint8>();
		}

		if (len < 0)
		{
			len = vec.size() - pos;
		}

		if (pos < 0)
		{
			len += pos;
			pos = 0;
		}

		if (len + pos > vec.size())
		{
			len = vec.size() - pos;
		}
		if (pos == 0 && len == vec.size())
		{
			return vec;
		}
		return std::vector<uint8>(vec.data() + pos, vec.data() + pos + len);
	}


	struct Symbol
	{

#ifdef USE_LEXEM_STORE
		typedef std::unordered_map<SubArray, std::unordered_mapDummyValue> LexemStore;
		static LexemStore lexemStore;

		inline Symbol() : lineNum(-1), token(NOTOKEN)
		{}
		inline Symbol(int lineNum, Token token) :
			lineNum(lineNum), token(token)
		{}
		inline Symbol(int lineNum, Token token, const std::vector<uint8> &lexem) :
			lineNum(lineNum), token(token), lex(lexem)
		{}
		inline Symbol(int lineNum, Token token, const std::vector<uint8> &lexem, int from, int len) :
			lineNum(lineNum), token(token)
		{
			LexemStore::const_iterator it = lexemStore.constFind(SubArray(lexem, from, len));

			if (it != lexemStore.constEnd())
			{
				lex = it.key().array;
			}
			else
			{
				lex = lexem.mid(from, len);
				lexemStore.insert(lex, std::unordered_mapDummyValue());
			}
		}
		int lineNum;
		Token token;
		inline std::vector<uint8> unquotedLexem() const
		{
			return lex.mid(1, lex.length() - 2);
		}
		inline std::vector<uint8> lexem() const
		{
			return lex;
		}
		inline operator std::vector<uint8>() const
		{
			return lex;
		}
		std::vector<uint8> lex;

#else

		inline Symbol() : lineNum(-1), token(NOTOKEN), from(0), len(-1)
		{}
		inline Symbol(int lineNum, Token token) :
			lineNum(lineNum), token(token), from(0), len(-1)
		{}
		inline Symbol(int lineNum, Token token, const std::vector<uint8> &lexem) :
			lineNum(lineNum), token(token), lex(lexem), from(0)
		{
			len = lex.size();
		}
		inline Symbol(int lineNum, Token token, const std::vector<uint8> &lexem, int from, int len) :
			lineNum(lineNum), token(token), lex(lexem), from(from), len(len)
		{}
		int lineNum;
		Token token;
		inline std::vector<uint8> lexem() const
		{
			return sub(lex, from, len);
			//return lex.mid(from, len);
		}
		inline std::vector<uint8> unquotedLexem() const
		{
			return sub(lex, from + 1, len - 2);
			//return lex.mid(from + 1, len - 2);
		}
		inline operator SubArray() const
		{
			return SubArray(lex, from, len);
		}
		bool operator==(const Symbol& o) const
		{
			return SubArray(lex, from, len) == SubArray(o.lex, o.from, o.len);
		}
		std::vector<uint8> lex;
		int from, len;

#endif
	};
	//Q_DECLARE_TYPEINFO(Symbol, Q_MOVABLE_TYPE);

	typedef std::vector<Symbol> Symbols;

	struct SafeSymbols
	{
		Symbols symbols;
		std::vector<uint8> expandedMacro;
		std::set<std::vector<uint8>> excludedSymbols;
		int index;
	};
	//Q_DECLARE_TYPEINFO(SafeSymbols, Q_MOVABLE_TYPE);

	class SymbolStack : public std::stack<SafeSymbols>
	{
	public:
		inline bool hasNext()
		{
			while (!empty() && top().index >= top().symbols.size())
			{
				pop();
			}

			return !empty();
		}
		inline Token next()
		{
			while (!empty() && top().index >= top().symbols.size())
			{
				pop();
			}

			if (empty())
			{
				return NOTOKEN;
			}

			return top().symbols.at(top().index++).token;
		}
		bool test(Token);
		inline const Symbol &symbol() const
		{
			return top().symbols.at(top().index - 1);
		}
		inline Token token()
		{
			return symbol().token;
		}
		inline std::vector<uint8> lexem() const
		{
			return symbol().lexem();
		}
		inline std::vector<uint8> unquotedLexem()
		{
			return symbol().unquotedLexem();
		}

		bool dontReplaceSymbol(const std::vector<uint8> &name);
		std::set<std::vector<uint8>> excludeSymbols();
	};

	inline bool SymbolStack::test(Token token)
	{
		int stackPos = size() - 1;
		while (stackPos >= 0 && this->c.at(stackPos).index >= this->c.at(stackPos).symbols.size())
			--stackPos;
		if (stackPos < 0)
			return false;
		if (this->c.at(stackPos).symbols.at(this->c.at(stackPos).index).token == token)
		{
			next();
			return true;
		}
		return false;
	}

	inline bool SymbolStack::dontReplaceSymbol(const std::vector<uint8> &name)
	{
		for (int i = 0; i < size(); ++i)
		{
			if (name == this->c.at(i).expandedMacro || (this->c.at(i).excludedSymbols.find(name) != this->c.at(i).excludedSymbols.end()))
			{
				return true;
			}
		}
		return false;
	}

	inline std::set<std::vector<uint8>> SymbolStack::excludeSymbols()
	{
		std::set<std::vector<uint8>> set;
		for (int i = 0; i < size(); ++i)
		{
			set.insert(this->c.at(i).expandedMacro);
			set.insert(this->c.at(i).excludedSymbols.begin(), this->c.at(i).excludedSymbols.end());

			//set << this->c.at(i).expandedMacro;
			//set += this->c.at(i).excludedSymbols;
		}
		return set;
	}

}

#endif // SYMBOLS_H
