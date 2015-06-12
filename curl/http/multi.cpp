#include <curl/curl.h>

#include "pthread.h"
#include <string>
#include <iostream>
#include <vector>

class HTTPRequest
{
    public:
        HTTPRequest(CURL* curl)
        {
            m_curl = curl;
        }

        void appendData(char* data, size_t size)
        {
            std::cout << "data append : " << data ;
            m_data.append(data, size);
        }

        CURL* m_curl;
    private:
        std::string m_data;
};

class HTTPClient
{
    public:
        HTTPClient();
        ~HTTPClient();

        void start();
        void stop();

        void addRequest(const char* uri);

        void getResponse(int id)
        {
            long code = 0;
            curl_easy_getinfo(m_req[0].m_curl, CURLINFO_RESPONSE_CODE, &code );
            std::cout << "http response code is : " << code << "\n";
        }

        CURLM* multi_handle;
    private:
        static void* run(void* client);
        static size_t onWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid);
        static size_t onHeaderData(char* buffer, size_t size, size_t nmemb, void* lpVoid);

        int handle_count;

        pthread_t tid;

        std::vector<HTTPRequest> m_req;
};


size_t HTTPClient::onWriteData(void* buffer, size_t size, size_t nmemb, void *lpVoid)
{
    if( NULL == buffer )
    {
        return -1;
    }
    ((HTTPRequest*)lpVoid)->appendData((char*)buffer, size * nmemb);

    return nmemb * size;

}

size_t HTTPClient::onHeaderData(char* buffer, size_t size, size_t nmemb, void* lpVoid)
{
    /*how to parse the head data?*/
    std::cout << "header data received : \n";
    std::cout << buffer;
    std::cout << "\n<---------------------------------\n";
    return nmemb * size;
}

void* HTTPClient::run(void* param)
{
#if 0
    while (1)
    {
        int max_fd = -1;
        int res = 0;

        fd_set rs, ws, es;
        FD_ZERO(&rs);
        FD_ZERO(&ws);
        FD_ZERO(&es);

        curl_multi_fdset(((HTTPClient*)client)->multi_handle, &rs, &ws, &es, &max_fd);
        
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        res = select(max_fd + 1, &rs, &ws, &es, &tv);

        if (res < 0)
        {
            //error
            std::cout << "select error!!!\n";
        }
        else
        {
            curl_multi_perform(((HTTPClient*)client)->multi_handle, &((HTTPClient*)client)->handle_count);
        }
    }
#endif
    while (1)
    {
        HTTPClient* client = (HTTPClient*)param;
        int still_running = 0;
        int msgs_left = 0;

        curl_multi_perform(client->multi_handle, &still_running);

        do {
            struct timeval timeout;
            int rc; /* select() return code */
            CURLMcode mc; /* curl_multi_fdset() return code */

            fd_set fdread;
            fd_set fdwrite;
            fd_set fdexcep;
            int maxfd = -1;

            long curl_timeo = -1;

            FD_ZERO(&fdread);
            FD_ZERO(&fdwrite);
            FD_ZERO(&fdexcep);

            /* set a suitable timeout to play around with */
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            curl_multi_timeout(client->multi_handle, &curl_timeo);
            if(curl_timeo >= 0) {
                timeout.tv_sec = curl_timeo / 1000;
                if(timeout.tv_sec > 1)
                    timeout.tv_sec = 1;
                else
                    timeout.tv_usec = (curl_timeo % 1000) * 1000;
            }

            /* get file descriptors from the transfers */
            mc = curl_multi_fdset(client->multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

            if(mc != CURLM_OK)
            {
                fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
                break;
            }

            /* On success the value of maxfd is guaranteed to be >= -1. We call
               select(maxfd + 1, ...); specially in case of (maxfd == -1) there are
               no fds ready yet so we call select(0, ...) --or Sleep() on Windows--
               to sleep 100ms, which is the minimum suggested value in the
               curl_multi_fdset() doc. */

            if(maxfd == -1) {
#ifdef _WIN32
                Sleep(100);
                rc = 0;
#else
                /* Portable sleep for platforms other than Windows. */
                struct timeval wait = { 0, 100 * 1000 }; /* 100ms */
                rc = select(0, NULL, NULL, NULL, &wait);
#endif
            }
            else {
                /* Note that on some platforms 'timeout' may be modified by select().
                   If you need access to the original value save a copy beforehand. */
                rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
            }

            switch(rc) {
                case -1:
                    /* select error */
                    break;
                case 0: /* timeout */
                default: /* action */
                    curl_multi_perform(client->multi_handle, &still_running);
                    break;
            }
        } while(still_running);

        /* See how the transfers went */
        CURLMsg* msg;
        while ((msg = curl_multi_info_read(client->multi_handle, &msgs_left))) {
            if (msg && msg->msg == CURLMSG_DONE) {
                long code = 0;
                std::cout << "msg result code is : " << msg->data.result << "\n";
                curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &code );
                std::cout << "http response code is : " << code << "\n";
            }
        }
    }
}


HTTPClient::HTTPClient()
{
    multi_handle = curl_multi_init();
    handle_count = 0;
}

HTTPClient::~HTTPClient()
{
    curl_multi_cleanup(multi_handle);
}

void HTTPClient::addRequest(const char* uri)
{
    CURL* curl = NULL;
    curl = curl_easy_init();

    HTTPRequest *req = new HTTPRequest(curl);
    m_req.push_back(req);

    curl_easy_setopt(curl, CURLOPT_URL, uri);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onWriteData);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, onHeaderData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)req);

    curl_multi_add_handle(multi_handle, curl);
}

void HTTPClient::start()
{
    pthread_create(&tid, NULL, run, this);
}

void HTTPClient::stop()
{
    pthread_join(tid, NULL);
}


int main()
{
    HTTPClient client;

    client.addRequest("http://localhost:9091");

    client.start();

    while (1)
    {
        sleep(5);
    }
}

