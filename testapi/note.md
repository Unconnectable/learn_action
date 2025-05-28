```sh
curl -X POST "https://api.deepseek.com/chat/completions " \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer sk-3b469fab9f9d4cfd8cd61287a6d3818e" \
  -d '{
    "model": "deepseek-coder",
    "messages": [
      {"role": "system", "content": "你是一个专业的代码审查助手"},
      {"role": "user", "content": "请分析以下代码是否存在错误：function add(a, b) { return a - b; }"}
    ],
    "temperature": 0.5,
    "max_tokens": 200
  }'
```