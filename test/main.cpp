#include <iostream>
#include <stdio.h>

#include "SerialComms/serial.h"

using namespace std;

int main()
{
    // socat pty,link=/dev/serial,raw stdout; chmod 777 /dev/serial

    Serial *s = new Serial("/dev/serial", 1000000);

    unsigned char *i = "OLA K ASE";

    s->SendBuffer(i, sizeof(i)+1);

    unsigned char *o;

    while(true) {
        s->Read(o, 16);
        printf(o);
        /*cout << "hallo" << endl;
        cout << o << endl;*/
    }

    s->Close();

    return 0;
}
