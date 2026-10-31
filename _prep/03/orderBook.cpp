#include <cassert>
#include <cstdint>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct Order {
    uint64_t order_id;
    string side;    // "BUY" or "SELL"
    int price;
    int quantity;
};

struct TopOfBook {
    std::optional<int> best_buy;
    std::optional<int> best_sell;
};

class OrderBook {
    using OrderList = list<Order>;
    using OrderIterator = OrderList::iterator;

    struct Location {
        bool is_buy;
        int price;
        OrderIterator order_it;
    };

    map<int, OrderList, greater<int>> bids_;
    map<int, OrderList> asks_;

    unordered_map<uint64_t, Location> by_id_;
    mutable mutex mutex_;

public:
    // 重复 id，非法方向或超出题目范围时返回 false，不修改订单簿
    bool addOrder(uint64_t order_id, const string & side, int price, int quantity) {
        if ((side != "BUY" && side != "SELL") || price < 1 || price > 1000000 || quantity < 1 || quantity > 10000) {
            return false;
        }

        lock_guard<mutex> lock(mutex_);
        if (by_id_.find(order_id) != by_id_.end()) {
            return false;
        }

        const bool is_buy = side == "BUY";
        if (is_buy) {
            auto & level = bids_[price];
            level.emplace_back(Order{order_id, side, price, quantity});
            auto order_it = prev(level.end());
            by_id_.emplace(order_id, Location{true, price, order_it});
        } else {
            auto & level = asks_[price];
            level.emplace_back(Order{order_id, side, price, quantity});
            auto order_it = prev(level.end());
            by_id_.emplace(order_id, Location{false, price, order_it});
        }

        return true;
    }

    bool cancelOrder(uint64_t order_id) {
        lock_guard<mutex> lock(mutex_);

        auto found = by_id_.find(order_id);
        if (found == by_id_.end()) {
            return false;
        }

        const Location location = found->second;
        if (location.is_buy) {
            auto level = bids_.find(location.price);
            level->second.erase(location.order_it);
            if (level->second.empty()) {
                bids_.erase(level);
            }
        } else {
            auto level = asks_.find(location.price);
            level->second.erase(location.order_it);
            if (level->second.empty()) {
                asks_.erase(level);
            }
        }
        by_id_.erase(found);
        return true;
    }

    TopOfBook queryTopOfBook() const {
        lock_guard<mutex> lock(mutex_);
        TopOfBook result;

        if (!bids_.empty()) {
            result.best_buy = bids_.begin()->first;
        }
        if (!asks_.empty()) {
            result.best_sell = asks_.begin()->first;
        }

        return result;
    }

    // 返回独立snapshot, 买单优先，然后卖单；同价订单保持 FIFO 顺序
    std::vector<Order> snapshot() const {
        lock_guard<mutex> lock(mutex_);
        vector<Order> result;
        result.reserve(by_id_.size());

        for (const auto & [price, orders] : bids_) {
            (void)price;
            result.insert(result.end(), orders.begin(), orders.end());
        }

        for (const auto & [price, orders] : asks_) {
            (void)price;
            result.insert(result.end(), orders.begin(), orders.end());
        }

        return result;
    }

    size_t size() const {
        lock_guard<mutex> lock(mutex_);
        return by_id_.size();
    }
};

//////////////////////////////
// 单文件示例和简单测试。作为在线笔试答案时，可按平台要求保留或删除 main。
int main() {
    OrderBook ob;

    assert(ob.addOrder(1, "BUY", 100, 5));
    assert(ob.addOrder(2, "SELL", 105, 10));
    assert(ob.addOrder(3, "BUY", 101, 15));

    auto top = ob.queryTopOfBook();
    assert(top.best_buy && *top.best_buy == 101);
    assert(top.best_sell && *top.best_sell == 105);

    assert(ob.cancelOrder(3));
    top = ob.queryTopOfBook();
    assert(top.best_buy && *top.best_buy == 100);
    assert(top.best_sell && *top.best_sell == 105);

    const auto saved = ob.snapshot();
    assert(saved.size() == 2);
    assert(saved[0].order_id == 1);
    assert(saved[1].order_id == 2);

    assert(!ob.addOrder(1, "BUY", 99, 1));  // 重复 ID
    assert(!ob.cancelOrder(999));            // 不存在的订单

    std::cout << "All order book tests passed.\n";
    return 0;
}