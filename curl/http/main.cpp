#include <iostream>
#include <pthread.h>
#include "http.h"
    
HTTPManager *manager;

void http_complete(HTTPRequest* request)
{
    request->getResult();
}

void* pthread_run(void* param)
{
    std::cout << static_cast<const char*>(param) << "\n" ;
    HTTPRequest *req = new HTTPRequest((const char*)param, std::bind(http_complete, std::placeholders::_1));
    manager->addRequest(req);

    sleep(5);
}

int main()
{
    manager = new HTTPManager;
    const char* req[] = {
        "http://localhost:9090",
        "http://localhost:9091",
        "http://localhost:9092",
        "http://localhost:9093",
        "http://localhost:9094",
    };

    pthread_t tid[5];
    for(int i = 0; i < 5; i++)
    {
        pthread_create(tid+i, NULL, pthread_run, const_cast<char*>(req[i]));
    }

    while (1)
    {
        sleep(5);
    }

    delete manager;

    return 0;
}
