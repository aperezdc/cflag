/*
 * cflag.h
 * Copyright (C) 2020 Adrian Perez de Castro <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef CFLAG_H
#define CFLAG_H

#include <stdio.h>

typedef enum {
    CFLAG_TYPE_BOOL = 0,
    CFLAG_TYPE_INT,
    CFLAG_TYPE_STRING,
    CFLAG_TYPE_CUSTOM,
    CFLAG_TYPE_HELP,
} CFlagType;

typedef enum {
    CFLAG_OK = 0,
    CFLAG_SHOW_HELP,
    CFLAG_UNDEFINED,
    CFLAG_BAD_FORMAT,
    CFLAG_NEEDS_ARG,
} CFlagStatus;

typedef struct _CFlag CFlag;

typedef CFlagStatus (*CFlagFunc) (const CFlag*, const char *arg);

struct _CFlag {
    CFlagFunc   func;
    const char *name;
    int         letter;
    void       *data;
    const char *help;
};


#define CFLAG(_t, _name, _letter, _data, _help) \
    ((CFlag) {                                  \
        .func = cflag_ ## _t,                   \
        .name = (_name),                        \
        .letter = (_letter),                    \
        .data = (_data),                        \
        .help = (_help),                        \
    })
#define CFLAG_HELP \
    CFLAG(help, "help", 'h', NULL, "Prints command line usage help.")
#define CFLAG_END \
    { NULL, }

CFlagStatus cflag_bool   (const CFlag*, const char*);
CFlagStatus cflag_int    (const CFlag*, const char*);
CFlagStatus cflag_uint   (const CFlag*, const char*);
CFlagStatus cflag_string (const CFlag*, const char*);
CFlagStatus cflag_bytes  (const CFlag*, const char*);
CFlagStatus cflag_help   (const CFlag*, const char*);

void cflag_usage(const CFlag specs[],
                 const char *progname,
                 const char *syntax,
                 FILE       *out);

int  cflag_parse(const CFlag specs[],
                 int        *pargc,
                 char     ***pargv);

const char* cflag_apply(const CFlag specs[],
                        const char *syntax,
                        int        *pargc,
                        char     ***pargv);

const char* cflag_status_name(CFlagStatus value);

#endif /* !CFLAG_H */
