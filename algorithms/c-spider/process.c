// process.c
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <json-c/json.h>
#include <time.h>
#include "crawler.h"

static int is_target_div(const char *contestName) {
    if(contestName == NULL) {
        return 0;
    }
    if(strstr(contestName, "Div. 1") != NULL ||
        strstr(contestName, "Div. 2") != NULL ||
        strstr(contestName, "Div. 3") != NULL ||
        strstr(contestName, "Div. 4") != NULL) 
    {
        return 1;
    }
    return 0;
}

static int is_dir_exis(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) { 
        if (mkdir(path, 0755) == -1) {
            if (errno != EEXIST) {
                fprintf(stderr, "[ER] --Failed to create directory '%s': %s--\n", path, strerror(errno));
                return -1;
            }
        } else {
            printf("[II] --Created directory: %s--\n", path);
        }
    }
    return 0; 
}

static void print_path(char *buffer, size_t size_,
                                   const char *subdir, const char *prefix, const char *suffix) {
    snprintf(buffer, size_, "%s/%s_%s", subdir, prefix, suffix);
}

const char* get_cf_rank_title(int rating) {
    if (rating < 1200) return "Newbie";
    if (rating < 1400) return "Pupil";
    if (rating < 1600) return "Specialist";
    if (rating < 1900) return "Expert";
    if (rating < 2100) return "Candidate Master";
    if (rating < 2300) return "Master";
    if (rating < 2400) return "International Master";
    if (rating < 2600) return "Grandmaster";
    if (rating < 3000) return "International Grandmaster";
    return "Legendary Grandmaster";
}

int filter_div1_4_ratings(const UserRatingChange source_ratings[], int source_size, UserRatingChange dest_ratings [], int limit) {
    int cnt = 0;
    if (source_ratings == NULL || dest_ratings == NULL || limit < 0) {
        fprintf(stderr, "[ER] --filter_div1_4_ratings: Invalid input parameters.--\n");
        return 0;
    }
    for (int i = 0; i < source_size; ++i) {
        if (is_target_div(source_ratings[i].contestName)) {
            if (cnt < limit) {
                dest_ratings[cnt++] = source_ratings[i]; 
            } else {
                fprintf(stderr, "[WW] --filter_div1_4_ratings: Destination array full (size %d). Skipping.--\n", limit);
                break; 
            } 
        } 
    }

    return cnt;
}


int filter_div1_4_contests(const ContestInfo source_contests[], int source_size, ContestInfo dest_contests [], int limit) {
    int cnt = 0;
    if (source_contests == NULL || dest_contests == NULL || limit < 0) {
        fprintf(stderr, "[ER] --filter_div1_4_contests: Invalid input parameters.--\n");
        return 0;
    }
    for (int i = 0; i < source_size; ++i) {
        if (is_target_div(source_contests[i].name)) {
            if (cnt < limit) {
                dest_contests[cnt++] = source_contests[i]; 
            } else {
                fprintf(stderr, "[WW] --filter_div1_4_contests: Destination array full (size %d). Skipping.--\n", limit);
                break; 
            } 
        }
    }

    return cnt;
}

void process_user_info(const UserInfo *user_info, const char *prefix) {
    const char *csv_dir = "csv";
    if (user_info == NULL || prefix == NULL) {
        fprintf(stderr, "[ER] --process_user_info: Invalid arguments.--\n");
        return;
    }

    printf("\n--------------- User Info (Console) ----------------\n");
    printf("  User Handle: %s\n", user_info->handle);
    printf("  Current Rating: %d (%s)\n", user_info->rating, get_cf_rank_title(user_info->rating));
    printf("  Max Rating: %d (%s)\n", user_info->maxRating, get_cf_rank_title(user_info->maxRating));
    printf("-----------------------------------------------------\n");

    // CSV
    
    if(is_dir_exis(csv_dir) != 0) return ;

    char filepath[512];
    print_path(filepath, sizeof(filepath), csv_dir, prefix, "user_summary.csv");

    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        fprintf(stderr, "[ER] --process_user_info (CSV): Failed to open file '%s': %s--\n", filepath, strerror(errno));
        return;
    }

    fprintf(fp, "Handle,CurrentRating,MaxRating,CurrentTitle,MaxTitle\n");
    fprintf(fp, "%s,%d,%d,%s,%s\n",
            user_info->handle,
            user_info->rating,
            user_info->maxRating,
            get_cf_rank_title(user_info->rating),
            get_cf_rank_title(user_info->maxRating));
    fclose(fp);
    printf("[II] --User summary data written to %s--\n", filepath);
}


