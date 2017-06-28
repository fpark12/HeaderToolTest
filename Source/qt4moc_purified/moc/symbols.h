#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "token.h"
#include <string>
#include <unordered_map>
#include <vector>
//#include <QDebug>

namespace header_tool {

//#define USE_LEXEM_STORE

struct SubArray
{
    inline SubArray():from(0),len(-1){}
    inline SubArray(const std::vector<unsigned char> &a):array(a),from(0), len(a.size()){}
    //inline SubArray(const char *s):array(s),from(0) { len = array.size(); }
    inline SubArray(const std::vector<unsigned char> &a, int from, int len):array(a), from(from), len(len){}
    std::vector<unsigned char> array;
    int from, len;
    inline bool operator==(const SubArray &other) const {
        if (len != other.len)
            return false;
        for (int i = 0; i < len; ++i)
            if (array.at(from + i) != other.array.at(other.from + i))
                return false;
        return true;
    }
};

inline uint32 qHash(const SubArray &key)
{
    const uint8 *p = reinterpret_cast<const uint8 *>(key.array.data() + key.from);
    int n = key.len;
    uint32 h = 0;
    uint32 g;

    while (n--) {
        h = (h << 4) + *p++;
        if ((g = (h & 0xf0000000)) != 0)
            h ^= g >> 23;
        h &= ~g;
    }
    return h;
}


struct Symbol
{

#ifdef USE_LEXEM_STORE
    typedef std::unordered_map<SubArray, QHashDummyValue> LexemStore;
    static LexemStore lexemStore;

    inline Symbol() : lineNum(-1),token(NOTOKEN){}
    inline Symbol(int lineNum, Token token):
        lineNum(lineNum), token(token){}
    inline Symbol(int lineNum, Token token, const std::vector<unsigned char> &lexem):
        lineNum(lineNum), token(token),lex(lexem){}
    inline Symbol(int lineNum, Token token, const std::vector<unsigned char> &lexem, int from, int len):
        lineNum(lineNum), token(token){
        LexemStore::const_iterator it = lexemStore.constFind(SubArray(lexem, from, len));

        if (it != lexemStore.constEnd()) {
            lex = it.key().array;
        } else {
            lex = lexem.mid(from, len);
            lexemStore.insert(lex, QHashDummyValue());
        }
    }
    int lineNum;
    Token token;
    inline std::vector<unsigned char> unquotedLexem() const { return lex.mid(1, lex.length()-2); }
    inline std::vector<unsigned char> lexem() const { return lex; }
    inline operator std::vector<unsigned char>() const { return lex; }
    std::vector<unsigned char> lex;

#else

    inline Symbol() : lineNum(-1),token(NOTOKEN), from(0),len(-1) {}
    inline Symbol(int lineNum, Token token):
        lineNum(lineNum), token(token), from(0), len(-1) {}
    inline Symbol(int lineNum, Token token, const std::vector<unsigned char> &lexem):
        lineNum(lineNum), token(token), lex(lexem), from(0) { len = lex.size(); }
    inline Symbol(int lineNum, Token token, const std::vector<unsigned char> &lexem, int from, int len):
        lineNum(lineNum), token(token),lex(lexem),from(from), len(len){}
    int lineNum;
    Token token;
    inline std::vector<unsigned char> lexem() const { return lex.mid(from, len); }
    inline std::vector<unsigned char> unquotedLexem() const { return lex.mid(from+1, len-2); }
    inline operator std::vector<unsigned char>() const { return lex.mid(from, len); }
    inline operator SubArray() const { return SubArray(lex, from, len); }
    std::vector<unsigned char> lex;
    int from, len;

#endif
};
//Q_DECLARE_TYPEINFO(Symbol, Q_MOVABLE_TYPE);


typedef std::vector<Symbol> Symbols;

}

#endif // SYMBOLS_H
