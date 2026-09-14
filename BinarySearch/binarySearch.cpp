#include <iostream>
#include <vector>

// 传入的数组必须有序

int binarySearch(std::vector<int> &nums, int target) {
    int left = 0;
    int right = static_cast<int>(nums.size()) - 1;

    while (left <= right) {
        int mid = (right + left)/2;
        if (target == nums[mid]) {
            return mid;
        }
        else if (target < nums[mid]) {
            right = mid - 1;
        }
        else if (target > nums[mid]) {
            left = mid + 1;
        }
    }

    return -1;
}

int main() {
    std::vector<int> nums{1, 3, 5, 7, 9, 11, 13};
    int target = 11;

    int index = binarySearch(nums, target);

    if (index != -1) {
        std::cout << "Found at index: " << index << '\n';
    } else {
        std::cout << "Not found\n";
    }

    return 0;
}