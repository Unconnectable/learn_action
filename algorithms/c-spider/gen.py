import pandas
import json
from datetime import datetime
import os
import argparse
import re

# --- config ---
OUTPUT_JSON_DIR = 'echarts_data'

# --- help function ---
def ensure_output_dir(directory):
    if directory and not os.path.exists(directory):
        try:
            os.makedirs(directory)
            print(f"[II] -- Created output directory: {directory} --")
        except OSError as e:
            print(f"[ER] -- Failed to create output directory {directory}: {e} --")
            return False
    return True

def save_option(option, suffix, username="user"): 
    if not OUTPUT_JSON_DIR or not ensure_output_dir(OUTPUT_JSON_DIR):
        print(f"--- ECharts Option for {username}_{suffix} ---")
        print(json.dumps(option, indent=4))
        print("--------------------------------------")
        return

    filepath = os.path.join(OUTPUT_JSON_DIR, f"{username}_{suffix}_option.json")
    try:
        with open(filepath, 'w') as f:
            json.dump(option, f, indent=4)
        print(f"[II] -- ECharts option saved to {filepath} --")
    except IOError as e:
        print(f"[ER] -- Failed to save ECharts option to {filepath}: {e} --")



# --- ECharts Option generate ---
def gen_rating_trend(pref):
    if pref.empty:
        print("[WW] -- No performance data for rating trend. --")
        return None
    
    sorted = pref.copy()
    if 'Date' in sorted.columns and pandas.api.types.is_datetime64_any_dtype(sorted['Date']):
        sorted = sorted.sort_values(by='Date')
    else:
        print(f"[WW] -- 'Date' column in pref is not datetime or missing. Rating trend sorting might be off. --")

    dates = sorted['Date'].astype(str).tolist() if 'Date' in sorted.columns else [f"Contest {i+1}" for i in range(len(sorted))]
    ratings = sorted['NewRating'].astype(int).tolist()

    option = {
        "title": {"text": "Rating Trend Over Time"}, "tooltip": {"trigger": "axis"},
        "legend": {"data": ["New Rating"]},
        "grid": {"left": '3%', "right": '4%', "bottom": '10%', "containLabel": True},
        "xAxis": {"type": "category", "boundaryGap": False, "data": dates, "axisLabel": {"rotate": 30, "interval": "auto"}},
        "yAxis": {"type": "value", "name": "Rating", "min": 'dataMin', "max": 'dataMax'},
        "dataZoom": [{"type": "slider", "start": 0, "end": 100, "bottom": 10}, {"type": "inside", "start": 0, "end": 100}],
        "series": [{"name": "New Rating", "type": "line", "smooth": True, "data": ratings, "areaStyle": {},
                    "markPoint": {"data": [{"type": "max", "name": "Max Rating"}, {"type": "min", "name": "Min Rating"}]},
                    "markLine": {"data": [{"type": "average", "name": "Avg Rating"}]}
                  }]
    }
    return option

def gen_monthly_partic(mon):
    if mon.empty:
        print("[WW] -- No monthly participation data. --")
        return None
    try:
        months = mon['Month'].astype(str).tolist() 
        counts = mon['ContestsAttended'].astype(int).tolist()
    except KeyError as e:
        print(f"[ER] -- Monthly CSV missing expected column: {e} --")
        return None
    except ValueError:
        print(f"[ER] -- Monthly CSV 'ContestsAttended' contains non-integer value. --")
        return None

    option = {
        "title": {"text": "Monthly Contest Participation (Last Year)"},
        "tooltip": {"trigger": "axis", "axisPointer": {"type": "shadow"}},
        "legend": {"data": ["Contests Attended"]},
        "grid": {"left": '8%', "right": '4%', "bottom": '3%', "containLabel": True},
        "xAxis": {"type": "category", "data": months, "axisTick": {"alignWithLabel": True}},
        "yAxis": {"type": "value", "name": "Number of Contests", "nameLocation": "middle", "nameGap": 35},
        "series": [{"name": "Contests Attended", "type": "bar", "barWidth": "60%", "data": counts, "itemStyle": {"borderRadius": [5, 5, 0, 0]}}]
    }
    return option

