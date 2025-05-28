# `CXX`

# 常见快速类型转换

1.`int` ->`string`

```c++
#include <string>
#include <iostream>

int num = 123;
std::string str = std::to_string(num);
std::cout << str << std::endl; // 输出 "123"
```

2.`string` -> `int`

```c++
#include <sstream>
int str_to_int(string str_) {
  int num;
  std::stringstream ss(str_);
  ss >> num; // 从字符串流读取到 int
  return num;
  //0123 -> 123
}
```

# 递归写法

### 题目:递归左右区间,单个元素返回值

偶数区间返回`L*R`,奇数返回`|L-R| * mid * |L+R|`
使用带返回值的写法即可

```cpp
lld dfs(const vector<lld> s) {
  //
  if (s.size() == 1)
    return s[0] mod MOD_N;

  lld size_ = s.size() / 2;

  if (s.size() % 2 == 0) {
    vector<lld> s1(s.begin(), s.begin() + size_);
    vector<lld> s2(s.begin() + size_, s.end());
    lld L = dfs(s1);
    lld R = dfs(s2);
    return mod_mul(L, R);

  }

  else {
    vector<lld> s1(s.begin(), s.begin() + size_);
    vector<lld> s2(s.begin() + size_ + 1, s.end());
    lld mid = s[size_];
    lld L = dfs(s1);
    lld R = dfs(s2);
    lld LR = mod_mul(std::abs(mod_add(L, -R)), mod_add(L, R));
    return mod_mul(mid, LR);
  }
}
void solve() {
  int n;
  cin >> n;
  vector<lld> s(n);
  for (auto &i : s)
    cin >> i;
  sum = dfs(s);
  std::cout << sum << ENDL;
}
```

# STL 常见用法

`String`

快速清空数组为 0/-1,但是不建议为其他除了 0/-1 之外的数字

```cpp
#include <cstring> // for memset

int arr[1000];
memset(arr, 0, sizeof(arr)); // 全部置 0
```

### 1.`vector`

区间表示法:`[begin, end)`,即左闭右开区间

```cpp
//从已有vector创建新的vector
bool check(vector<int> a) {
  int n = a.size();
  for (int pos = 1; pos < n; ++pos) {
    vector<lld> left(a.begin(), a.begin() + pos);
    // left创建了从[start,pos)的区间

    vector<lld> right(a.begin() + pos, a.end());
    // right创建了从[pos,end)的区间
  }
}
```

### 2. `string`

```cpp
string s;
s.erase(std::remove(s.begin(), s.end(), '0'), s.end()); //从字符串 s 中移除所有 '0' 字符。
```

### 3 .`bitset`

```cpp
std::bitset<8> b("10101010");
std::string str = b.to_string();  // "10101010"
//某些情况把数字转为二进制在转为字符
```

### 4.哈希数据结构通用函数

```cpp
std::unordered_set<int> s;
s.insert(42);
if (s.count(42)) {
  std::cout << "Found!" << std::endl;
}
auto it = mySet.find(key); //
if (it != mySet.end()) {
  std::cout << "找到元素: " << *it << std::endl;
} else {
  std::cout << key << " 未找到" << std::endl;
}
```



## 位运算

使用 01 串枚举所有的可能,根据 01 来决定如何运算

```cpp
bool f(int n) {
  for (int i = 0; i < 1 << (n - 1); ++i) {
    // 0-2^n-1-1
    //
    for (int j = 1; j < n; ++j) {
      // 检查j的第 j-1 位是否为 1
      // 如果是 则为 true 进入if
      // 否则进入 else
      if (i & 1 << (j - 1)) {
        return false;
        // body
      } else {
        return false;
        // body
      }
    }
  }
  return false;
}

```

## 查找类

### 高效查找两个固定数组的共同元素

### **哈希**