void process_performances(const UserProblemPerformance performances[], int cnt, const char *prefix) { 
    const char *csv_dir = "csv";

    if (performances == NULL || cnt <= 0 || prefix == NULL) {
        fprintf(stderr, "[ER] --process_performances: Invalid arguments or no data.--\n");
        return;
    }

    if(is_dir_exis(csv_dir) != 0) return ;

    char filepath[512];
    print_path(filepath, sizeof(filepath), csv_dir, prefix, "contest_performances.csv");

    FILE *fp = fopen(filepath, "w");
    if (fp == NULL) {
        fprintf(stderr, "[ER] --process_performances (CSV): Failed to open file '%s': %s--\n", filepath, strerror(errno));
        return;
    }

    fprintf(fp, "ContestID,ContestName,Date,Rank,OldRating,NewRating,RatingChange,OldTitle,NewTitle,TotalProblemsInContest,ProblemsUserAttempted");
    int max_pro_ = 0;
    for(int k = 0; k < cnt; ++k) {
        if (performances[k].contest_problems_num > max_pro_) { 
            max_pro_ = performances[k].contest_problems_num;  
        }
        if (max_pro_ > (sizeof(performances[0].problem_scores)/sizeof(ProblemScore)) ) {
             max_pro_ = (sizeof(performances[0].problem_scores)/sizeof(ProblemScore));
        }
    }

    for (int i = 0; i < max_pro_; ++i) {
        fprintf(fp, ",P%dIndex,P%dScore,P%dFailedAttempts", i + 1, i + 1, i + 1);
    }
    fprintf(fp, "\n");

    // CSV Data
    for (int i = 0; i < cnt; ++i) {
        char date_str[20];
        time_t contest_time = (time_t)performances[i].UserRatingInfo.ratingUpdateTimeSeconds;
        strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&contest_time));

        fprintf(fp, "%d,\"%s\",%s,%d,%d,%d,%d,\"%s\",\"%s\",%d,%d",
                performances[i].UserRatingInfo.contestId,
                performances[i].UserRatingInfo.contestName,
                date_str,
                performances[i].UserRatingInfo.rank,
                performances[i].UserRatingInfo.oldRating,
                performances[i].UserRatingInfo.newRating,
                performances[i].UserRatingInfo.newRating - performances[i].UserRatingInfo.oldRating,
                get_cf_rank_title(performances[i].UserRatingInfo.oldRating),
                get_cf_rank_title(performances[i].UserRatingInfo.newRating),
                performances[i].contest_problems_num,
                performances[i].problems_user_attempted);

        for (int j = 0; j < max_pro_; ++j) {
            if (j < performances[i].contest_problems_num && j < (sizeof(performances[i].problem_scores)/sizeof(ProblemScore))) {
                fprintf(fp, ",\"%s\",%d,%d",
                        performances[i].problem_scores[j].index,
                        performances[i].problem_scores[j].points,
                        performances[i].problem_scores[j].failedAttemptCount);
            } else {
                fprintf(fp, ",,,");
            }
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
    printf("[II] --Contest performances data written to %s--\n", filepath);
}



void process_division_stats(const UserProblemPerformance perf[], int cnt, const char *prefix) {
    const char *csv_dir = "csv";
    if (perf == NULL || cnt <= 0 || prefix == NULL) {
        fprintf(stderr, "[ER] --process_division_stats: Invalid arguments or no data.--\n");
        return;
    }

    if(is_dir_exis(csv_dir) != 0) return;

    int div_cnt[4] = {0};
    int other_cnt = 0;
    for (int k = 0; k < cnt; ++k) {
        const char *name = perf[k].UserRatingInfo.contestName;
        if (strstr(name, "Div. 1")) div_cnt[0]++;
        else if (strstr(name, "Div. 2")) div_cnt[1]++;
        else if (strstr(name, "Div. 3")) div_cnt[2]++;
        else if (strstr(name, "Div. 4")) div_cnt[3]++;
        else other_cnt++;
    }

    printf("\n[II] --Participation Counts by Division (Based on %d detailed performances):--\n", cnt);
    printf("  Div. 1: %d\n", div_cnt[0]);
    printf("  Div. 2: %d\n", div_cnt[1]);
    printf("  Div. 3: %d\n", div_cnt[2]);
    printf("  Div. 4: %d\n", div_cnt[3]);
    if (other_cnt > 0) printf("  Other/Mixed: %d\n", other_cnt);

    char filepath[512];
    print_path(filepath, sizeof(filepath), csv_dir, prefix, "division_counts.csv");

    FILE *fp = fopen(filepath, "w");
    if (fp) {
        fprintf(fp, "Division,ContestsAttended\n");
        fprintf(fp, "Div. 1,%d\n", div_cnt[0]);
        fprintf(fp, "Div. 2,%d\n", div_cnt[1]);
        fprintf(fp, "Div. 3,%d\n", div_cnt[2]);
        fprintf(fp, "Div. 4,%d\n", div_cnt[3]);
        if (other_cnt > 0) fprintf(fp, "Other/Mixed,%d\n", other_cnt);
        fclose(fp);
        printf("[II] --Division participation counts written to %s--\n", filepath);
    } else {
        fprintf(stderr, "[ER] --process_division_stats (CSV): Failed to open file '%s': %s--\n", filepath, strerror(errno));
    }
}

