#include <time.h>
#include <iostream>
#include "VideoPlayer.h"

int main(int argc, char **argv)
{
    freopen("/dev/null", "w", stderr);
    freopen("CON", "w", stdout);
    srand(time(NULL));

    std::cout << "entrer un chemin de dossier absolu" << std::endl;
    char **path;
    std::cin >> *path;

    VideoPlayer *p = new VideoPlayer(*path);
    p->loop();
    delete p;

    return 0;
}