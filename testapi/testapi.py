from openai import OpenAI

client = OpenAI(
    base_url="https://api.deepseek.com/",
    api_key = "sk-3b469fab9f9d4cfd8cd61287a6d3818e"
)

completion = client.chat.completions.create(
    model="deepseek-chat",
    messages=[
        {
                "role": "user",
                "content": "下面这段的代码的效率很低，且没有处理边界情况。请先解释这段代码的问题与解决方法，然后进行优化：\n```\ndef fib(n):\n    if n <= 2:\n        return n\n    return fib(n-1) + fib(n-2)\n```"
        }
    ]
)
print(completion.choices[0].message.content)