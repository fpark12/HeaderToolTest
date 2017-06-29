#ifndef GENERATOR_H
#define GENERATOR_H

#include "moc.h"

namespace header_tool {

class Generator
{
    FILE *out;
    ClassDef *cdef;
    std::vector<uint32> meta_data;
public:
    Generator(ClassDef *classDef, const std::list<std::vector<uint8>> &metaTypes, const std::unordered_map<std::vector<uint8>, std::vector<uint8>> &knownQObjectClasses, const std::unordered_map<std::vector<uint8>, std::vector<uint8>> &knownGadgets, FILE *outfile = 0);
    void generateCode();
private:
    bool registerableMetaType(const std::vector<uint8> &propertyType);
    void registerClassInfoStrings();
    void generateClassInfos();
    void registerFunctionStrings(const std::vector<FunctionDef> &list);
    void generateFunctions(const std::vector<FunctionDef> &list, const char *functype, int type, int &paramsIndex);
    void generateFunctionRevisions(const std::vector<FunctionDef> &list, const char *functype);
    void generateFunctionParameters(const std::vector<FunctionDef> &list, const char *functype);
    void generateTypeInfo(const std::vector<uint8> &typeName, bool allowEmptyName = false);
    void registerEnumStrings();
    void generateEnums(int index);
    void registerPropertyStrings();
    void generateProperties();
    void generateMetacall();
    void generateStaticMetacall();
    void generateSignal(FunctionDef *def, int index);
    void generatePluginMetaData();
    std::multimap<std::vector<uint8>, int> automaticPropertyMetaTypesHelper();
    std::map<int, std::multimap<std::vector<uint8>, int> > methodsWithAutomaticTypesHelper(const std::vector<FunctionDef> &methodList);

    void strreg(const std::vector<uint8> &); // registers a string
    int stridx(const std::vector<uint8> &); // returns a string's id
    std::list<std::vector<uint8>> strings;
    std::vector<uint8> purestSuperClass;
    std::list<std::vector<uint8>> metaTypes;
    std::unordered_map<std::string, std::vector<uint8>> knownQObjectClasses;
    std::unordered_map<std::string, std::vector<uint8>> knownGadgets;
};

}

#endif // GENERATOR_H
