#include <time.h>
#include <iostream>
#include "VideoPlayer.h"

using namespace std::chrono;
using namespace std;


int main(int argc, char **argv)
{
    freopen("/dev/null", "w", stderr);
    freopen("CON", "w", stdout);
    srand(time(NULL));

    if (!argv[1])
    {
        fprintf(stderr, "Veuillez specifier un chemin de fichier");
        return -1;
    }
    VideoPlayer p(argv[1]);
    p.loop();
    return 0;

}