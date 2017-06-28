#ifndef MOC_H
#define MOC_H

#include "parser.h"
#include <QStringList>
#include <map>
#include <tuple>
#include <stdio.h>
#include <ctype.h>

namespace header_tool {

struct QMetaObject;

struct Type
{
    enum ReferenceType { NoReference, Reference, RValueReference, Pointer };

    inline Type() : isVolatile(false), isScoped(false), firstToken(NOTOKEN), referenceType(NoReference) {}
    inline explicit Type(const std::vector<unsigned char> &_name) : name(_name), isVolatile(false), isScoped(false), firstToken(NOTOKEN), referenceType(NoReference) {}
    std::vector<unsigned char> name;
    uint isVolatile : 1;
    uint isScoped : 1;
    Token firstToken;
    ReferenceType referenceType;
};

struct EnumDef
{
    std::vector<unsigned char> name;
    std::list<std::vector<unsigned char>> values;
};

struct ArgumentDef
{
    ArgumentDef() : isDefault(false) {}
    Type type;
    std::vector<unsigned char> rightType, normalizedType, name;
    std::vector<unsigned char> typeNameForCast; // type name to be used in cast from void * in metacall
    bool isDefault;
};

struct FunctionDef
{
    FunctionDef(): returnTypeIsVolatile(false), access(Private), isConst(false), isVirtual(false), isStatic(false),
                   inlineCode(false), wasCloned(false), isCompat(false), isInvokable(false),
                   isScriptable(false), isSlot(false), isSignal(false),
                   isConstructor(false), isDestructor(false), isAbstract(false), revision(0) {}
    Type type;
    std::vector<unsigned char> normalizedType;
    std::vector<unsigned char> tag;
    std::vector<unsigned char> name;
    bool returnTypeIsVolatile;

    std::list<ArgumentDef> arguments;

    enum Access { Private, Protected, Public };
    Access access;
    bool isConst;
    bool isVirtual;
    bool isStatic;
    bool inlineCode;
    bool wasCloned;

    std::vector<unsigned char> inPrivateClass;
    bool isCompat;
    bool isInvokable;
    bool isScriptable;
    bool isSlot;
    bool isSignal;
    bool isConstructor;
    bool isDestructor;
    bool isAbstract;

    int revision;
};

struct PropertyDef
{
    PropertyDef():notifyId(-1), constant(false), final(false), gspec(ValueSpec), revision(0){}
    std::vector<unsigned char> name, type, read, write, reset, designable, scriptable, editable, stored, user, notify, inPrivateClass;
    int notifyId;
    bool constant;
    bool final;
    enum Specification  { ValueSpec, ReferenceSpec, PointerSpec };
    Specification gspec;
    bool stdCppSet() const {
        std::vector<unsigned char> s("set");
        s += toupper(name[0]);
        s += name.mid(1);
        return (s == write);
    }
    int revision;
};


struct ClassInfoDef
{
    std::vector<unsigned char> name;
    std::vector<unsigned char> value;
};

struct ClassDef {
    ClassDef():
        hasQObject(false), hasQGadget(false), notifyableProperties(0)
        , revisionedMethods(0), revisionedProperties(0), begin(0), end(0){}
    std::vector<unsigned char> classname;
    std::vector<unsigned char> qualified;
    std::list<std::tuple<std::vector<unsigned char>, FunctionDef::Access> > superclassList;

    struct Interface
    {
        inline explicit Interface(const std::vector<unsigned char> &_className)
            : className(_className) {}
        std::vector<unsigned char> className;
        std::vector<unsigned char> interfaceId;
    };
    std::list<std::list<Interface> >interfaceList;

    bool hasQObject;
    bool hasQGadget;

    std::list<FunctionDef> constructorList;
    std::list<FunctionDef> signalList, slotList, methodList, publicList;
    int notifyableProperties;
    std::list<PropertyDef> propertyList;
    std::list<ClassInfoDef> classInfoList;
    std::map<std::vector<unsigned char>, bool> enumDeclarations;
    std::list<EnumDef> enumList;
    std::map<std::vector<unsigned char>, std::vector<unsigned char>> flagAliases;
    int revisionedMethods;
    int revisionedProperties;

    int begin;
    int end;
};

struct NamespaceDef {
    std::vector<unsigned char> name;
    int begin;
    int end;
};

class Moc : public Parser
{
public:
    Moc()
        : noInclude(false), generatedCode(false), mustIncludeQMetaTypeH(false)
        {}

    std::vector<unsigned char> filename;

    bool noInclude;
    bool generatedCode;
    bool mustIncludeQMetaTypeH;
    std::vector<unsigned char> includePath;
    std::list<std::vector<unsigned char>> includeFiles;
    std::list<ClassDef> classList;
    std::map<std::vector<unsigned char>, std::vector<unsigned char>> interface2IdMap;
    std::list<std::vector<unsigned char>> metaTypes;
    QSet<std::vector<unsigned char>> knownQObjectClasses;

    void parse();
    void generate(FILE *out);
    std::list<QMetaObject*> generate(bool ignoreProperties);

    bool parseClassHead(ClassDef *def);
    inline bool inClass(const ClassDef *def) const {
        return index > def->begin && index < def->end - 1;
    }

    inline bool inNamespace(const NamespaceDef *def) const {
        return index > def->begin && index < def->end - 1;
    }

    Type parseType();

    bool parseEnum(EnumDef *def);

    bool parseFunction(FunctionDef *def, bool inMacro = false);
    bool parseMaybeFunction(const ClassDef *cdef, FunctionDef *def);

    void parseSlots(ClassDef *def, FunctionDef::Access access);
    void parseSignals(ClassDef *def);
    void parseProperty(ClassDef *def);
    void createPropertyDef(PropertyDef &def);
    void parseEnumOrFlag(ClassDef *def, bool isFlag);
    void parseFlag(ClassDef *def);
    void parseClassInfo(ClassDef *def);
    void parseInterfaces(ClassDef *def);
    void parseDeclareInterface();
    void parseDeclareMetatype();
    void parseSlotInPrivate(ClassDef *def, FunctionDef::Access access);
    void parsePrivateProperty(ClassDef *def);

    void parseFunctionArguments(FunctionDef *def);

    std::vector<unsigned char> lexemUntil(Token);
    bool until(Token);

    // test for Q_INVOCABLE, Q_SCRIPTABLE, etc. and set the flags
    // in FunctionDef accordingly
    bool testFunctionAttribute(FunctionDef *def);
    bool testFunctionAttribute(Token tok, FunctionDef *def);
    bool testFunctionRevision(FunctionDef *def);

    void checkSuperClasses(ClassDef *def);
    void checkProperties(ClassDef* cdef);
};

inline std::vector<unsigned char> noRef(const std::vector<unsigned char> &type)
{
    if (type.endsWith('&')) {
        if (type.endsWith("&&"))
            return type.left(type.length()-2);
        return type.left(type.length()-1);
    }
    return type;
}

}

#endif // MOC_H
