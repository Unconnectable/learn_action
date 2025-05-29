#include <unistd.h>
#include <json-c/json.h>
#include "crawler.h"

/* return 1 means error and 0 is normal.
*  root_obj need just free soucre once.
*
*/

int parse_user_info(const MemoryStruct *response, UserInfo *user_info_ptr) {
    json_object *root_obj = NULL;
    json_object *status_obj = NULL;
    json_object *result_array_obj = NULL;
    json_object *user_obj = NULL;
    json_object *handle_obj = NULL;
    json_object *rating_obj = NULL;
    json_object *maxRating_obj = NULL;

    const char *status_str = NULL;
    int return_code = 1;

    memset(user_info_ptr, 0 , sizeof(UserInfo));

    // printf("parse_user_info: ready to parse %zu bytes of JSON data\n", response -> size);
    if(response == NULL || response -> memory == NULL || response -> size == 0) {
        fprintf(stderr , "parse_user_info: Invalid response input data.\n");
        return 1;
    }

    // --- output ---
    // printf("--- BEGIN Full JSON Response ---\n");
    // printf("%s\n", response->memory); 
    // printf("--- END Full JSON Response ---\n");
    // --- end ---




    root_obj = json_tokener_parse(response -> memory);
    if (root_obj == NULL) {
        fprintf(stderr, "parse_user_info: Failed to parse JSON string.\n");
        return 1;
    }

    if (!json_object_is_type(root_obj, json_type_object)) {
        fprintf(stderr, "parse_user_info: Root JSON is not an object.\n");
        goto cleanup_user_info;
    }


    // ------- check status --------
    if (!json_object_object_get_ex(root_obj, "status", &status_obj) || !json_object_is_type(status_obj, json_type_string)) {
        fprintf(stderr , "parse_user_info: 'status' field not found or not a string.\n");
        goto cleanup_user_info;
    }

    status_str = json_object_get_string(status_obj);
    if (strcmp(status_str, "OK") != 0) {
        fprintf(stderr, "parse_user_info: API status is not 'OK': %s\n", status_str);
        json_object *comment_obj = NULL;
        if (json_object_object_get_ex(root_obj, "comment", &comment_obj) && json_object_is_type(comment_obj, json_type_string)) {
            fprintf(stderr , "          comment: %s\n", json_object_get_string(comment_obj));
        }
        goto cleanup_user_info;
    }


    // --------- get result array -----------
    if (!json_object_object_get_ex(root_obj, "result", &result_array_obj) || !json_object_is_type(result_array_obj, json_type_array)) {
        fprintf(stderr, "parse_user_info: 'result' field not found or not an array.\n");
        goto cleanup_user_info;
    }

    // ------ check result array -------
    if (json_object_array_length(result_array_obj) == 0) {
        fprintf(stderr, "parse_user_info: 'result' array is empty.\n");
        goto cleanup_user_info;
    }

    user_obj = json_object_array_get_idx(result_array_obj, 0);
    if (user_obj == NULL || !json_object_is_type(user_obj, json_type_object)) {
        fprintf(stderr, "parse_user_info: First element in 'result' array is not a valid user object.\n");
        goto cleanup_user_info;
    }


    // ------- get user info --------
    if (json_object_object_get_ex(user_obj, "handle", &handle_obj) && json_object_is_type(handle_obj, json_type_string)) {
        const char *handle_str = json_object_get_string(handle_obj);
        strncpy(user_info_ptr -> handle, handle_str, sizeof(user_info_ptr -> handle) - 1);
        user_info_ptr -> handle[sizeof(user_info_ptr -> handle) - 1] = '\0';
    } else {
        fprintf(stderr, "parse_user_info: Warning - Failed to get 'handle' string.\n");
    }

    if (json_object_object_get_ex(user_obj, "rating", &rating_obj) && json_object_is_type(rating_obj, json_type_int)) {
        user_info_ptr -> rating = json_object_get_int(rating_obj);
    } else {
        fprintf(stderr, "parse_user_info: Warning - Failed to get 'rating' int or field not present.\n");
        user_info_ptr -> rating = 0;
    }

    if (json_object_object_get_ex(user_obj, "maxRating", &maxRating_obj) && json_object_is_type(maxRating_obj, json_type_int)) {
        user_info_ptr -> maxRating = json_object_get_int(maxRating_obj);
    } else {
        fprintf(stderr, "parse_user_info: Warning - Failed to get 'maxRating' int or field not present.\n");
        user_info_ptr -> maxRating = 0;
    }
    // ------ get user info end -------


    // ------ check last time ------
    if (strlen(user_info_ptr -> handle) > 0) {
        printf("parse_user_info: Successfully parsed user info.\n");
        return_code = 0;
    } else {
        fprintf(stderr, "parse_user_info: Failed to extract mandatory 'handle'.\n");
    }

    // fprintf(stderr , "But our function has not been implemented yet ^_^\n");
    // return 1;
cleanup_user_info :
    if(root_obj != NULL) {
        printf("parse_user_info: Cleaning up JSON object...\n");
        json_object_put(root_obj);
    }
    return return_code;

}

