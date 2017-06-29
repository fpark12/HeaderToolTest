#include "parser.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

namespace header_tool {

#ifdef USE_LEXEM_STORE
Symbol::LexemStore Symbol::lexemStore;
#endif

static const char *error_msg = 0;

#ifdef Q_CC_MSVC
#define ErrorFormatString "%s(%d): "
#else
#define ErrorFormatString "%s:%d: "
#endif

void Parser::error(int rollback) {
    index -= rollback;
    error();
}
void Parser::error(const char *msg) {
    if (msg || error_msg)
        fprintf(stderr, ErrorFormatString "Error: %s\n",
                 currentFilenames.top().c_str(), symbol().lineNum, msg?msg:error_msg);
    else
        fprintf(stderr, ErrorFormatString "Parse error at \"%s\"\n",
                 currentFilenames.top().c_str(), symbol().lineNum, symbol().lexem().data());
    exit(EXIT_FAILURE);
}

void Parser::warning(const char *msg) {
    if (displayWarnings && msg)
        fprintf(stderr, ErrorFormatString "Warning: %s\n",
                currentFilenames.top().c_str(), std::max(0, index > 0 ? symbol().lineNum : 0), msg);
}

void Parser::note(const char *msg) {
    if (displayNotes && msg)
        fprintf(stderr, ErrorFormatString "Note: %s\n",
                currentFilenames.top().c_str(), std::max(0, index > 0 ? symbol().lineNum : 0), msg);
}

}
