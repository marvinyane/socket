#include <cerrno>
#include <cstring>
#include <iostream>
#include <algorithm>

#include "http.h"

HTTPRequest::HTTPRequest(std::string uri, std::function<void(HTTPRequest*)> callback)
{
    m_easyHandle = curl_easy_init();

    curl_easy_setopt(m_easyHandle, CURLOPT_URL, uri.c_str());
    curl_easy_setopt(m_easyHandle, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(m_easyHandle, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(m_easyHandle, CURLOPT_WRITEFUNCTION, onWriteData);
    curl_easy_setopt(m_easyHandle, CURLOPT_HEADERFUNCTION, onHeaderData);
    curl_easy_setopt(m_easyHandle, CURLOPT_WRITEDATA, (void*)this);

    m_callback = callback;
}

HTTPRequest::~HTTPRequest()
{
    curl_easy_cleanup(m_easyHandle);
}

void HTTPRequest::append(char* buffer, unsigned int length)
{
    this->m_recv.append(buffer, length);
}

size_t HTTPRequest::onWriteData(void* buffer, size_t size, size_t nmemb, void *lpVoid)
{
    HTTPRequest* request = static_cast<HTTPRequest*>(lpVoid);
    request->append(static_cast<char*>(buffer), size * nmemb);

    return size * nmemb;
}

size_t HTTPRequest::onHeaderData(void *buffer, size_t size, size_t nmemb, void *lpVoid)
{
    return size * nmemb;
}

int HTTPRequest::getResult()
{
    double statDouble ;
    double contentLength ;
    long statLong ;
    char* statString = NULL ;

    if( CURLE_OK == curl_easy_getinfo( m_easyHandle, CURLINFO_HTTP_CODE , &statLong ) ){
        std::cout << "Response code:  " << statLong << std::endl ;
    }

    if( CURLE_OK == curl_easy_getinfo( m_easyHandle, CURLINFO_CONTENT_TYPE , &statString ) ){
        std::cout << "Content type:   " << statString << std::endl ;
    }

    if( CURLE_OK == curl_easy_getinfo( m_easyHandle, CURLINFO_SIZE_DOWNLOAD , &statDouble ) ){
        std::cout << "Download size:  " << statDouble << "bytes" << std::endl ;
    }

    if (CURLE_OK == curl_easy_getinfo( m_easyHandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLength ) )
    {
        std::cout << "Content Length:  " << contentLength << std::endl;
    }

    return statLong;

}

std::string HTTPRequest::receive()
{
    CURLcode res = curl_easy_perform(m_easyHandle);

    /*check?*/
    if (CURLE_OK == res)
    {
        return this->m_recv;
    }

    return std::string();
}

void HTTPRequest::finished(long code)
{
    m_callback(this);

    curl_easy_cleanup(m_easyHandle);
}

HTTPManager::HTTPManager()
   : m_multiHandle(NULL)
     , m_running(true)
     , m_request()
     , tid(0)
{
    /*check single*/

    m_pipe[0] = m_pipe[1] = -1;
    int res = pipe(m_pipe);
    if (res != 0)
    {
        /*TODO?*/
        perror("make pipe failed!!!");
    }

    m_multiHandle = curl_multi_init();

    pthread_mutex_init(&m_lock, NULL);

    pthread_mutex_init(&m_mutex, NULL);
    pthread_cond_init(&m_cond, NULL);
    
    pthread_create(&tid, NULL, threadRun, this);

}

HTTPManager::~HTTPManager()
{
    m_running = false;
    set();
    pthread_join(tid, NULL);
    
    pthread_mutex_destroy(&m_lock);

    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cond);

    curl_multi_cleanup(m_multiHandle);

    close(m_pipe[0]);
    close(m_pipe[1]);
}

HTTPRequest* HTTPManager::searchRequest(CURL* handle)
{
    lock();
    std::vector<HTTPRequest*>::iterator it = m_request.begin();
    for(; it != m_request.end(); ++it)
    {
        if ((*it)->checkHandle(handle))
        {
            unlock();
            return *it;
        }
    }

    unlock();
    return NULL;
}

void HTTPManager::removeRequest(HTTPRequest* req)
{
    curl_multi_remove_handle(m_multiHandle, req->getHandle());
    lock();
    m_request.erase(std::remove(m_request.begin(), m_request.end(), req), m_request.end());
    unlock();
    std::cout << "left vector : " << m_request.size() << "\n";
}

void HTTPManager::addRequest(HTTPRequest *req)
{
    curl_multi_add_handle(m_multiHandle, req->getHandle());
    lock();
    m_request.push_back(req);
    unlock();
    set();
}


void* HTTPManager::threadRun(void* param)
{
    HTTPManager* client = static_cast<HTTPManager*>(param);
    client->run();
}

void HTTPManager::run()
{
    curl_global_init(CURL_GLOBAL_ALL);
    while (m_running)
    {
        int still_running = 0;
        int msgs_left = 0;

        lock();
        if (m_request.empty())
        {
            unlock();
            wait();
        }
        else
        {
            unlock();
        }

        curl_multi_perform(m_multiHandle, &still_running);

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

            curl_multi_timeout(m_multiHandle, &curl_timeo);
            if(curl_timeo >= 0) {
                timeout.tv_sec = curl_timeo / 1000;
                if(timeout.tv_sec > 1)
                    timeout.tv_sec = 1;
                else
                    timeout.tv_usec = (curl_timeo % 1000) * 1000;
            }

            /* get file descriptors from the transfers */
            mc = curl_multi_fdset(m_multiHandle, &fdread, &fdwrite, &fdexcep, &maxfd);

            if(mc != CURLM_OK)
            {
                fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
                usleep(10000);
                break;
            }


            /* On success the value of maxfd is guaranteed to be >= -1. We call
               select(maxfd + 1, ...); specially in case of (maxfd == -1) there are
               no fds ready yet so we call select(0, ...) --or Sleep() on Windows--
               to sleep 100ms, which is the minimum suggested value in the
               curl_multi_fdset() doc. */

            if(maxfd == -1) {
                /* Portable sleep for platforms other than Windows. */
                struct timeval wait = { 0, 100 * 1000 }; /* 100ms */
                rc = select(0, NULL, NULL, NULL, &wait);
            }
            else {
                /* Note that on some platforms 'timeout' may be modified by select().
                   If you need access to the original value save a copy beforehand. */
                /*set pipe*/

                FD_SET(m_pipe[0], &fdread);

                if (maxfd > m_pipe[0])
                {
                    maxfd = m_pipe[0];
                }
                rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
            }

            switch(rc) {
                case -1:
                    /* select error */
                    break;
                case 0: /* timeout */
                default: /* action */
                    {
                        if (FD_ISSET(m_pipe[0], &fdread)) {
                            char c;
                            int n;

                            printf("read pipe !!!\n");

                            do {
                                n = read(m_pipe[0], &c, 1);
                            } while (n < 0 && errno == EINTR);

                            if (n < 0) {
                                printf("Error reading from pipe (%s)", strerror(errno));
                            }
                        }
                        else
                        {
                            curl_multi_perform(m_multiHandle, &still_running);
                        }
                    }
                    break;
            }

            CURLMsg* msg;
            while ((msg = curl_multi_info_read(m_multiHandle, &msgs_left))) {
                if (msg && msg->msg == CURLMSG_DONE) {
                    long code = 0;
                    std::cout << "msg result code is : " << msg->data.result << "\n";

                    /*notify easy handle finished!*/
                    HTTPRequest* req = searchRequest(msg->easy_handle);
                    if (req)
                    {
                        req->finished(code);
                    }
                    removeRequest(req);
                }
            }
        } while(still_running);
        /* See how the transfers went */
    }
    curl_global_cleanup();
}

void HTTPManager::wait()
{
    std::cout << "http manager wait!\n";
    pthread_mutex_lock(&m_mutex);
    pthread_cond_wait(&m_cond, &m_mutex);
    pthread_mutex_unlock(&m_mutex);
}

void HTTPManager::set()
{
    std::cout << "http manager set!\n";
    pthread_mutex_lock(&m_mutex);
    pthread_cond_broadcast(&m_cond);
    pthread_mutex_unlock(&m_mutex);
}

void HTTPManager::interrupt()
{
    char dummy = 0;
    int n;
    do {
        n = write(m_pipe[1], &dummy, 1);
        printf("write pipe n = %d \n", n);
    } while ((n < 0) && (n == EINTR));

    if (n < 0) {
        printf("Error writing to pipe (%s)", strerror(n));
    }
}

void HTTPManager::clear()
{
    std::vector<HTTPRequest*>::iterator it = m_request.begin();
    for(; it != m_request.end(); it++)
    {
        curl_multi_remove_handle(m_multiHandle, (*it)->getHandle());
    }

    m_request.clear();
}


void HTTPManager::lock()
{
    pthread_mutex_lock(&m_lock);
}

void HTTPManager::unlock()
{
    pthread_mutex_unlock(&m_lock);
}
