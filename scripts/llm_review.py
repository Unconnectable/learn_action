import os
import json
from github import Github
from openai import OpenAI

# 设置 API Key 和 base URL
LLM_API_KEY = os.getenv("LLM_API_KEY")
GITHUB_TOKEN = os.getenv("GITHUB_TOKEN")
REPO_NAME = os.getenv("GITHUB_REPOSITORY")

# 初始化 DeepSeek 客户端
client = OpenAI(
    api_key=LLM_API_KEY,
    base_url="https://api.deepseek.com"
)

# 初始化 GitHub 客户端
g = Github(GITHUB_TOKEN)
repo = g.get_repo(REPO_NAME)

# 获取事件数据
event_path = os.getenv("GITHUB_EVENT_PATH")
with open(event_path, "r") as f:
    event_data = json.load(f)


def get_llm_review(prompt):
    """调用 DeepSeek 获取评审建议"""
    response = client.chat.completions.create(
        model="deepseek-chat",
        messages=[
            {"role": "system", "content": "你是一个专业的代码审查助手"},
            {"role": "user", "content": prompt}
        ],
        temperature=0.5,
        max_tokens=800
    )
    return response.choices[0].message.content.strip()


# 处理 Pull Request 事件
if "pull_request" in event_data:
    pr_number = event_data["pull_request"]["number"]
    pr = repo.get_pull(pr_number)
    print("🔍 正在分析 Pull Request")

    # 获取 PR Diff
    diff_url = f"https://api.github.com/repos/{REPO_NAME}/pulls/{pr_number}"
    headers = {
        "Authorization": f"token {GITHUB_TOKEN}",
        "Accept": "application/vnd.github.v3.diff"
    }
    from requests import get
    diff_content = get(diff_url, headers=headers).text

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

    review_text = get_llm_review(prompt)

    # 在 PR 页面添加评论
    pr.create_issue_comment(f"🤖 **LLM Code Reviewer**: \n\n{review_text}")
    print("✅ PR 评审完成，已提交评论。")
    print(review_text)

# 处理 Commit 事件
elif event_data.get("ref", "").startswith("refs/heads/"):
    after_sha = event_data["after"]
    commit = repo.get_commit(after_sha)
    print(f"🔍 正在分析 Commit: {after_sha}")

    # 获取 Commit 内容
    files = commit.raw_data["files"]
    diff_str = "\n".join([f"{f['filename']}:\n{f.get('patch', '无 patch 信息')}" for f in files])

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

    review_text = get_llm_review(prompt)

    # 在 Commit 页面添加评论
    commit.create_comment(body=f"🤖 **LLM Code Reviewer**: \n\n{review_text}")
    print("✅ Commit 评审完成，已提交评论。")
    print(review_text)

else:
    print("⚠️ 不支持的事件类型")
    exit(1)