#ifndef CRAWLER_H
#define CRAWLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// store libcurl data
typedef struct {
    char *memory;
    size_t size;
} MemoryStruct;

// store user data
typedef struct {
    char handle[100];
    int maxRating;
    int rating;
    // int problemsolved;
} UserInfo;

// user rating change
typedef struct {
    int contestId;
    int rank;
    int oldRating;
    int newRating;
    char contestName[256];
    long long ratingUpdateTimeSeconds;
} UserRatingChange;

typedef struct {
    char index[12];
    int points;
    int failedAttemptCount;
    long long AcTimeSeconds;
} ProblemScore;

typedef struct {
    UserRatingChange UserRatingInfo;
    ProblemScore problem_scores[15];
    int contest_problems_num;
    int problems_user_attempted;
} UserProblemPerformance;

// --- contest info ---
typedef struct {
    int id;
    char name[256];
    long long startTimeSeconds;
    char phase[50];
} ContestInfo;

int             fetch_data(const char *url, MemoryStruct *chunk_ptr);
int             parse_user_info(const MemoryStruct *response, UserInfo *user_info_ptr);
int             parse_user_rating(const MemoryStruct *response, UserRatingChange ratings_list[], int max_ratings, int *count_ptr);
int             parse_contest_list(const MemoryStruct *response, ContestInfo contests[], int max_contests, int *count_ptr);
int             parse_contest_standings(const MemoryStruct *response, const char *handle, UserProblemPerformance *performance_entry);
int             filter_div1_4_ratings(const UserRatingChange source_ratings[], int source_count, UserRatingChange dest_ratings [], int max_dest_size);
int             filter_div1_4_contests(const ContestInfo source_contests[], int source_count, ContestInfo dest_contests [], int max_dest_size);
// 5. 统计和展示用户参加各级比赛的次数，参赛次数的月度分布，出勤率
// all of them face to user

void            process_user_info(const UserInfo *user_info, const char *filename_prefix);
void            process_performances(const UserProblemPerformance performances[], int count, const char *filename_prefix);

// ----- 
void            process_division_stats(const UserProblemPerformance performances[], int count, const char *filename_prefix);
void            process_monthly_stats(const UserProblemPerformance performances[], int count, const char *filename_prefix);
void            process_attendance_rate(int user_yearly_attendance, int total_yearly_contests, const char *filename_prefix);
void            process_all_yearly_contests(const ContestInfo contests[], int count, const char *filename_prefix);






// void            process_user_rating(const UserRatingChange ratings_list[], int count);
// void            process_contest_list(const ContestInfo contests[], int count);
// void            process_filtered_user_rating(const UserRatingChange filtered_rating[], int count);
// void            process_filtered_contest_list(const ContestInfo filtered[], int count);
// void            calculate_and_print_division_counts(const UserRatingChange filtered_ratings[], int count);
// void            calculate_and_print_monthly_distribution(const UserRatingChange filtered_ratings[], int count);
// void            calculate_and_print_attendance_rate(const UserRatingChange filtered_ratings[], int rating_count, int total_div1_4_contest_count_last_year);



void            wait_for_next_request();

#endif

