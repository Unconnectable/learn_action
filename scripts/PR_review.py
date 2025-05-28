import os
import json
import requests
from github import Github

# 获取环境变量
GITHUB_TOKEN = os.getenv("GITHUB_TOKEN")
LLM_API_KEY = os.getenv("LLM_API_KEY")

# 初始化 GitHub 客户端
g = Github(GITHUB_TOKEN)
repo = g.get_repo(os.getenv("GITHUB_REPOSITORY"))

# 获取事件类型
event_path = os.getenv("GITHUB_EVENT_PATH")
with open(event_path, "r") as f:
    event_data = json.load(f)

# 判断事件类型
if "pull_request" in event_data:
    pr_number = event_data["number"]
    pr = repo.get_pull(pr_number)
    print("🔍 正在分析 Pull Request")

    # 获取 PR Diff
    diff_url = f"https://api.github.com/repos/ {os.getenv('GITHUB_REPOSITORY')}/pulls/{pr_number}"
    headers = {
        "Authorization": f"token {GITHUB_TOKEN}",
        "Accept": "application/vnd.github.v3.diff"
    }
    diff_content = requests.get(diff_url, headers=headers).text

    # 构造 prompt
    prompt = f"""
请分析以下 Pull Request：
PR 标题: {pr.title}
PR 描述: {pr.body or '无'}
代码变更（Diff）:
{diff_content}

请从以下角度给出中文评审建议：
1. 代码风格是否规范？
2. 是否存在潜在 bug？
3. 是否符合项目设计模式？
4. 是否需要添加测试？
5. 其他建议？

请输出简洁清晰的评审意见。
"""

    # 调用 LLM（以 DeepSeek 为例）
    response = requests.post(
        "https://api.deepseek.com/chat/completions ",
        headers={
            "Content-Type": "application/json",
            "Authorization": f"Bearer {LLM_API_KEY}"
        },
        json={
            "model": "deepseek-coder",
            "messages": [{"role": "user", "content": prompt}],
            "temperature": 0.5,
            "max_tokens": 800
        }
    )

    if response.status_code != 200:
        print("❌ LLM 调用失败:", response.text)
        exit(1)

    review_text = response.json()["choices"][0]["message"]["content"]

    # 在 PR 下留言
    pr.create_issue_comment(f"🤖 **LLM Code Reviewer**: \n\n{review_text}")

elif "ref" in event_data and event_data["ref"].startswith("refs/heads/"):
    # 处理 push 事件（即 commit）
    after_sha = event_data["after"]
    commit = repo.get_commit(after_sha)
    print(f"🔍 正在分析 Commit: {after_sha}")

    # 获取 commit diff
    diff = commit.raw_data["files"]
    diff_str = "\n".join([f"{f['filename']}:\n{f.get('patch', '无 patch 信息')} " for f in diff])

    # 构造 prompt
    prompt = f"""
请分析以下 Git Commit:
Commit Message: {commit.commit.message}
Author: {commit.commit.author.name}
Date: {commit.commit.author.date}

代码变更（Patch）:
{diff_str}

请从以下角度给出中文评审建议：
1. 本次修改是否合理？
2. 是否引入潜在问题？
3. 是否有风格不一致？
4. 是否需要补充文档或注释？

请输出简洁清晰的评审意见。
"""

    # 调用 LLM
    response = requests.post(
        "https://api.deepseek.com/chat/completions ",
        headers={
            "Content-Type": "application/json",
            "Authorization": f"Bearer {LLM_API_KEY}"
        },
        json={
            "model": "deepseek-coder",
            "messages": [{"role": "user", "content": prompt}],
            "temperature": 0.5,
            "max_tokens": 600
        }
    )

    if response.status_code != 200:
        print("❌ LLM 调用失败:", response.text)
        exit(1)

    review_text = response.json()["choices"][0]["message"]["content"]

    # 在 Commit 页面添加评论
    commit.create_comment(body=f"🤖 **LLM Code Reviewer**: \n\n{review_text}")

else:
    print("⚠️ 不支持的事件类型")
    exit(1)