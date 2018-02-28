#include "cargv_version.h"
#include <stdio.h>


int main(int argc, const char **argv)
{
    printf("%s: %s\n", CARGV_NAME, CARGV_VERSION_STRING);
    return 0;
}
