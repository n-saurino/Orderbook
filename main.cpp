#include <iostream>
#include <map>
#include <list>
#include <vector>
#include <memory>

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

struct TradeInfo{
    OrderId order_id_;
    Quantity quantity_;
    Price price_;
};

class Trade{
public:
    Trade(const TradeInfo& bid_trade, const TradeInfo& ask_trade): bid_trade_(bid_trade), ask_trade_(ask_trade){

    }

    TradeInfo GetBidTrade() const{
        return bid_trade_;
    }

    TradeInfo GetAskTrade() const{
        return ask_trade_;
    }

private:
    TradeInfo bid_trade_;
    TradeInfo ask_trade_;
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

class Order{
public:
    Order(OrderType order_type, OrderId order_id, Side side, Price price, Quantity quantity):
        order_type_(order_type), order_id_(order_id), side_(side), price_(price), 
        initial_quantity_(quantity), remaining_quantity_(quantity){

        }

    OrderType GetOrderType() const{
        return order_type_;
    }

    OrderId GetOrderId() const{
        return order_id_;
    }    

    Side GetSide() const{
        return side_;
    }

    Price GetPrice() const{
        return price_;
    }

    Quantity GetInitialQuantity() const{
        return initial_quantity_;
    }

    Quantity GetRemainingQuantity() const{
        return remaining_quantity_;
    }

    Quantity GetFilled() const{
        return GetInitialQuantity() - GetRemainingQuantity();
    }

    bool IsFilled() const{
        return GetRemainingQuantity() == 0;
    }

    void Fill(Quantity quantity){
        if(quantity > remaining_quantity_){
            throw std::logic_error("Attempted to fill a quantity of " + std::to_string(quantity) +
                               ", which exceeds the remaining quantity of " + std::to_string(remaining_quantity_) +
                               " in the current order.");
        }

        remaining_quantity_ -= quantity;
    }


private:
    OrderType order_type_;
    OrderId order_id_;
    Side side_;
    Price price_;
    Quantity initial_quantity_;
    Quantity remaining_quantity_;

};

typedef std::shared_ptr<Order> OrderPointer;
typedef std::list<OrderPointer> OrderPointers;

class OrderModify{
public:
    OrderModify(OrderId order_id, Side side, Price price, Quantity quantity): 
                order_id_(order_id), side_(side), price_(price), quantity_(quantity){

                } 

private:
    OrderId order_id_;
    Side side_;
    Price price_;
    Quantity quantity_;
};

int main(int, char**){
    std::cout << "Hello, from Orderbook!\n";
}