int parse_user_rating(const MemoryStruct *response, UserRatingChange ratings_list[], int max_ratings, int *count_ptr) {
    json_object *root_obj = NULL;
    json_object *status_obj = NULL;
    json_object *result_array_obj = NULL;
    json_object *contest_obj = NULL;
    json_object *comment_obj = NULL;

    const char *status_str = NULL;
    int return_code = 1;
    int actual_count = 0;
    int array_len = 0;
    *count_ptr = 0;

    // printf("parse_user_rating: ready to parse %zu bytes\n", response -> size);
    if(response == NULL || response -> memory == NULL || response -> size == 0) {
        fprintf(stderr , "parse_user_rating: Invalid response input data.\n");
        return 1;
    }

    root_obj = json_tokener_parse(response -> memory);
    if (root_obj == NULL) {
        fprintf(stderr, "parse_user_rating: Failed to parse JSON string.\n");
        return 1;
    }

    if (!json_object_is_type(root_obj, json_type_object)) {
        fprintf(stderr, "parse_user_rating: Root JSON is not an object.\n");
        goto cleanup_user_rating;
    }


    // ------ check status --------
    if (!json_object_object_get_ex(root_obj, "status", &status_obj) || !json_object_is_type(status_obj, json_type_string)) {
        fprintf(stderr, "parse_user_rating: 'status' field not found or not a string.\n");
        goto cleanup_user_rating;
    }

    status_str = json_object_get_string(status_obj);
    if (strcmp(status_str, "OK") != 0) {
        fprintf(stderr, "parse_user_rating: API status is not 'OK': %s\n", status_str);
        if (json_object_object_get_ex(root_obj, "comment", &comment_obj) && json_object_is_type(comment_obj, json_type_string)) {
            fprintf(stderr, "             Comment: %s\n", json_object_get_string(comment_obj));
        }
        goto cleanup_user_rating;
    }


    // -------- get result array ----------
    if (!json_object_object_get_ex(root_obj, "result", &result_array_obj) || !json_object_is_type(result_array_obj, json_type_array)) {
        fprintf(stderr, "parse_user_rating: 'result' field not found or not an array.\n");
        goto cleanup_user_rating;
    }

    // -------- traverse result array ----------
    array_len = json_object_array_length(result_array_obj);
    printf("parse_user_rating: Found %d rating change records in JSON.\n", array_len);

    for(int i = 0; i < array_len; ++i) {
        if(actual_count >= max_ratings) {
            fprintf(stderr, "parse_user_rating: Warning - Reached maximum storage capacity (%d), skipping remaining records.\n", max_ratings);
            break;
        }

        contest_obj = json_object_array_get_idx(result_array_obj, i);
        if (contest_obj == NULL || !json_object_is_type(contest_obj, json_type_object)) {
            fprintf(stderr, "parse_user_rating: Warning - Skipping invalid object at index %d in result array.\n", i);
            continue;
        }

        UserRatingChange current_change = {0};
        json_object *temp_obj = NULL;

        // ------ contest id (int) ------
        if (json_object_object_get_ex(contest_obj, "contestId", &temp_obj) && json_object_is_type(temp_obj, json_type_int)) {
            current_change.contestId = json_object_get_int(temp_obj);
        }

        // ------ contest name (string) ------
        if (json_object_object_get_ex(contest_obj, "contestName", &temp_obj) && json_object_is_type(temp_obj, json_type_string)) {
            strncpy(current_change.contestName, json_object_get_string(temp_obj), sizeof(current_change.contestName) - 1);
            current_change.contestName[sizeof(current_change.contestName) - 1] = '\0'; 
        }

        // ------- rank (int) -------
        if (json_object_object_get_ex(contest_obj, "rank", &temp_obj) && json_object_is_type(temp_obj, json_type_int)) {
            current_change.rank = json_object_get_int(temp_obj);
        }

        // -------- rating update time seconds (int64) ---------
        if (json_object_object_get_ex(contest_obj, "ratingUpdateTimeSeconds", &temp_obj) && json_object_is_type(temp_obj, json_type_int)) {
            current_change.ratingUpdateTimeSeconds = json_object_get_int64(temp_obj);
        }

        // -------- old rating ---------
        if (json_object_object_get_ex(contest_obj, "oldRating", &temp_obj) && json_object_is_type(temp_obj, json_type_int)) {
            current_change.oldRating = json_object_get_int(temp_obj);
        }

        // ------- new rating ----------
        if (json_object_object_get_ex(contest_obj, "newRating", &temp_obj) && json_object_is_type(temp_obj, json_type_int)) {
            current_change.newRating = json_object_get_int(temp_obj);
        }

        ratings_list[actual_count++] = current_change;

    }

    if(actual_count > 0 || array_len == 0) {
        return_code = 0;
        *count_ptr = actual_count;
    } else if (array_len > 0 && actual_count == 0) {
        fprintf(stderr, "parse_user_rating: Warning - Found records in JSON but failed to parse any.\n");
    }

cleanup_user_rating :
    if (root_obj != NULL) {
        json_object_put(root_obj);
    }
    return return_code;

}


