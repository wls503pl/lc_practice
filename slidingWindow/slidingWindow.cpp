#include <vector>
#include <deque>
#include <iostream>

using namespace std;

vector<int> max_slidingWindow(const vector<int>& nums, int win_size)
{
    vector<int> results;
    deque<int> dq;
    for (int i = 0; i < nums.size(); ++i){
        while(!dq.empty() && dq.front() <= i - win_size)
        {
            dq.pop_front();
        }

        while(!dq.empty() && nums[dq.back()] <= nums[i])
        {
            dq.pop_back();
        }

        dq.push_back(i);

        if (i >= win_size - 1) {
            results.push_back(nums[dq.front()]);
        }
    }

    return results;
}

int main()
{
    int k = 3;
    vector<int> a = {101,100,98,102,103,99,96,99,100};
    vector<int> results = max_slidingWindow(a,k);

    cout << "input: ";
    for (int x : a) cout << x << " ";
    cout << "\nwindow size = " << k << "\noutput: ";
    for (int x : results) cout << x << " ";
    cout << endl;

    return 0;
}