/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
    QMultiMap<std::vector<uint8>, int> automaticPropertyMetaTypesHelper();
    QMap<int, QMultiMap<std::vector<uint8>, int> > methodsWithAutomaticTypesHelper(const std::vector<FunctionDef> &methodList);

    void strreg(const std::vector<uint8> &); // registers a string
    int stridx(const std::vector<uint8> &); // returns a string's id
    std::list<std::vector<uint8>> strings;
    std::vector<uint8> purestSuperClass;
    std::list<std::vector<uint8>> metaTypes;
    std::unordered_map<std::vector<uint8>, std::vector<uint8>> knownQObjectClasses;
    std::unordered_map<std::vector<uint8>, std::vector<uint8>> knownGadgets;
};

}

#endif // GENERATOR_H
