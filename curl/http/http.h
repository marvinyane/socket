#ifndef __HTTP_H__
#define __HTTP_H__

#include <string>
#include <vector>
#include <curl/curl.h>

#include <pthread.h>

class HTTPRequest
{
    public:
        HTTPRequest(std::string uri);
        ~HTTPRequest();

        /*async*/
        void receive(char* buffer, unsigned int size);

        /*sync*/
        std::string receive();

        /**
         * @brief http response code
         */
        int getResult();

        bool checkHandle(CURL* handle)
        {
            return handle == m_easyHandle;
        }

        CURL* getHandle()
        {
            return m_easyHandle;
        }

        void append(char* buffer, unsigned int length);
        void finished(long code);
    private:
        static size_t onWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid);
        static size_t onHeaderData(void* buffer, size_t size, size_t nmemb, void* lpVoid);
        
        CURL* m_easyHandle;
        std::string m_recv;
};


class HTTPManager
{
    public:
        HTTPManager();
        ~HTTPManager();

        void addRequest(HTTPRequest* req);
        void removeRequest(HTTPRequest* req);

        HTTPRequest* searchRequest(CURL* handle);

        void wait();
        void set();

        void lock();
        void unlock();

        void interrupt();

        void clear();

    private:
        static void* threadRun(void* );

        void run();

        bool m_running;
        CURLM* m_multiHandle;
        std::vector<HTTPRequest*> m_request;

        pthread_t tid;
        pthread_mutex_t m_lock;
        pthread_mutex_t m_mutex;
        pthread_cond_t m_cond;
        int m_pipe[2];
        
};


#endif
