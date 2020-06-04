/* cflag.c
 * Copyright (C) 2020 Adrian Perez de Castro <aperez@igalia.com>
 *
 * Distributed under terms of the MIT license.
 */

#include "cflag.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>


static inline bool
is_long_flag(const char *s)
{
    return (s[0] == '-' && s[1] == '-' && s[2] != '\0');
}

static inline bool
is_short_flag(const char *s)
{
    return (s[0] == '-' && s[1] != '\0' && s[2] == '\0');
}

static inline bool
is_negated(const char *s)
{
    return strncmp("--no-", s, 5) == 0;
}

static const CFlag*
find_short(const CFlag specs[],
           int         letter)
{
    for (unsigned i = 0; specs[i].name != NULL || specs[i].letter != '\0'; ++i) {
        if (specs[i].letter == '\0')
            continue;
        if (specs[i].letter == letter)
            return &specs[i];
    }
    return NULL;
}

static const CFlag*
find_long(const CFlag specs[],
          const char *name)
{
    for (unsigned i = 0; specs[i].name != NULL || specs[i].letter != '\0'; ++i) {
        if (!specs[i].name)
            continue;
        if (strcmp(specs[i].name, name) == 0)
            return &specs[i];
    }
    return NULL;
}

static inline bool
needs_arg(const CFlag *spec)
{
    return (*spec->func)(NULL, NULL) == CFLAG_NEEDS_ARG;
}

void
cflag_usage(const CFlag specs[],
            const char *progname,
            const char *usage,
            FILE       *out)
{
    assert(specs);
    assert(progname);
    assert(usage);

    if (!out)
        out = stderr;
    {
        const char *slash = strrchr(progname, '/');
        if (slash)
            progname = slash + 1;
    }

    fprintf(out, "Usage: %s %s\n", progname, usage);
    fprintf(out, "Command line options:\n\n");

    for (unsigned i = 0;; ++i) {
        const CFlag *spec = &specs[i];

        const bool has_letter = spec->letter != '\0';
        const bool has_name = spec->name != NULL;

        if (!(has_name || has_letter))
            break;

        if (has_letter && has_name)
            fprintf(out, "-%c, --%s", spec->letter, spec->name);
        else if (has_name)
            fprintf(out, "--%s", spec->name);
        else
            fprintf(out, "-%c", spec->letter);

        if (needs_arg(spec))
            fprintf(out, " <ARG>");

        fprintf(out, "\n   %s\n\n", spec->help);
    }
}

int
cflag_parse(const CFlag specs[],
            int        *pargc,
            char     ***pargv)
{
    assert(specs);
    assert(pargc);
    assert(pargv);

    int argc = *pargc;
    char **argv = *pargv;

    for (; argc > 0; --argc, ++argv) {
        const char *arg = *argv;

        bool negated = false;
        const CFlag *spec;
        if (is_short_flag(arg)) {
            if (arg[1] == '-') /* -- stop processing command line flags */
                break;
            spec = find_short(specs, arg[1]);
        } else if (is_long_flag(arg)) {
            spec = find_long(specs, &arg[2]);
            if (!spec && is_negated(arg)) {
                const CFlag *negspec = find_long(specs, &arg[5]);
                if (negspec->func == cflag_bool) {
                    spec = negspec;
                    negated = true;
                }
            }
        } else {
            *pargc = argc; *pargv = argv;
            return CFLAG_OK;
        }

        if (!spec) {
            *pargc = argc; *pargv = argv;
            return CFLAG_UNDEFINED;
        }

        arg = NULL;
        if (needs_arg(spec)) {
            if (argc == 1) {
                *pargc = argc; *pargv = argv;
                return CFLAG_NEEDS_ARG;
            }
            arg = *(++argv);
            --argc;
        }

        const CFlagStatus status = (*spec->func)(spec, arg);
        if (status != CFLAG_OK) {
            *pargc = argc; *pargv = argv;
            return status;
        }

        /*
         * XXX: This fixup here is ugly, but avoids needing to pass
         *      additional parameters to cflag_<type> functions.
         */
        if (spec->func == cflag_bool && negated)
            *((bool*) spec->data) = false;
    }

    *pargc = argc; *pargv = argv;
    return CFLAG_OK;
}

const char*
cflag_status_name(CFlagStatus value)
{
    switch (value) {
        case CFLAG_OK: return "success";
        case CFLAG_SHOW_HELP: return "help requested";
        case CFLAG_UNDEFINED: return "no such option";
        case CFLAG_BAD_FORMAT: return "argument has invalid format";
        case CFLAG_NEEDS_ARG: return "missing argument";
    }
    assert(!"Unreachable");
    abort();
}

const char*
cflag_apply(const CFlag specs[],
            const char *syntax,
            int        *pargc,
            char     ***pargv)
{
    assert(specs);
    assert(syntax);
    assert(pargc);
    assert(pargv);

    int argc = *pargc;
    char **argv = *pargv;

    const char *argv0 = *argv++; argc--;
    {
        const char *slash = strrchr(argv0, '/');
        if (slash) argv0 = slash + 1;
    }

    const CFlagStatus status = cflag_parse(specs, &argc, &argv);
    switch (status) {
        case CFLAG_SHOW_HELP:
            cflag_usage(specs, argv0, syntax, stdout);
            exit(EXIT_SUCCESS);
        case CFLAG_OK:
            *pargc = argc;
            *pargv = argv;
            return argv0;
        default:
            break;
    }

    fprintf(stderr, "%s: %s: '%s'\n", argv0, cflag_status_name(status), *argv);
    exit(EXIT_FAILURE);
}

CFlagStatus
cflag_bool(const CFlag *spec,
           const char  *arg)
{
    (void) arg;

    if (!spec)
        return CFLAG_OK;

    *((bool*) spec->data) = true;
    return CFLAG_OK;
}

CFlagStatus
cflag_int(const CFlag *spec,
          const char  *arg)
{
    if (!spec)
        return CFLAG_NEEDS_ARG;

    return (sscanf(arg, "%d", (int*) spec->data) == 1) ? CFLAG_OK : CFLAG_BAD_FORMAT;
}

CFlagStatus
cflag_uint(const CFlag *spec,
           const char  *arg)
{
    if (!spec)
        return CFLAG_NEEDS_ARG;

    return (sscanf(arg, "%u", (unsigned*) spec->data) == 1) ? CFLAG_OK : CFLAG_BAD_FORMAT;
}

CFlagStatus
cflag_string(const CFlag *spec,
             const char  *arg)
{
    if (!spec)
        return CFLAG_NEEDS_ARG;

    *((const char**) spec->data) = arg;
    return CFLAG_OK;
}

CFlagStatus
cflag_bytes(const CFlag *spec, const char *arg)
{
    if (!spec)
        return CFLAG_NEEDS_ARG;

    char *endpos;
    unsigned long long v = strtoull(arg, &endpos, 0);
    if (v == ULLONG_MAX && errno == ERANGE)
        return CFLAG_BAD_FORMAT;

    if (endpos) {
        switch (*endpos) {
            case 'g': case 'G': v *= 1024 * 1024 * 1024; break; /* gigabytes */
            case 'm': case 'M': v *= 1024 * 1024;        break; /* megabytes */
            case 'k': case 'K': v *= 1024;               break; /* kilobytes */
            case 'b': case 'B': case '\0':               break; /* bytes     */
        }
    }

    *((size_t*) spec->data) = v;
    return CFLAG_OK;
}

CFlagStatus
cflag_help(const CFlag *spec,
           const char  *arg)
{
    (void) spec;
    (void) arg;

    return CFLAG_SHOW_HELP;
}
