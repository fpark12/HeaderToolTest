#ifndef TOKEN_H
#define TOKEN_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

enum Token {
    NOTOKEN,
    IDENTIFIER,
    INTEGER_LITERAL,
    CHARACTER_LITERAL,
    STRING_LITERAL,
    BOOLEAN_LITERAL,
    HEADER_NAME,
    LANGLE,
    RANGLE,
    LPAREN,
    RPAREN,
    ELIPSIS,
    LBRACK,
    RBRACK,
    LBRACE,
    RBRACE,
    EQ,
    SCOPE,
    SEMIC,
    COLON,
    DOTSTAR,
    QUESTION,
    DOT,
    DYNAMIC_CAST,
    STATIC_CAST,
    REINTERPRET_CAST,
    CONST_CAST,
    TYPEID,
    THIS,
    TEMPLATE,
    THROW,
    TRY,
    CATCH,
    TYPEDEF,
    FRIEND,
    CLASS,
    NAMESPACE,
    ENUM,
    STRUCT,
    UNION,
    VIRTUAL,
    PRIVATE,
    PROTECTED,
    PUBLIC,
    EXPORT,
    AUTO,
    REGISTER,
    EXTERN,
    MUTABLE,
    ASM,
    USING,
    INLINE,
    EXPLICIT,
    STATIC,
    CONST,
    VOLATILE,
    OPERATOR,
    SIZEOF,
    NEW,
    DELETE,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,
    HAT,
    AND,
    OR,
    TILDE,
    NOT,
    PLUS_EQ,
    MINUS_EQ,
    STAR_EQ,
    SLASH_EQ,
    PERCENT_EQ,
    HAT_EQ,
    AND_EQ,
    OR_EQ,
    LTLT,
    GTGT,
    GTGT_EQ,
    LTLT_EQ,
    EQEQ,
    NE,
    LE,
    GE,
    ANDAND,
    OROR,
    INCR,
    DECR,
    COMMA,
    ARROW_STAR,
    ARROW,
    CHAR,
    WCHAR,
    BOOL,
    SHORT,
    INT,
    LONG,
    SIGNED,
    UNSIGNED,
    FLOAT,
    DOUBLE,
    VOID,
    CASE,
    DEFAULT,
    IF,
    ELSE,
    SWITCH,
    WHILE,
    DO,
    FOR,
    BREAK,
    CONTINUE,
    GOTO,
    SIGNALS,
    SLOTS,
    RETURN,
    Q_META_TOKEN_BEGIN,
    Q_OBJECT_TOKEN = Q_META_TOKEN_BEGIN,
    Q_GADGET_TOKEN,
    Q_PROPERTY_TOKEN,
    Q_ENUMS_TOKEN,
    Q_FLAGS_TOKEN,
    Q_DECLARE_FLAGS_TOKEN,
    Q_DECLARE_INTERFACE_TOKEN,
    Q_DECLARE_METATYPE_TOKEN,
    Q_CLASSINFO_TOKEN,
    Q_INTERFACES_TOKEN,
    Q_SIGNALS_TOKEN,
    Q_SLOTS_TOKEN,
    Q_SIGNAL_TOKEN,
    Q_SLOT_TOKEN,
    Q_PRIVATE_SLOT_TOKEN,
    Q_MOC_COMPAT_TOKEN,
    Q_QT3_SUPPORT_TOKEN,
    Q_INVOKABLE_TOKEN,
    Q_SCRIPTABLE_TOKEN,
    Q_PRIVATE_PROPERTY_TOKEN,
    Q_REVISION_TOKEN,
    Q_META_TOKEN_END,
    SPECIAL_TREATMENT_MARK = Q_META_TOKEN_END,
    MOC_INCLUDE_BEGIN,
    MOC_INCLUDE_END,
    CPP_COMMENT,
    C_COMMENT,
    FLOATING_LITERAL,
    HASH,
    QUOTE,
    SINGLEQUOTE,
    LANGLE_SCOPE,
    DIGIT,
    CHARACTER,
    NEWLINE,
    WHITESPACE,
    BACKSLASH,
    INCOMPLETE,

    PP_DEFINE,
    PP_UNDEF,
    PP_IF,
    PP_IFDEF,
    PP_IFNDEF,
    PP_ELIF,
    PP_ELSE,
    PP_ENDIF,
    PP_INCLUDE,
    PP_HASHHASH,
    PP_HASH,
    PP_DEFINED,
    PP_INCOMPLETE,

    PP_MOC_TRUE,
    PP_MOC_FALSE,

    PP_NOTOKEN = NOTOKEN,
    PP_IDENTIFIER = IDENTIFIER,
    PP_INTEGER_LITERAL = INTEGER_LITERAL,
    PP_CHARACTER_LITERAL = CHARACTER_LITERAL,
    PP_STRING_LITERAL = STRING_LITERAL,
    PP_LANGLE = LANGLE,
    PP_RANGLE = RANGLE,
    PP_LPAREN = LPAREN,
    PP_RPAREN = RPAREN,
    PP_COMMA = COMMA,
    PP_PLUS = PLUS,
    PP_MINUS = MINUS,
    PP_STAR = STAR,
    PP_SLASH = SLASH,
    PP_PERCENT = PERCENT,
    PP_HAT = HAT,
    PP_AND = AND,
    PP_OR = OR,
    PP_TILDE = TILDE,
    PP_NOT = NOT,
    PP_LTLT = LTLT,
    PP_GTGT = GTGT,
    PP_EQEQ = EQEQ,
    PP_NE = NE,
    PP_LE = LE,
    PP_GE = GE,
    PP_ANDAND = ANDAND,
    PP_OROR = OROR,
    PP_QUESTION = QUESTION,
    PP_COLON = COLON,
    PP_FLOATING_LITERAL = FLOATING_LITERAL,
    PP_QUOTE = QUOTE,
    PP_SINGLEQUOTE = SINGLEQUOTE,
    PP_DIGIT = DIGIT,
    PP_CHARACTER = CHARACTER,
    PP_WHITESPACE = WHITESPACE,
    PP_NEWLINE = NEWLINE,
    PP_CPP_COMMENT = CPP_COMMENT,
    PP_C_COMMENT = C_COMMENT,
    PP_BACKSLASH = BACKSLASH
};

// for debugging only
#if defined(DEBUG_MOC)
const char *tokenTypeName(Token t);
#endif

typedef Token PP_Token;

QT_END_NAMESPACE

#endif // TOKEN_H
