#include <iostream>
#include <map>
#include <list>
#include <vector>

enum class OrderType{
    GoodTillCancel,
    FillAndKill
};

enum class Side{
    Bid,
    Ask
};

typedef int32_t Price;
typedef uint32_t Quantity;
typedef uint64_t OrderId;

struct LevelInfo{
    Price price_;
    Quantity quantity_;
};

typedef std::vector<LevelInfo> LevelInfos;

struct Trade{
    OrderId order_id_;
    Quantity quantity_;
    Price price_;
};

typedef std::vector<Trade> Trades;

class OrderbookLevelInfos{
public:
    OrderbookLevelInfos(const LevelInfos& bids, const LevelInfos& asks): bids_(bids), asks_(asks){

    }

    const LevelInfos& GetBids() const{
        return bids_;
    }

    const LevelInfos& GetAsks() const{
        return asks_;
    }
    
private:
    LevelInfos bids_;
    LevelInfos asks_;
};

int main(int, char**){
    std::cout << "Hello, from Orderbook!\n";
}
