#include "token.h"

namespace header_tool {

#if defined(DEBUG_MOC)
const char *tokenTypeName(Token t)
{
    switch (t) {
#define CREATE_CASE(Name) case Name: return #Name;
    FOR_ALL_TOKENS(CREATE_CASE)
#undef CREATE_CASE
    }
    return "";
}
#endif

}