```cpp
//哈希
#include <unordered_set>
#include <vector>

long long count_ele(const std::vector<long long> &a,
                    const std::vector<long long> &b) {
  std::unordered_set<long long> set_a(a.begin(),
                                      a.end()); // 将数组 a 存入哈希表
  long long count = 0;
  for (int num : b) {
    if (set_a.count(num)) { // 检查 b 的元素是否在 a 中存在
      count++;
    }
  }
  return count;
}
```

双指针实现

```cpp
long long count_elements(std::vector<long long> &a, std::vector<long long> &b) {
  std::sort(a.begin(), a.end());
  std::sort(b.begin(), b.end());
  long long i = 0, j = 0, count = 0;
  while (i < a.size() && j < b.size()) {
    if (a[i] == b[j]) {
      count++;
      i++;
      j++;
    } else if (a[i] < b[j]) {
      i++;
    } else {
      j++;
    }
  }
  return count;
}
```

### **双指针**

#### 排序后记录出现次数大于等于 2 的元素个数

```cpp
  // 1 1 1 2 2 3  sum=2
  int sum = 0;
  vector<int> s;
  int n = s.size();
  for (int i = 0, j = 1; j < n;) {
    while (j < n and s[i] == s[j])
      j++;
    if (j - i >= 2) {
      sum++;
    }
    i = j;
  }
```

### 滑动窗口写法

```cpp
```

### 计算区间最多覆盖

给定区间 $L_1=[a_1,b_1]...L_n=[a_n,b_n],求最多多少个数字能被覆盖$

1.把区间$左边a_i 标记为1,可到达,右边(b_i+1)标记为-1不可到达$

2.排序规则:先处理位置小的;同位置时,先处理 -1(结束)再处理 +1(开始)

3.只需要看第二个位置就行,如果直接求和就是答案

```cpp
vector<pair<lld, int>> events;

for (int i = 0; i < n; ++i) {
  lld a, b;
  cin >> a >> b;
  events.push_back({a, 1});      // 开始事件
  events.push_back({b + 1, -1}); // 结束事件
}

sort(events.begin(), events.end(),
     [](const pair<lld, int> &a, const pair<lld, int> &b) {
       if (a.first == b.first)
         return a.second < b.second;
       return a.first < b.first;
     });

int curr = 0, ans = 0;
for (auto x : events) {
  curr += x.second;
  ans = max(ans, curr);
}
cout << ans << '\n';
```



# 数论

## gcd lcm

```cpp
lld gcd(lld x, lld y) {
  if (y == 0) {
    return x;
  } else {
    return gcd(y, x % y);
  }
}

lld lcm(lld x, lld y) {
  return x * y / gcd(x, y);
}
```

## T 个询问区间`[L,R]`多少个素数

```cpp
const int MX = 1e5 + 10; // 只需预处理到sqrt(1e9)≈31623
vector<int> p;           // 存储小素数

// 预处理所有小于√R的素数 存储在数组p
void init() {
  // np[i]=true表示i不是素数
  vector<bool> np(MX);
  np[0] = np[1] = true;
  for (int i = 2; i < MX; ++i) {
    if (!np[i])
      p.push_back(i);
    for (int j = 0; j < p.size() && i * p[j] < MX; ++j) {
      np[i * p[j]] = true;
      if (i % p[j] == 0)
        break;
    }
  }
}

// 分段筛法查询区间[L,R]的素数个数
lld qry(lld L, lld R) {
  if (L > R)
    return 0;
  if (R < 2)
    return 0;

  // 标记区间[L,R]内的数是否为素数
  vector<bool> seg(R - L + 1, true);

  for (lld prime : p) {
    if (prime * prime > R)
      break;

    lld start = max(prime * prime, (L + prime - 1) / prime * prime);
    for (lld j = start; j <= R; j += prime) {
      seg[j - L] = false;
    }
  }

  if (L == 1)
    seg[0] = false; // 1不是素数

  lld cnt = 0;
  for (bool is_prime : seg) {
    cnt += is_prime;
  }
  return cnt;
}

int main() {
  init();
  int T = 1;
  cin >> T;
  while (T--) {
    lld L, R;
    cin >> L >> R;
    cout << qry(L, R) << std::endl;
  }
  return 0;
}
```

