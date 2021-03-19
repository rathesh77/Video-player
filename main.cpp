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

    cout<<"entrer un chemin de dossier absolu"<<endl;
    char **path;
    cin>>*path;

    VideoPlayer *p = new VideoPlayer(*path);
    p->loop();
    delete p;

    return 0;
}