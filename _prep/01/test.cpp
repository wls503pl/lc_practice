#include <vector>
#include <sstream>
#include <string>
#include <iostream>

using namespace std;

vector<int> getTotalExecutionTime(int n, vector<string> logs) {
    vector<int> result(n, 0);
    vector<int> st;
    string part1, part2, part3;
    int prevTime = 0;

    for (const auto & log : logs) {
        stringstream ss(log);

        getline(ss, part1, ':');
        getline(ss, part2, ':');
        getline(ss, part3);

        int id = stoi(part1);
        int timestamp = stoi(part3);

        if (part2 == "start") {
            if (!st.empty()) {
                result[st.back()] += timestamp - prevTime;
            }

            st.emplace_back(id);
            prevTime = timestamp;
        } else {
            result[id] += timestamp - prevTime + 1;
            prevTime = timestamp + 1;
            st.pop_back();
        }
    }

    return result;
}

/////////////////////////////

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
