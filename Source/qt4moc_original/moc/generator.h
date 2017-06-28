#ifndef GENERATOR_H
#define GENERATOR_H

#include "moc.h"

namespace header_tool {

class Generator
{
    FILE *out;
    ClassDef *cdef;
    std::vector<uint> meta_data;
public:
    Generator(ClassDef *classDef, const std::list<std::vector<unsigned char>> &metaTypes, FILE *outfile = 0);
    void generateCode();
    QMetaObject *generateMetaObject(bool ignoreProperties);
private:
    void generateClassInfos();
    void generateFunctions(std::list<FunctionDef> &list, const char *functype, int type);
    void generateFunctionRevisions(std::list<FunctionDef>& list, const char *functype);
    void generateEnums(int index);
    void generateProperties();
    void generateMetacall();
    void generateStaticMetacall();
    void generateSignal(FunctionDef *def, int index);

    // used by binary QMetaObject generator
    void _generateClassInfos();
    void _generateFunctions(std::list<FunctionDef> &list, int type);
    void _generateEnums(int index);
    void _generateProperties();

    int strreg(const char *); // registers a string and returns its id
    std::list<std::vector<unsigned char>> strings;
    std::vector<unsigned char> purestSuperClass;
    std::list<std::vector<unsigned char>> metaTypes;
};

}

#endif // GENERATOR_H