# 附录

## ASCII 对照表

```c++
//number
0: 48
b: 49
...
9: 57
//快速把char输出为int
cout << x - 48 << endl;


//字母
a: 97
b: 98
...
z: 122
```

## 头文件

```cpp

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <functional> // 需要包含这个头文件以使用 std::greater
#include <iostream>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_set>
#include <vector>

#define lld long long // long long 的 printf 占位符是 lld
#define ENDL '\n'     // 将 endl 替换为 \n 取消缓冲区
#define mod %
#define LOG(var) std::cout << #var << " = " << (var) << ENDL
const lld N = 1e5 + 9;
const lld MOD_N = 1e9 + 7;
const lld MAX_ = 1e9;

using std::cin;
using std::cout;
using std::priority_queue;
using std::queue;
using std::set;
using std::stack;
using std::string;
using std::unordered_set;
using std::vector;

// 大根堆 小根堆
std::priority_queue<int, std::vector<int>, std::greater<int>> min_heap;
std::priority_queue<int> max_heap;

// 自定义排序
auto cmp = [](int a, int b) {
  if (a % 2 == b % 2)
    return a > b;       // 同奇偶时降序
  return a % 2 < b % 2; // 奇数优先
};
std::priority_queue<int, vector<int>, decltype(cmp)> custom_heap(cmp);

// 按照年龄
struct Person {
  string name;
  int age;
};
auto cmp_ = [](Person a, Person b) {
  return a.age > b.age;
  // 注意是 >,表示小的在前
  // 小根堆年龄
  // 也就是实际是20 25 30
};
priority_queue<Person, vector<Person>, decltype(cmp_)> age_heap(cmp_);

//同理其他的容器 set
std::set<lld, decltype(cmp)> st(cmp);

lld safe_mod(lld x) {
  // 对负数取mod 保证结果在 [0, MOD-1]
  return (x % MOD_N + MOD_N) % MOD_N;
}
// 取模加法 (a + b) % MOD_N
lld mod_add(lld a, lld b) { return (a mod MOD_N + b mod MOD_N) mod MOD_N; }

// 取模乘法 (a * b) % MOD_N
lld mod_mul(lld a, lld b) { return (a mod MOD_N * b mod MOD_N) mod MOD_N; }

// 取模快速幂 (a^b) % MOD_N
lld mod_pow(lld a, lld b) {
  lld res = 1;
  while (b > 0) {
    if (b & 1)
      res = mod_mul(res, a);
    a = mod_mul(a, a);
    b >>= 1;
  }
  return res;
}

// gcd 和 lcm
lld gcd(lld x, lld y) {
  if (y == 0) {
    return x;
  } else {
    return gcd(y, x % y);
  }
}

lld lcm(lld x, lld y) { return x * y / gcd(x, y); }

inline int read();
inline void write(lld x);

bool check(lld mid) {
  // **body**
}
int bin_sh(lld L, lld R) {
  lld mid = L + (R - L) / 2;
  while (L <= R) {
    mid = L + (R - L) / 2;
    if (check(mid)) {
      L = mid + 1; // 调整左边界
    } else {
      R = mid - 1;
    }
  }
  return mid;
}
void solve() {
  // body
}
int main() {
  std::ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int T = 1;
  cin >> T;
  while (T--) {
    solve();
  }
}
// lld read 和 lld 类型的 write
inline lld read() {
  lld x = 0, f = 1;
  char ch = getchar();
  while (!isdigit(ch)) {
    if (ch == '-')
      f = -1;
    ch = getchar();
  }
  while (isdigit(ch)) {
    x = x * 10 + (ch - '0');
    ch = getchar();
  }
  return x * f;
}
//修改参数类型即可
inline void write(lld x) {
  if (x < 0)
    putchar('-'), x = -x;
  if (x > 9)
    write(x / 10);
  putchar(x % 10 + '0');
}
```
