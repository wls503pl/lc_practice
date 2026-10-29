#include <vector>
#include <string>
#include <sstream>

#include <iostream>

using namespace std;

vector<int> getTotalExecutionTime(int n, vector<string> logs) {
    // answer[i] 记录编号为 i 的函数累计运行时间
    vector<int> answer(n, 0);

    // 栈顶是当前正在运行的函数
    // 栈里下面的函数被暂停，等上面的函数结束后继续运行
    vector<int> st;

    // 上一段尚未计入时间的起点
    int prevTime = 0;

    // 按顺序处理每一条执行log
    for (const string & log : logs) {
        // 把一条log拆成：函数编号、start/end、时间戳
        string idText, action, timeText;
        stringstream ss(log);

        getline(ss, idText, ':');   // 读到第一个冒号前：idText = "2"
        getline(ss, action, ':');   // 读到第二个冒号前：action = "start"
        getline(ss, timeText); // 没有第三个冒号了，读取剩余内容：timeText = "4"

        int id = stoi(idText);
        int timestamp = stoi(timeText);

        if (action == "start") {
            // 新函数从 timestamp 这一秒开始运行
            // 如果已有函数正在运行，它只运行到 timestamp 前一秒
            if (!st.empty()) {
                answer[st.back()] += timestamp - prevTime;
            }

            // 新函数开始运行，压入栈顶
            st.push_back(id);

            // 新函数的计时从 timestamp 开始
            prevTime = timestamp;
        } else { // action == "end"
            // 函数运行到 timestamp 这一秒结束，结束的这一秒也要算进去
            answer[id] += timestamp - prevTime + 1;

            // 当前函数运行结束，移除栈顶
            st.pop_back();

            // 被暂停的函数如果存在，就从下一秒恢复运行
            prevTime = timestamp + 1;
        }
    }

    // 返回 0 到 n-1 号函数各自的累计运行时间
    return answer;
}

int main() {
    int n = 3;
    vector<string> logs = {
        "0:start:0",
        "2:start:4",
        "2:end:5",
        "1:start:7",
        "1:end:10",
        "0:end:11"
    };

    vector<int> result = getTotalExecutionTime(n, logs);

    cout << "[";
    for (int i = 0; i < result.size(); ++i) {
        if (i > 0) cout << ", ";
        cout << result[i];
    }
    cout << "]" << endl;

    return 0;
}