void process_monthly_stats(const UserProblemPerformance perf[], int cnt, const char *prefix) {
    const char *csv_dir = "csv";
    if (perf == NULL || cnt <= 0 || prefix == NULL) {
        fprintf(stderr, "[ER] --process_monthly_stats: Invalid arguments or no data.--\n");
        return;
    }

    if(is_dir_exis(csv_dir) != 0) return;

    int monthly_cal[12] = {0}; 
    for (int k = 0; k < cnt; ++k) {
        time_t ts = (time_t)perf[k].UserRatingInfo.ratingUpdateTimeSeconds;
        struct tm *timeinfo = localtime(&ts);
        if (timeinfo) {
            monthly_cal[timeinfo->tm_mon]++;
        }
    }

    const char *month_names[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    printf("\n[II] --Monthly Participation (Based on %d detailed performances):--\n", cnt);
    for (int m = 0; m < 12; ++m) {
        printf("  %s: %d\n", month_names[m], monthly_cal[m]);
    }

    char filepath[512];
    print_path(filepath, sizeof(filepath), csv_dir, prefix, "monthly_cal.csv");
    FILE *fp = fopen(filepath, "w");
    if (fp) {
        fprintf(fp, "Month,ContestsAttended\n");
        for (int m = 0; m < 12; ++m) {
            fprintf(fp, "%s,%d\n", month_names[m], monthly_cal[m]);
        }
        fclose(fp);
        printf("[II] --Monthly participation data written to %s--\n", filepath);
    } else {
        fprintf(stderr, "[ER] --process_monthly_stats (CSV): Failed to open file '%s': %s--\n", filepath, strerror(errno));
    }
}

void process_attendance_rate(int user_statis, int total, const char *prefix) {
    const char* csv_dir = "csv";
    if (prefix == NULL) {
        fprintf(stderr, "[ER] --process_attendance_rate: Invalid filename_prefix.--\n");
        return;
    }

    if(is_dir_exis(csv_dir) != 0) return;

    double rate = 0.0;
    if (total > 0) {
        rate = ((double)user_statis / total) * 100.0;
    }

    printf("\n[II] --Attendance Rate (Last Year):--\n");
    printf("  User attended %d Div. 1-4 contests.\n", user_statis);
    printf("  Total Div. 1-4 contests held: %d\n", total);
    if (total > 0) {
        printf("  Attendance Rate: %.2f%%\n", rate);
    } else {
        printf("  Attendance Rate: N/A (No Div. 1-4 contests found in the list for the year).\n");
    }

    char filepath[512];
    print_path(filepath, sizeof(filepath), csv_dir, prefix, "attendance_rate.csv");
    FILE *fp = fopen(filepath, "w");
    if (fp) {
        fprintf(fp, "Metric,Value\n");
        fprintf(fp, "UserAttendedLastYear,%d\n", user_statis);
        fprintf(fp, "TotalDivContestsLastYear,%d\n", total);
        if (total > 0) {
            fprintf(fp, "AttendanceRate (%%),%.2f\n", rate);
        } else {
            fprintf(fp, "AttendanceRate (%%),N/A\n");
        }
        fclose(fp);
        printf("[II] --Attendance rate data written to %s--\n", filepath);
    } else {
        fprintf(stderr, "[ER] --process_attendance_rate (CSV): Failed to open file '%s': %s--\n", filepath, strerror(errno));
    }
}





void wait_for_next_request() {
    const int wait = 2;
    printf("wait_for_next_seconds: follow the API rate limit, need to wait %d seconds...\n", wait);
    sleep(wait);
}
