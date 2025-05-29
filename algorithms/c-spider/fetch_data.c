#include "crawler.h"

// we can see detail of this function in https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)userp;
    char *ptr;
    if(mem -> memory == NULL) {
        ptr = malloc(realsize + 1);
    } else {
        ptr = realloc(mem -> memory , mem -> size + realsize + 1);
    } 

    if(ptr == NULL) {
        fprintf(stderr , "WriteMemoryCallback: malloc or realloc failure!\n");
        return 0;
    }

    mem -> memory = ptr;
    memcpy(&(mem -> memory[mem -> size]), contents , realsize);
    mem -> size += realsize;
    mem -> memory[mem -> size] = 0;

    return realsize;
}

int fetch_data(const char *url, MemoryStruct *chunk_ptr) {
    CURL *curl_handle;
    CURLcode res;
    long http_code = 0;

    chunk_ptr -> memory = NULL;
    chunk_ptr -> size = 0;
    
    printf("fetch_data: ready to fetch url: %s\n", url);

    // we see this API in https://curl.se/libcurl/c/curl_easy_init.html
    curl_handle = curl_easy_init();
    if(!curl_handle) {
        fprintf(stderr , "fetch_data: curl_easy_init() failure!\n");
        return 1;
    }
    
    // -- set curl option -- 
    // Interface : CURLcode curl_easy_setopt(CURL *handle, CURLoption option, parameter);
    // https://curl.se/libcurl/c/curl_easy_setopt.html
    curl_easy_setopt(curl_handle, CURLOPT_URL, url); // 设置请求 url 地址
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); // 设置回调函数， 处理数据
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)chunk_ptr); 
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "c-crawler/1.0 (Ubuntu 24.04 LTS; Student Project)"); // user-agent
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 1L); // SSL-verify, 1L 启用对等方证书验证
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 2L); // SSL-verify, 2L 启用主机名验证并进行匹配检查(严格) 
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L); // 1L 启用重定向跟随
    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 10L); // 连接超时设为 10 秒
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 30L); // 总操作超时设为 30 秒
    
    // -- excute request --
    res = curl_easy_perform(curl_handle);

    // -- check result --
    if(res != CURLE_OK) {
        fprintf(stderr , "fetch_data: curl_easy_perform() failure!: %s\n", curl_easy_strerror(res));
        if (chunk_ptr -> memory != NULL) {
            free(chunk_ptr -> memory);
            chunk_ptr -> memory = NULL;
            chunk_ptr -> size = 0;
        }
        curl_easy_cleanup(curl_handle);
        return 1;
    }

    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
    printf("fetch_data: HTTP CODE: %ld\n", http_code);

    curl_easy_cleanup(curl_handle);

    if(http_code >= 200 && http_code < 300) {
        printf("fetch_data: success!\n");
        return 0;
    } else {
        fprintf(stderr , "fetch_data: HTTP request failure, HTTP CODE: %ld\n", http_code);
        // free(chunk_ptr -> memory);
        // chunk_ptr -> memory = NULL;
        // chunk_ptr -> size = 0;
        return 1;
    }


    // return 1; // just to satisfy the compiler 
}

