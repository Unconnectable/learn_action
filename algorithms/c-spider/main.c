// main.c 
#include "crawler.h"
#include <time.h>
#include <unistd.h>


int main (int argc, char *argv[]) {
    const char *target_user = ""; 

    
    if(argc < 2) {
        fprintf(stderr, "[ER] --Usage: %s <username>--\n", argv[0]); 
        return 1; 
    } else if(argc > 2) {
        fprintf(stderr, "[WW] --Multiple params. Using: %s--\n", argv[1]); 
    }
    target_user = argv[1];
    printf("[II] --Target user: %s--\n", target_user); 

    
    // ------- definition begin ----------
    char api_url_buffer[512];
    MemoryStruct api_response_buffer;
    int fetch_status, parse_status; 

    
    UserInfo user_data;

    
    #define MAX_RATING_CHANGES 500
    UserRatingChange rating_history[MAX_RATING_CHANGES];      
    UserRatingChange filtered_rating_history[MAX_RATING_CHANGES]; 
    int rating_history_cnt = 0;     
    int filtered_rating_cnt = 0;    

    
    #define MAX_CONTESTS 2000
    ContestInfo contest_list[MAX_CONTESTS];         
    ContestInfo filtered_contest_list[MAX_CONTESTS];  
    int contest_cnt = 0;            
    int filtered_contest_cnt = 0;   

    
    #define MAX_PERFORMANCE MAX_RATING_CHANGES 
    UserProblemPerformance contest_performance[MAX_PERFORMANCE]; 
    int performance_cnt = 0;        
    // ------- definition end ---------------

    
    // ----- initialize data ------------
    memset(&user_data, 0, sizeof(user_data));
    memset(rating_history, 0, sizeof(rating_history));
    memset(filtered_rating_history, 0, sizeof(filtered_rating_history));
    memset(contest_list, 0, sizeof(contest_list));
    memset(filtered_contest_list, 0, sizeof(filtered_contest_list));
    memset(contest_performance, 0, sizeof(contest_performance));

    
    printf("[II] --Initializing libcurl...--\n"); 
    if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
        fprintf(stderr , "[ER] --Cannot initialize libcurl--\n"); 
        return 1;
    }
    printf("[II] --libcurl initialized--\n\n"); 

    
    // --------------- user info ------------------
    snprintf(api_url_buffer , sizeof(api_url_buffer), "https://codeforces.com/api/user.info?handles=%s", target_user);
    printf("[II] --Fetching user info: %s--\n", api_url_buffer); 
    api_response_buffer.memory = NULL;   api_response_buffer.size = 0;
    fetch_status = fetch_data(api_url_buffer, &api_response_buffer);

    if(fetch_status == 0) {
        printf("[II] --User info fetched (%zu bytes)--\n", api_response_buffer.size); 

        parse_status = parse_user_info(&api_response_buffer , &user_data);

        if(parse_status == 0) {
            printf("[II] --User info parsed--\n"); 
        } else {
            fprintf(stderr, "[ER] --Failed to parse user info. Exiting--\n"); 
            if(api_response_buffer.memory) free(api_response_buffer.memory);
            curl_global_cleanup();
            return 1;
        }
        if (api_response_buffer.memory != NULL) free(api_response_buffer.memory);
   } else {
        fprintf(stderr, "[ER] --Failed to fetch user info. Exiting--\n"); 
        if(api_response_buffer.memory != NULL) free(api_response_buffer.memory);
        curl_global_cleanup();
        return 1;
    }
    printf("\n"); 
    wait_for_next_request();
    // --------------------- end ----------------------------

    
    // ------------------------ user rating ----------------------
    snprintf(api_url_buffer, sizeof(api_url_buffer), "https://codeforces.com/api/user.rating?handle=%s", target_user);
    printf("[II] --Fetching user rating: %s--\n", api_url_buffer); 
    api_response_buffer.memory = NULL;   api_response_buffer.size = 0;
    fetch_status = fetch_data(api_url_buffer, &api_response_buffer);

    if (fetch_status == 0) {
        printf("[II] --User rating fetched (%zu bytes)--\n", api_response_buffer.size); 

        parse_status = parse_user_rating(&api_response_buffer, rating_history, MAX_RATING_CHANGES, &rating_history_cnt);

        if (parse_status == 0) {
            printf("[II] --User rating parsed (%d records)--\n", rating_history_cnt); 
            filtered_rating_cnt = filter_div1_4_ratings(rating_history, rating_history_cnt,
                                                              filtered_rating_history, MAX_RATING_CHANGES);
            printf("[II] --Filtered %d Div. 1-4 ratings--\n", filtered_rating_cnt); 
        } else {
            fprintf(stderr, "[ER] --Failed to parse user rating--\n"); 
        }
        if (api_response_buffer.memory != NULL) free(api_response_buffer.memory);
    } else {
        fprintf(stderr, "[ER] --Failed to fetch user rating--\n"); 
        if (api_response_buffer.memory != NULL) free(api_response_buffer.memory);
    }
    printf("\n");
    // ------------------ end ------------------------


    
    // --------------- contest standing performance -------------
    printf("[II] ---Fetching Detailed Problem Scores---\n"); 
    time_t last_year_ts = time(NULL) - (365L * 24 * 60 * 60); 

    for (int i = 0; i < filtered_rating_cnt; ++i) {
        UserRatingChange *current_rating_summary = &filtered_rating_history[i]; 
        
        if (current_rating_summary->ratingUpdateTimeSeconds < last_year_ts) {
            continue;
        }

        if (performance_cnt >= MAX_PERFORMANCE) {
            fprintf(stderr, "[WW] --Reached MAX_PERFORMANCE (%d). Skipping details--\n", MAX_PERFORMANCE); 
            break;
        }
        
        contest_performance[performance_cnt].UserRatingInfo = *current_rating_summary;

        snprintf(api_url_buffer, sizeof(api_url_buffer),
                 "https://codeforces.com/api/contest.standings?contestId=%d&handles=%s&showUnofficial=true",
                 current_rating_summary->contestId, target_user);

        printf("\n[II] --Fetching standings: ID %d (%s)...--\n", current_rating_summary->contestId, current_rating_summary->contestName); 
        wait_for_next_request();
        api_response_buffer.memory = NULL; api_response_buffer.size = 0;
        fetch_status = fetch_data(api_url_buffer, &api_response_buffer);

        if (fetch_status == 0) {

            parse_status = parse_contest_standings(&api_response_buffer, target_user, &contest_performance[performance_cnt]);

            if (parse_status == 0) {
                printf("[II] --Parsed standings for contest %d--\n", current_rating_summary->contestId); 
                performance_cnt++; 
            } else {
                fprintf(stderr, "[ER] --Failed to parse standings for contest %d--\n", current_rating_summary->contestId); 
            }
        } else {
            fprintf(stderr, "[ER] --Failed to fetch standings for contest %d--\n", current_rating_summary->contestId); 
        }
        if (api_response_buffer.memory != NULL) {
            free(api_response_buffer.memory);
            api_response_buffer.memory = NULL;
            api_response_buffer.size = 0;
        }
    }
    printf("[II] ---Detailed Scores Fetching Complete. %d performances recorded---\n\n", performance_cnt); 
    wait_for_next_request();
    // -------------- end ------------------


    // ------------- contest list --------------
    snprintf(api_url_buffer, sizeof(api_url_buffer), "https://codeforces.com/api/contest.list?gym=false");
    printf("[II] --Fetching contest list: %s--\n", api_url_buffer); 
    api_response_buffer.memory = NULL; api_response_buffer.size = 0;
    fetch_status = fetch_data(api_url_buffer, &api_response_buffer);

    if (fetch_status == 0) {
        printf("[II] --Contest list fetched (%zu bytes)--\n", api_response_buffer.size); 

        parse_status = parse_contest_list(&api_response_buffer, contest_list, MAX_CONTESTS, &contest_cnt);
        
        if (parse_status == 0) {
            printf("[II] --Contest list parsed (%d yearly contests before Div filter)--\n", contest_cnt); 
            filtered_contest_cnt = filter_div1_4_contests(contest_list, contest_cnt,
                                                              filtered_contest_list, MAX_CONTESTS);
            printf("[II] --Filtered %d Div. 1-4 contests (last year)--\n", filtered_contest_cnt); 
        } else {
            fprintf(stderr, "[ER] --Failed to parse contest list--\n"); 
        }
        if (api_response_buffer.memory != NULL) {
            free(api_response_buffer.memory);
            api_response_buffer.memory = NULL;
            api_response_buffer.size = 0;
        }
    } else {
        fprintf(stderr, "[ER] --Failed to fetch contest list--\n"); 
        if (api_response_buffer.memory != NULL) {
            free(api_response_buffer.memory);
            api_response_buffer.memory = NULL;
            api_response_buffer.size = 0;
        }
    }
    printf("\n");


    
    // ---------- last part for print data ------------
    printf("[II] ---Generating Statistics and Output Files---\n\n"); 

    
    process_user_info(&user_data, target_user); 

    
    if (performance_cnt > 0) {
        process_performances(contest_performance, performance_cnt, target_user);
    } else {
        printf("[II] --No detailed contest performances for CSV output--\n"); 
    }
    
    if (performance_cnt > 0) {
        process_division_stats(contest_performance, performance_cnt, target_user);
    } else {
        printf("[II] --No data for division statistics--\n"); 
    }

    
    if (performance_cnt > 0) {
        process_monthly_stats(contest_performance, performance_cnt, target_user);
    } else {
        printf("[II] --No data for monthly statistics--\n"); 
    }
   
    process_attendance_rate(performance_cnt, filtered_contest_cnt, target_user);

    printf("\n[II] ---Statistics Generation and Output Complete---\n\n"); 

    
    printf("[II] --Cleaning up libcurl.--\n"); 
    curl_global_cleanup();

    printf("Program complete.\n"); 
    printf("Hope you have a nice day ~\n"); 

    return 0;
}
