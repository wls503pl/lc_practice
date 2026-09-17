#include <algorithm>
#include <bitset>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using SubscribersPerNews = std::unordered_map<long, std::vector<long>>;

std::vector<std::string> split(const std::string&);
std::string print(const SubscribersPerNews&);
std::string print(const bool);

struct Subscription
{
    long id = 0, minInterest = 0, maxNewsPerSecond = 0;
    std::unordered_set<std::string> topics;
    std::deque<double> deliveryTimes;      // sliding-window rate limit
    std::unordered_set<long> notified;     // never send same news twice
};

struct NewsItem
{
    long id = 0, interest = 0;
    double timestamp = 0.0;
    std::vector<std::string> topics;
};

class NewsProvider
{
public:
    bool AddSubscription(long id, long minInterest, long maxNewsPerSecond,
                          const std::vector<std::string>& topics)
    {
        if (id < 1 || id >= (1LL << 32)) return false;
        if (minInterest < 1 || minInterest >= (1LL << 32)) return false;
        if (maxNewsPerSecond < 1 || maxNewsPerSecond >= (1LL << 12)) return false;
        if (topics.empty() || (long long)topics.size() >= (1LL << 10)) return false;

        Subscription& s = subs_[id];
        s.id = id;
        s.minInterest = minInterest;
        s.maxNewsPerSecond = maxNewsPerSecond;
        s.topics = std::unordered_set<std::string>(topics.begin(), topics.end());
        s.deliveryTimes.clear();
        return true;
    }

    bool RemoveSubscription(long id)
    {
        return subs_.erase(id) > 0;
    }

    bool NewsReceived(long id, double timestamp, long interest,
                       const std::vector<std::string>& topics)
    {
        if (id < 1 || id >= (1LL << 32)) return false;
        if (interest < 1 || interest >= (1LL << 32)) return false;
        if (topics.empty() || (long long)topics.size() >= (1LL << 10)) return false;
        if (news_.count(id)) return false;

        news_[id] = {id, interest, timestamp, topics};
        pending_.push_back(id);
        return true;
    }

    SubscribersPerNews Publish(double timestamp, double maxAge)
    {
        SubscribersPerNews result;
        if (maxAge <= 0.0 || maxAge >= (1LL << 32)) return result;

        // Keep news whose age is still within maxAge (or not yet due);
        // drop anything that's aged out for good.
        std::vector<long> eligible, stillPending;
        for (long id : pending_)
        {
            double age = timestamp - news_.at(id).timestamp;
            if (age < 0.0) { stillPending.push_back(id); continue; }
            if (age > maxAge) continue;
            eligible.push_back(id);
            stillPending.push_back(id);
        }
        pending_.swap(stillPending);

        // Priority: highest interest, then earliest timestamp, then highest id.
        std::sort(eligible.begin(), eligible.end(), [this](long a, long b)
        {
            const NewsItem& ia = news_.at(a);
            const NewsItem& ib = news_.at(b);
            if (ia.interest != ib.interest) return ia.interest > ib.interest;
            if (ia.timestamp != ib.timestamp) return ia.timestamp < ib.timestamp;
            return ia.id > ib.id;
        });

        for (auto& [subId, s] : subs_)
        {
            while (!s.deliveryTimes.empty() && s.deliveryTimes.front() < timestamp - 1.0)
                s.deliveryTimes.pop_front();

            for (long id : eligible)
            {
                if ((long)s.deliveryTimes.size() >= s.maxNewsPerSecond) break;
                if (s.notified.count(id)) continue;

                const NewsItem& n = news_.at(id);
                if (n.interest < s.minInterest) continue;
                bool overlap = false;
                for (const auto& t : n.topics)
                    if (s.topics.count(t)) { overlap = true; break; }
                if (!overlap) continue;

                result[id].push_back(subId);
                s.notified.insert(id);
                s.deliveryTimes.push_back(timestamp);
            }
        }
        return result;
    }

private:
    std::map<long, Subscription> subs_;
    std::unordered_map<long, NewsItem> news_;
    std::vector<long> pending_;
};

int main()
{
    NewsProvider provider;

    std::string line;
    while(std::getline(std::cin, line))
    {
        if(line.empty())
            continue;
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        std::vector<std::string> args;
        std::string token;
        while (iss >> token)
        {
            args.push_back(token);
        }

        if (command == "subscribe")
        {
            if(args.size() < 4)
            {
                std::cerr << "Malformed input: " << line << std::endl;
                std::exit(-1);
            }
            long id = std::stol(args[0]);
            long minInterest = std::stol(args[1]);
            long maxNewsPerSecond = std::stol(args[2]);
            std::vector<std::string> topics;
            for (size_t i = 3; i < args.size(); i++)
            {
                topics.push_back(args[i]);
            }

            bool subscribed = provider.AddSubscription(id, minInterest, maxNewsPerSecond, topics);
            std::cout << "subscribed=" << print(subscribed) << std::endl;
        }
        else if (command == "unsubscribe")
        {
            if(args.size() != 1)
            {
                std::cerr << "Malformed input: " << line << std::endl;
                std::exit(-1);
            }
            long id = std::stol(args[0]);

            bool unsubscribed = provider.RemoveSubscription(id);
            std::cout << "unsubscribed=" << print(unsubscribed) << std::endl;
        }
        else if (command == "news")
        {
            if(args.size() < 4)
            {
                std::cerr << "Malformed input: " << line << std::endl;
                std::exit(-1);
            }
            long id = std::stol(args[0]);
            double timestamp = std::stod(args[1]);
            long interest = std::stol(args[2]);
            std::vector<std::string> topics;
            for (size_t i = 3; i < args.size(); i++)
            {
                topics.push_back(args[i]);
            }

            bool news_received = provider.NewsReceived(id, timestamp, interest, topics);
            std::cout << "news_received=" << print(news_received) << std::endl;
        }
        else if (command == "publish")
        {
            if(args.size() != 2)
            {
                std::cerr << "Malformed input: " << line << std::endl;
                std::exit(-1);
            }
            double timestamp = std::stod(args[0]);
            double maxAgeInMs = std::stod(args[1]);

            SubscribersPerNews subscribersPerNews = provider.Publish(timestamp, maxAgeInMs);
            std::cout << "publish:" << std::endl;
            std::cout << print(subscribersPerNews);
        }
        else
        {
            std::cerr << "Malformed input! " << command << std::endl;
            std::exit(-1);
        }
    }
    return 0;
}

std::vector<std::string> split(const std::string& str)
{
    std::vector<std::string> result;
    std::stringstream stream(str);

    std::string word;
    while (stream >> word)
    {
        result.emplace_back(word);
    }

    return result;
}

std::string print(const SubscribersPerNews& subscribersPerNews)
{
    std::ostringstream oss;
    if (subscribersPerNews.empty())
    {
        oss << "none" << std::endl;
    }
    else
    {
        std::map<long, std::vector<long>> sortedMap(subscribersPerNews.begin(), subscribersPerNews.end());
        for (auto& [newsId, subscriptionIds] : sortedMap) {
            oss << "- news=" << newsId << " to [";
            std::sort(subscriptionIds.begin(), subscriptionIds.end());
            for (size_t i = 0; i < subscriptionIds.size(); ++i)
            {
                oss << subscriptionIds[i] << (i + 1 < subscriptionIds.size() ? ", " : "");
            }
            oss << "]" << std::endl;
        }
    }
    return oss.str();
}

std::string print(const bool value)
{
    return value ? "True" : "False";
}
