#ifndef GENERATOR_H
#define GENERATOR_H

#include "moc.h"

QT_BEGIN_NAMESPACE

class Generator
{
    FILE *out;
    ClassDef *cdef;
    QVector<uint> meta_data;
public:
    Generator(ClassDef *classDef, const QList<QByteArray> &metaTypes, FILE *outfile = 0);
    void generateCode();
    QMetaObject *generateMetaObject(bool ignoreProperties);
private:
    void generateClassInfos();
    void generateFunctions(QList<FunctionDef> &list, const char *functype, int type);
    void generateFunctionRevisions(QList<FunctionDef>& list, const char *functype);
    void generateEnums(int index);
    void generateProperties();
    void generateMetacall();
    void generateStaticMetacall();
    void generateSignal(FunctionDef *def, int index);

    // used by binary QMetaObject generator
    void _generateClassInfos();
    void _generateFunctions(QList<FunctionDef> &list, int type);
    void _generateEnums(int index);
    void _generateProperties();

    int strreg(const char *); // registers a string and returns its id
    QList<QByteArray> strings;
    QByteArray purestSuperClass;
    QList<QByteArray> metaTypes;
};

QT_END_NAMESPACE

#endif // GENERATOR_H
