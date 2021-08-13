#include <time.h>
#include "VideoPlayer.h"

int main(int argc, char **argv)
{
    //freopen("/dev/null", "w", stderr);
    freopen("CON", "w", stdout);
    srand(time(NULL));

    std::cout << "entrer un chemin de dossier absolu" << std::endl;
    std::string s;
    std::cin >> s;

    VideoPlayer *p = VideoPlayer::construct(s);

    p->loop();

    return 0;
}