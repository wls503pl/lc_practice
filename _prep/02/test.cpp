#include <map>
#include <utility>

using namespace std;

class OrderBook {

    map<int, long long> buyOrders_;
    map<int, long long> sellOrders_;

public:
    void addOrder(int price, int quantity, bool isBuy);
    void cancelOrder(int price, int quantity, bool isBuy);
    std::pair<int, int> getBestBidAsk();
    int getPriceLevelAggregation(int price);
};

void OrderBook::addOrder(int price, int quantity, bool isBuy) {
    if (price <= 0 || quantity <= 0) {
        return ;
    }

    auto & orders = isBuy ? buyOrders_ : sellOrders_;

    orders[price] += quantity;
}

void OrderBook::cancelOrder(int price, int quantity, bool isBuy) {
    if (price <= 0 || quantity <= 0) {
        return ;
    }

    auto & orders = isBuy ? buyOrders_ : sellOrders_;

    auto it = orders.find(price);
    if (it != orders.end()) {
        if (quantity >= it->second) {
            orders.erase(it);
        } else {
            it->second -= quantity;
        }
    }
}

std::pair<int, int> OrderBook::getBestBidAsk() {
    int bestBid = -1;
    int bestAsk = -1;

    if (!buyOrders_.empty()) {
        bestBid = buyOrders_.rbegin()->first;
    }

    if (!sellOrders_.empty()) {
        bestAsk = sellOrders_.begin()->first;
    }

    return {bestBid, bestAsk};
}

int OrderBook::getPriceLevelAggregation(int price) {
    if (price <= 0) {
        return 0;
    }

    int total = 0;

    auto buy_it = buyOrders_.find(price);
    if (buy_it != buyOrders_.end()) {
        total += buy_it->second;
    }

    auto sell_it = sellOrders_.find(price);
    if (sell_it != sellOrders_.end()) {
        total += sell_it->second;
    }

    return total;
}

//////////////////////// test
#include <iostream>

int main() {
    OrderBook ob;

    // 添加买单和卖单
    ob.addOrder(100, 5, true);
    ob.addOrder(95, 5, true);
    ob.addOrder(90, 10, true);
    ob.addOrder(90, 6, true);
    ob.addOrder(110, 7, false);
    ob.addOrder(105, 8, false);
    ob.addOrder(110, 8, false);

    // 预期：(100, 110)
    auto bidAsk = ob.getBestBidAsk();
    cout << "Best bid/ask: ("
         << bidAsk.first << ", " << bidAsk.second << ")\n";

    // 预期：90 价位总数量为 10；100 价位总数量为 5
    cout << "Aggregation at 90: "
         << ob.getPriceLevelAggregation(90) << '\n';
    cout << "Aggregation at 110: "
         << ob.getPriceLevelAggregation(110) << '\n';

    // 取消 100 价位的  股买单
    ob.cancelOrder(100, 5, true);

    // 预期：(95, 105)
    bidAsk = ob.getBestBidAsk();
    cout << "Best bid/ask after cancellation: ("
         << bidAsk.first << ", " << bidAsk.second << ")\n";

    // 预期：100 价位总数量为 0
    cout << "Aggregation at 100 after cancellation: "
         << ob.getPriceLevelAggregation(100) << '\n';

    // 额外测试：同价位买卖数量会被汇总
    ob.addOrder(95, 4, true);
    ob.addOrder(95, 6, false);

    // 预期：10
    cout << "Aggregation at 95 (buy + sell): "
         << ob.getPriceLevelAggregation(95) << '\n';

    return 0;
}