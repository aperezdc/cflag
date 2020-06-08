cflag
=====

Installation
------------

With [clib](https://github.com/clibs/clib):

```sh
clib install aperezdc/cflag
```

Example
-------

```c
#include "cflag.h"

int
main(int argc, char **argv)
{
    int requests = 5000;
    int concurrency = 10;
    bool verbose = false;
    const char *url = "https://perezdecastro.org";

    static const struct cflag options[] = {
        CFLAG(int, "requests", 'r', &requests,
            "Number of total requests"),
        CFLAG(int, "concurrency", 0 /* no short option */, &concurrency,
            "Number of concurrent requests"),
        CFLAG(bool, NULL /* no long option */, 'v', &verbose,
            "Verbosely show progress"),
        CFLAG(string, "url", 'U', &url,
            "Target URL"),
        CFLAG_HELP,
        CFLAG_END
    };

    cflag_apply("[options] --url URL", &argc, &argv);

    return EXIT_SUCCESS;
}
```
