#include <iostream>
#include "http.h"

int main()
{
    HTTPManager manager;
    HTTPRequest req1("http://localhost:9090");
    HTTPRequest req2("http://localhost:9091");
    HTTPRequest req3("http://localhost:9092");
    HTTPRequest req4("http://localhost:9093");

    while (1)
    {

    manager.addRequest(&req1);
    //sleep(4);
    //manager.addRequest(&req2);
    //sleep(5);
    //manager.addRequest(&req3);
    //sleep(9);
    //manager.addRequest(&req4);
    //sleep(4);
    //
 
    sleep(5);
    manager.removeRequest(&req1);
    manager.interrupt();
    sleep(1);
    }

    while (1)
    {
        sleep(5);
    }

    return 0;
}
