// debug.c
#include "crawler.h"  

int main () {

    char debug_api_url[256];
    MemoryStruct debug_api_response;
    int debug_fetch_status = 0;

    memset(debug_api_url, 0, sizeof(debug_api_url));
    printf("Enter the Codeforces API URL to fetch:\n> ");

    if (fgets(debug_api_url, sizeof(debug_api_url), stdin) == NULL) {
        fprintf(stderr, "Error reading input.\n");
        return 1;
    }

    debug_api_url[strcspn(debug_api_url, "\n")] = '\0';

    if (strlen(debug_api_url) == 0) {
         fprintf(stderr, "Error: No URL entered.\n");
         return 1;
    }

    printf("DEBUG: URL string before fetching (length %zu): [%s]\n", strlen(debug_api_url), debug_api_url);


    printf("Initializing libcurl...\n");
    if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
        fprintf(stderr , "cannot initialize libcurl\n");
        return 1;
    }

    debug_api_response.memory = NULL;
    debug_api_response.size = 0;

    printf("Fetching data from: %s\n", debug_api_url); 
    debug_fetch_status = fetch_data(debug_api_url, &debug_api_response);

    if(debug_fetch_status == 0) {
        printf("Success! Received %zu bytes.\n", debug_api_response.size);
        printf("--- Response Data ---\n");
        printf("%s\n", debug_api_response.memory ? debug_api_response.memory : "(empty response)");
        printf("--- End Response Data ---\n");
    } else {
        fprintf(stderr, "Failed to fetch data from the URL.\n");
    }

    if(debug_api_response.memory != NULL) {
        printf("Freeing response memory...\n");
        free(debug_api_response.memory);
    }

    printf("Cleaning up libcurl...\n");
    curl_global_cleanup();

    printf("Debug program complete.\n");

    return 0;
}
