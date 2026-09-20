#include <unordered_map>
#include <list>
#include <iostream>

using namespace std;

class LRUCache {
private:
    using Node = std::pair<int, int>;   // {key, value}

    int capacity_;
    std::list<Node> lru_;   // front 表示最近使用，back表示最近没有被使用
    unordered_map<int, list<Node>::iterator> index_;

public:
    explicit LRUCache(int capacity) : capacity_(capacity) {}

    int get(int key) {
        auto it = index_.find(key);
        if (it == index_.end()) {
            return -1;  // 不存在返回 -1
        }
        // 命中，把节点移动到链表头（复杂度 O(1)）
        lru_.splice(lru_.begin(), lru_, it->second);

        return it->second->second;
    }

    void put(int key, int value) {
        auto it = index_.find(key);
        if (it != index_.end()) {
            it->second->second = value;
            lru_.splice(lru_.begin(), lru_, it->second);
            return;
        }

        // 新的，插入 lru_ 双链表的头部
        lru_.emplace_front(key, value);
        // 写入新的键值对到哈希表（哈希表value注意是迭代器类型）
        index_[key] = lru_.begin();

        // 超出：淘汰链表尾元素（最久未使用，least recent used）
        if ((int)index_.size() > capacity_) {
            auto& [oldKey, oldValue] = lru_.back();
            index_.erase(oldKey);   // 哈希表同步删除该元素
            lru_.pop_back();
        }
    }
};

int main() {
    LRUCache cache(2); // 容量为2

    cache.put(1, 100);
    cache.put(2, 200);
    std::cout << cache.get(1) << std::endl; // 100，此时顺序：1(头) -> 2(尾)

    cache.put(3, 300); // 容量满，淘汰最久未用的 2
    std::cout << cache.get(2) << std::endl; // -1，已被淘汰

    cache.put(4, 400); // 淘汰 1
    std::cout << cache.get(1) << std::endl; // -1
    std::cout << cache.get(3) << std::endl; // 300
    std::cout << cache.get(4) << std::endl; // 400

    return 0;
}