int parse_contest_list(const MemoryStruct *response, ContestInfo contests[], int max_contests, int *count_ptr) {
    json_object *root_obj = NULL;
    json_object *status_obj = NULL;
    json_object *result_array_obj = NULL;
    json_object *contest_obj = NULL;
    json_object *comment_obj = NULL;

    const char *status_str = NULL;
    int return_code = 1;
    int array_len = 0;
    int actual_count = 0;
    *count_ptr = 0;

    time_t current_time = time(NULL);
    time_t one_year_ago = current_time - (365 * 24 * 60 * 60);

    if(response == NULL || response -> memory == NULL || response -> size == 0) {
        fprintf(stderr , "parse_contest_list: Invalid response input data.\n");
        return 1;
    }

    root_obj = json_tokener_parse(response -> memory);
    if (root_obj == NULL) {
        fprintf(stderr, "parse_contest_list: Failed to parse JSON string.\n");
        return 1;
    }

    if (!json_object_is_type(root_obj, json_type_object)) {
        fprintf(stderr, "parse_contest_list: Root JSON is not an object.\n");
        goto cleanup_contest_list;
    }


    // ------ check status --------
    if (!json_object_object_get_ex(root_obj, "status", &status_obj) || !json_object_is_type(status_obj, json_type_string)) {
        fprintf(stderr, "parse_contest_list: 'status' field not found or not a string.\n");
        goto cleanup_contest_list;
    }

    status_str = json_object_get_string(status_obj);
    if (strcmp(status_str, "OK") != 0) {
        fprintf(stderr, "parse_contest_list: API status is not 'OK': %s\n", status_str);
        if (json_object_object_get_ex(root_obj, "comment", &comment_obj) && json_object_is_type(comment_obj, json_type_string)) {
            fprintf(stderr, "             Comment: %s\n", json_object_get_string(comment_obj));
        }
        goto cleanup_contest_list;
    }


    // -------- get result array ----------
    if (!json_object_object_get_ex(root_obj, "result", &result_array_obj) || !json_object_is_type(result_array_obj, json_type_array)) {
        fprintf(stderr, "parse_contest_list: 'result' field not found or not an array.\n");
        goto cleanup_contest_list;
    }

    // -------- traverse result array ----------
    array_len = json_object_array_length(result_array_obj);
    printf("parse_contest_list: Found %d contest records in JSON.\n", array_len);

    for (int i = 0; i < array_len; ++i) {
        contest_obj = json_object_array_get_idx(result_array_obj , i);
        if (contest_obj == NULL || !json_object_is_type(contest_obj, json_type_object)) {
            fprintf(stderr , "parse_constest_list: Warning - Skipping invaild object at index %d.\n", i);
            continue;
        }

        ContestInfo current_contest = {0};
        json_object *temp_obj = NULL;
        long long start_time = 0;
        const char *phase_str = NULL;

        if (json_object_object_get_ex(contest_obj, "startTimeSeconds", &temp_obj) && json_object_is_type(temp_obj, json_type_int)) {
            start_time = json_object_get_int64(temp_obj);
            if(start_time < one_year_ago) {
                continue;
            }
            current_contest.startTimeSeconds = start_time;
        } else {
            fprintf(stderr, "parse_contest_list: Warning - Skipping contest at index %d due to missing or invalid startTimeSeconds.\n", i);
            continue;
        }

        if (json_object_object_get_ex(contest_obj, "phase", &temp_obj) && json_object_is_type(temp_obj, json_type_string)) {
            phase_str = json_object_get_string(temp_obj);
            if (strcmp(phase_str , "FINISHED") != 0) {
                continue;
            } 
            strncpy(current_contest.phase, phase_str, sizeof(current_contest.phase) - 1);
            current_contest.phase[sizeof(current_contest.phase) - 1] = '\0';
        } else {
            fprintf(stderr, "parse_contest_list: Warning - Skipping contest at index %d due to missing or invalid phase.\n", i);
            continue;
        }

        if(actual_count >= max_contests) {
            fprintf(stderr, "parse_contest_list: Warning - Reached maximum storage capacity (%d), skipping remaining valid contests.\n", max_contests);
            break;
        }

        if (json_object_object_get_ex(contest_obj, "id", &temp_obj) && json_object_is_type(temp_obj, json_type_int)) {
            current_contest.id = json_object_get_int(temp_obj);
        } 

        if (json_object_object_get_ex(contest_obj, "name", &temp_obj) && json_object_is_type(temp_obj, json_type_string)) {
            const char *name_str = json_object_get_string(temp_obj);
            strncpy(current_contest.name, name_str, sizeof(current_contest.name) - 1);
            current_contest.name[sizeof(current_contest.name) - 1] = '\0';
        }

        contests[actual_count++] = current_contest;
    }

    if(actual_count > 0 || array_len == 0) {
        return_code = 0;
        *count_ptr = actual_count;
    } else if (array_len > 0 && actual_count == 0) {
        fprintf(stderr, "parse_contest_list: Warning - Found records in JSON but failed to parse any.\n");
    }


cleanup_contest_list:
    if(root_obj != NULL) {
        json_object_put(root_obj);
    }
    return return_code;
}

