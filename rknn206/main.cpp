

#include <iostream>
#include <iomanip>
#include <utility>


#include "WavRecognize/WavRecognize.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    WavRecognize wa;
    wa.Init(0,0);
    wa.GetWavSignalsThread();
    // wa.GetChunkThread();
    wa.ForwardThread();

    return 0;
}