def gen_div_partic(div):
    if div.empty:
        print("[WW] -- No division participation data. --")
        return None
    segments = [] 
    try:
        for index, row in div.iterrows():
            segments.append({"value": int(row['ContestsAttended']), "name": str(row['Division'])})
    except KeyError as e:
        print(f"[ER] -- Division CSV missing expected column: {e} --")
        return None
    except ValueError:
        print(f"[ER] -- Division CSV 'ContestsAttended' contains non-integer value. --")
        return None

    option = {
        "title": {"text": "Contest Participation by Division", "left": "center"},
        "tooltip": {"trigger": "item", "formatter": "{a} <br/>{b} : {c} ({d}%)"},
        "legend": {"orient": "vertical", "left": "left", "data": [item['name'] for item in segments if item['value'] > 0]},
        "series": [{"name": "Division", "type": "pie", "radius": ['40%', '70%'], "avoidLabelOverlap": False,
                    "itemStyle": {"borderRadius": 10, "borderColor": '#fff', "borderWidth": 2},
                    "label": {"show": False, "position": 'center'},
                    "emphasis": {"label": {"show": True, "fontSize": '20', "fontWeight": 'bold'}},
                    "labelLine": {"show": False},
                    "data": [item for item in segments if item['value'] > 0]
                  }]
    }
    return option

def gen_problem_scores(pref):
    if pref.empty:
        print("[WW] -- No performance data for problem scores chart. --")
        return None

    contests = pref.copy() 
    if 'Date' in contests.columns and pandas.api.types.is_datetime64_any_dtype(contests['Date']):
        contests = contests.sort_values(by='Date').tail(11)
    else:
        contests = contests.tail(7)

    all_cols = contests.columns.tolist() 
    series = [] 
    legend_items = [] 

    score_cols = sorted( 
        [col for col in all_cols if re.match(r'P\d+Score', col)],
        key=lambda x: int(re.search(r'P(\d+)Score', x).group(1))
    )

    if not score_cols:
        print("[WW] -- No P<number>Score columns found for problem scores chart. --")
        return None

    for scr_col_name in score_cols: 
        match = re.search(r'P(\d+)Score', scr_col_name) 
        if not match: continue
        
        num_str = match.group(1) 
        idx = f'P{num_str}Index' 

        series_lab = f"P{num_str}" 
        if idx in all_cols:
            unique_idxs = list(set(filter(None, contests[idx].fillna('').astype(str).tolist()))) 
            if len(unique_idxs) == 1 and unique_idxs[0]:
                 series_lab = f"Problem {unique_idxs[0]}"
        
        legend_items.append(series_lab)
        series.append({
            "name": series_lab, "type": "bar", "stack": "Contest",
            "emphasis": {"focus": "series"},
            "data": contests[scr_col_name].fillna(0).astype(int).tolist()
        })

    raw = contests['ContestName'].astype(str).tolist() 
    formatted_names = [] 
    for name in raw:
        split_idx = name.find(' (') 
        if split_idx != -1 and len(name) > 22 :
            fmt_name = name[:split_idx] + '\n' + name[split_idx:] 
        else:
            fmt_name = name
        formatted_names.append(fmt_name)
            
    option = {
        "title": {"text": "Problem Scores in Recent Contests"},
        "tooltip": {"trigger": "axis", "axisPointer": {"type": "shadow"}},
        "legend": {"data": legend_items, "top": 30, "type": "scroll", "textStyle": {"fontSize": 10}},
        "grid": {"left": '3%', "right": '4%', "bottom": '20%', "containLabel": True},
        "xAxis": {"type": "category", "data": formatted_names, "axisLabel": {"interval": 0, "rotate": 30, "fontSize": 9}},
        "yAxis": {"type": "value", "name": "Score"},
        "series": series
    }
    return option