int parse_contest_standings(const MemoryStruct *response, const char *handle, UserProblemPerformance *performance_entry) {
    json_object *root_obj = NULL;
    json_object *status_obj = NULL, *comment_obj = NULL;
    json_object *result_obj = NULL;
    json_object *problems_array_obj = NULL, *rows_array_obj = NULL;
    json_object *user_row_obj = NULL; 
    json_object *problem_results_array_obj;

    const char *status_str;
    int return_code = 1; 

    if (response == NULL || response->memory == NULL || response->size == 0 ||
        handle == NULL || performance_entry == NULL) {
        fprintf(stderr, "parse_contest_standings: Error - Invalid input parameters.\n");
        return 1;
    }

    memset(performance_entry->problem_scores, 0, sizeof(performance_entry->problem_scores));
    performance_entry->contest_problems_num = 0;
    performance_entry->problems_user_attempted = 0;

    root_obj = json_tokener_parse(response->memory);
    if (root_obj == NULL) {
        fprintf(stderr, "parse_contest_standings: Error - Failed to parse JSON string for contestId %d.\n", performance_entry->UserRatingInfo.contestId);
        return 1;
    }

    if (!json_object_object_get_ex(root_obj, "status", &status_obj) || !json_object_is_type(status_obj, json_type_string)) {
        fprintf(stderr, "parse_contest_standings: Error - 'status' field not found or not a string in JSON response for contestId %d.\n", performance_entry->UserRatingInfo.contestId);
        goto cleanup_standings_parse;
    }
    status_str = json_object_get_string(status_obj);
    if (strcmp(status_str, "OK") != 0) {
        fprintf(stderr, "parse_contest_standings: Error - API status is not 'OK' for contestId %d. Status: %s\n", performance_entry->UserRatingInfo.contestId, status_str);
        if (json_object_object_get_ex(root_obj, "comment", &comment_obj) && json_object_is_type(comment_obj, json_type_string)) {
            fprintf(stderr, "                          API Comment: %s\n", json_object_get_string(comment_obj));
        }
        goto cleanup_standings_parse;
    }

    if (!json_object_object_get_ex(root_obj, "result", &result_obj) || !json_object_is_type(result_obj, json_type_object)) {
        fprintf(stderr, "parse_contest_standings: Error - 'result' field not found or not an object in JSON response for contestId %d.\n", performance_entry->UserRatingInfo.contestId);
        goto cleanup_standings_parse;
    }

    // -------------- get contest problems list -------------------
    if (json_object_object_get_ex(result_obj, "problems", &problems_array_obj) && json_object_is_type(problems_array_obj, json_type_array)) {
        performance_entry->contest_problems_num = json_object_array_length(problems_array_obj);

        int problems_num = performance_entry->contest_problems_num;

        for (int i = 0; i < problems_num; ++i) {
            json_object *problem_item_obj = json_object_array_get_idx(problems_array_obj, i);
            json_object *problem_index_obj;
            if (problem_item_obj && json_object_object_get_ex(problem_item_obj, "index", &problem_index_obj) && json_object_is_type(problem_index_obj, json_type_string)) {
                strncpy(performance_entry->problem_scores[i].index, json_object_get_string(problem_index_obj), sizeof(performance_entry->problem_scores[i].index) - 1);
                performance_entry->problem_scores[i].index[sizeof(performance_entry->problem_scores[i].index) - 1] = '\0';
            }
        }
    } else {
        fprintf(stderr, "parse_contest_standings: Warning - 'problems' array not found or not an array for contestId %d. Problem indices might be missing.\n", performance_entry->UserRatingInfo.contestId);
    }

    // ----------------- get participants standings list -> rows array ------------
    if (!json_object_object_get_ex(result_obj, "rows", &rows_array_obj) || !json_object_is_type(rows_array_obj, json_type_array)) {
        fprintf(stderr, "parse_contest_standings: Error - 'rows' array not found or not an array for contestId %d.\n", performance_entry->UserRatingInfo.contestId);
        goto cleanup_standings_parse;
    }

    int rows_size = json_object_array_length(rows_array_obj);

    // get target user's handle.
    for (int i = 0; i < rows_size; ++i) {
        json_object *cur_row_obj = json_object_array_get_idx(rows_array_obj, i);
        json_object *party_obj, *members_array_obj, *first_member_obj, *handle_obj;

        if (cur_row_obj && json_object_object_get_ex(cur_row_obj, "party", &party_obj) &&
            json_object_is_type(party_obj, json_type_object) &&
            json_object_object_get_ex(party_obj, "members", &members_array_obj) &&
            json_object_is_type(members_array_obj, json_type_array) &&
            json_object_array_length(members_array_obj) > 0) {

            first_member_obj = json_object_array_get_idx(members_array_obj, 0); 
            if (first_member_obj && json_object_object_get_ex(first_member_obj, "handle", &handle_obj) && json_object_is_type(handle_obj, json_type_string)) {
                if (strcmp(json_object_get_string(handle_obj), handle) == 0) {
                    user_row_obj = cur_row_obj; 
                    break;
                }
            }
        }
    }

    if (user_row_obj == NULL) {
        return_code = 0; 
        goto cleanup_standings_parse;
    }

    // parse user result array.
    if (json_object_object_get_ex(user_row_obj, "problemResults", &problem_results_array_obj) &&
        json_object_is_type(problem_results_array_obj, json_type_array)) {

        int results_nums = json_object_array_length(problem_results_array_obj);

        if (performance_entry->contest_problems_num == 0 && results_nums > 0) {
            fprintf(stderr, "parse_contest_standings: Warning - contest_problems_num was 0, user has %d problem results for contest %d. Problem indices may be unreliable if not pre-filled.\n",
                    results_nums, performance_entry->UserRatingInfo.contestId);
        }

        int limit = results_nums;

        for (int i = 0; i < limit; ++i) {
            json_object *item_obj = json_object_array_get_idx(problem_results_array_obj, i); 
            ProblemScore *cur = &(performance_entry->problem_scores[i]);
            json_object *temp_json_val;

            // parse points.
            if (json_object_object_get_ex(item_obj, "points", &temp_json_val) && (
                json_object_is_type(temp_json_val, json_type_int) || json_object_is_type(temp_json_val, json_type_double)
            )) {
                cur->points = (int)json_object_get_double(temp_json_val);
            }

            // get failed attempted count 
            if (json_object_object_get_ex(item_obj, "rejectedAttemptCount", &temp_json_val) && json_object_is_type(temp_json_val, json_type_int)) {
                cur->failedAttemptCount = json_object_get_int(temp_json_val);
            }

            // get Ac time seconds
            if (json_object_object_get_ex(item_obj, "bestSubmissionTimeSeconds", &temp_json_val) && json_object_is_type(temp_json_val, json_type_int)) {
                cur->AcTimeSeconds = json_object_get_int64(temp_json_val);
            }

            if (cur->points > 0 || cur->failedAttemptCount > 0) {
                performance_entry->problems_user_attempted++;
            }
        }
        return_code = 0;
    } else {
        return_code = 0; 
    }

cleanup_standings_parse:
    if (root_obj != NULL) {
        json_object_put(root_obj);
    }
    return return_code;
}

