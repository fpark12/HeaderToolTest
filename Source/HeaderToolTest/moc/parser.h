#ifndef PARSER_H
#define PARSER_H

#include <QStack>
#include "symbols.h"

QT_BEGIN_NAMESPACE

class Parser
{
public:
    Parser():index(0), displayWarnings(true), displayNotes(true) {}
    Symbols symbols;
    int index;
    bool displayWarnings;
    bool displayNotes;

    QStack<QByteArray> currentFilenames;

    inline bool hasNext() const { return (index < symbols.size()); }
    inline Token next() { if (index >= symbols.size()) return NOTOKEN; return symbols.at(index++).token; }
    bool test(Token);
    void next(Token);
    void next(Token, const char *msg);
    inline void prev() {--index;}
    inline Token lookup(int k = 1);
    inline const Symbol &symbol_lookup(int k = 1) { return symbols.at(index-1+k);}
    inline Token token() { return symbols.at(index-1).token;}
    inline QByteArray lexem() { return symbols.at(index-1).lexem();}
    inline QByteArray unquotedLexem() { return symbols.at(index-1).unquotedLexem();}
    inline const Symbol &symbol() { return symbols.at(index-1);}

    void error(int rollback);
    void error(const char *msg = 0);
    void warning(const char * = 0);
    void note(const char * = 0);

};

inline bool Parser::test(Token token)
{
    if (index < symbols.size() && symbols.at(index).token == token) {
        ++index;
        return true;
    }
    return false;
}

inline Token Parser::lookup(int k)
{
    const int l = index - 1 + k;
    return l < symbols.size() ? symbols.at(l).token : NOTOKEN;
}

inline void Parser::next(Token token)
{
    if (!test(token))
        error();
}

inline void Parser::next(Token token, const char *msg)
{
    if (!test(token))
        error(msg);
}

QT_END_NAMESPACE

#endif