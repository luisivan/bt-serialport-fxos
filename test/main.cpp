#include <iostream>

#include "SerialComms/serial.h"

using namespace std;

int main()
{
    //const int rate = ;

    Serial *s = new Serial("/dev/serial", 1000000);

    unsigned char *i = "OLA K ASE";
    unsigned char *o;

    s->SendBuffer(i, sizeof(i)+1);

    //s->Close();
    while(s->Read(o, 16)) {
        cout << "hallo" << endl;
        cout << o << endl;
    }


    return 0;
}
