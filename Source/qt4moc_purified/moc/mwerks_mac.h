#ifndef MWERKS_MAC_H
#define MWERKS_MAC_H

#include <QtGlobal>

namespace header_tool {

#ifdef Q_OS_MAC

#define macintosh

/*make moc a plugin*/
enum moc_status {
    moc_success = 1,
    moc_parse_error = 2,
    moc_no_qobject = 3,
    moc_not_time = 4,
    moc_no_source = 5,
    moc_general_error = 6
};

#endif

}

#endif // MWERKS_MAC_H
