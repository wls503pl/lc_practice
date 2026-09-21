#include <deque>
#include <cstdint>

using namespace std;

struct Order {
    std::int64_t timestamp; // 订单自带时间戳（外部保证以非递减顺序传入）
    double price;
    double qty;
    std::int64_t id = 0;
};

class HighValueOrderLimiter {
private:
    static constexpr double kPriceThreshold = 500.0;    // 限价 500
    static constexpr std::size_t kLimit = 6;            // 窗口内笔数上限 6
    static constexpr std::int64_t kWindowMs = 1000;     // 窗口长度 1000 ms

    std::deque<Order> window_;

public:
    void allow(const Order& order) {
        if (order.price >= 500) {
            while (!window_.empty() && window_.front().timestamp <= order.timestamp - kWindowMs) {
                window_.pop_front();
            }

            if (window_.size() >= kLimit) {
                return; // 限流用 return
                // window_.pop_front() 是保留最近符合要求的 6 笔以内的订单
            }

            window_.push_back(order);
        } 
    }
};