def gen_sum_and_attend(summary, attendance, username="user"): 
    output = {} 
    if not summary.empty:
        row = summary.iloc[0] 
        output['user'] = {
            "handle": row.get('Handle', username),
            "currentRating": int(row.get('CurrentRating', 0)),
            "maxRating": int(row.get('MaxRating', 0)),
            "currentTitle": row.get('CurrentTitle', 'N/A'),
            "maxTitle": row.get('MaxTitle', 'N/A')
        }
    else:
        output['user'] = {"handle": username, "currentRating": "N/A", "maxRating": "N/A", "currentTitle": "N/A", "maxTitle": "N/A"}

    if not attendance.empty:
        att_dict = dict(zip(attendance['Metric'], attendance['Value'])) 
        output['attendance'] = {
            "attendedLastYear": int(float(str(att_dict.get('UserAttendedLastYear', '0')).replace(',',''))),
            "totalContestsLastYear": int(float(str(att_dict.get('TotalDivContestsLastYear', '0')).replace(',',''))),
            "ratePercentage": float(str(att_dict.get('AttendanceRate (%)', '0.0')).replace(',',''))
        }
    else:
        output['attendance'] = {"attendedLastYear": 0, "totalContestsLastYear": 0, "ratePercentage": 0.0}
    
    save_option(output, "user_report_summary", username=username) 
    return output


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate ECharts option JSON from Codeforces user CSV data.")
    parser.add_argument("username", help="The Codeforces username, used as a prefix for CSV files and output JSON files.")
    cli_args = parser.parse_args() 
    username = cli_args.username 

    start_time = datetime.now() 
    print(f"[II] -- Starting Python script for user '{username}' at {start_time.strftime('%Y-%m-%d %H:%M:%S')} --")

    CSV_SUBDIR = "csv";
    
    summary_csv = os.path.join(CSV_SUBDIR, f'{username}_user_summary.csv')
    performances_csv = os.path.join(CSV_SUBDIR, f'{username}_contest_performances.csv')
    division_csv = os.path.join(CSV_SUBDIR, f'{username}_division_counts.csv')
    monthly_csv = os.path.join(CSV_SUBDIR, f'{username}_monthly_cal.csv')
    attendance_csv = os.path.join(CSV_SUBDIR, f'{username}_attendance_rate.csv')
    
    dataframes = {} 
    csv_map = { 
        "user_summary": summary_csv, "performances": performances_csv,
        "monthly": monthly_csv, "division": division_csv,
        "attendance": attendance_csv
    }

    for df_key, fpath in csv_map.items(): 
        try:
            dataframes[df_key] = pandas.read_csv(fpath)
            print(f"[II] -- Successfully loaded {fpath} --")
        except FileNotFoundError:
            print(f"[ER] -- CSV file not found: {fpath} --")
            dataframes[df_key] = pandas.DataFrame()
        except pandas.errors.EmptyDataError:
            print(f"[WW] -- CSV file is empty: {fpath} --")
            dataframes[df_key] = pandas.DataFrame()
        except Exception as e:
            print(f"[ER] -- Error loading CSV file {fpath}: {e} --")
            dataframes[df_key] = pandas.DataFrame()

    pref = dataframes.get("performances") 
    if pref is not None and not pref.empty:
        if 'Date' in pref.columns:
            try:
                dataframes["performances"]['Date'] = pandas.to_datetime(pref['Date'])
            except Exception as e:
                print(f"[WW] -- Could not convert 'Date' in performances_df to datetime: {e}. --")
        else:
            print(f"[WW] -- 'Date' column missing in performances_df. --")

    gen_sum_and_attend(dataframes.get("user_summary"), dataframes.get("attendance"), username=username)
    
    rating = gen_rating_trend(dataframes.get("performances"))
    if rating: save_option(rating, "rating_trend", username=username)

    monthly = gen_monthly_partic(dataframes.get("monthly"))
    if monthly: save_option(monthly, "monthly_cal", username=username)

    div = gen_div_partic(dataframes.get("division"))
    if div: save_option(div, "division_participation", username=username)

    problem_scores = gen_problem_scores(dataframes.get("performances"))
    if problem_scores: save_option(problem_scores, "problem_scores_recent", username=username)

    end = datetime.now() 
    print(f"[II] -- Python script for user '{username}' finished at {end.strftime('%Y-%m-%d %H:%M:%S')} (Duration: {end - start_time}) --")
