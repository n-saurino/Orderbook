#include <iostream>
#include <map>
#include <unordered_map>
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

class Orderbook{
public:
    Orderbook(){

    }

private:
    struct OrderEntry{
        OrderPointer order_;
        OrderPointers::iterator location_;
    };

    std::map<Price, OrderPointers, std::greater<Price>> bids_;
    std::map<Price, OrderPointers, std::less<Price>> asks_;
    std::unordered_map<OrderId, OrderEntry> orders_;

    bool CanMatch(Side side, Price price){
        if(side == Side::Bid){
            // no asks to match with our bid order
            if(asks_.empty()){
                return false;
            }

            // split the pair at the top of the asks map
            auto& [ask_price, _] = *asks_.begin();
            
            return price >= ask_price ? 1 : 0;
        
        }else if(side == Side::Ask){
            // no bids to match with our ask order
            if(bids_.empty()){
                return false;
            }

            // split the pair at the top of the asks map
            auto& [bid_price, _] = *bids_.begin();

            return price <= bid_price ? 1 : 0;
        }
    }

    Trades MatchOrders(){
        Trades trades;
        trades.reserve(orders_.size());

        while(true){
            if(bids_.empty() || asks_.empty()){
                break;
            }

            auto& [bid_price, bids] = *bids_.begin();
            auto& [ask_price, asks] = *asks_.begin();

            // there is no one crossing the market
            if(bid_price < ask_price){
                break;
            }

            while(!bids.empty() && !asks.empty()){
                OrderPointer bid = bids.front();
                OrderPointer ask = asks.front();
                
                Quantity min_fill_quantity = std::min(ask->GetRemainingQuantity(), bid->GetRemainingQuantity());
                
                bid->Fill(min_fill_quantity);
                ask->Fill(min_fill_quantity);

                if(bid->IsFilled()){    
                    bids.pop_front();
                    orders_.erase(bid->GetOrderId());
                }

                if(ask->IsFilled()){
                    asks.pop_front();
                    orders_.erase(ask->GetOrderId());
                }

                if(bids.empty()){
                    bids_.erase(bid_price);
                }

                if(asks.empty()){
                    asks_.erase(ask_price);
                }

                trades.push_back(Trade{TradeInfo{bid->GetOrderId(), bid->GetFilled(), bid_price}, 
                                        TradeInfo{ask->GetOrderId(), ask->GetFilled(), ask_price}});
            }


            OrderPointer order = nullptr;

            // Only one of these if statements should trip
            if(!bids_.empty()){
                auto& [bid_price, bids] = *bids_.begin();
                order = bids.front();
            }

            // Only one of these if statements should trip
            if(!asks_.empty()){
                auto& [ask_price, asks] = *asks_.begin();
                order = asks.front();
            }

            if(order->GetOrderType() == OrderType::FillAndKill){
                CancelOrder(order->GetOrderType());
            }    
        }
        return trades;
    }

public:
    Trades AddOrder(OrderPointer order){
        if(orders_.contains(order->GetOrderId())){
            return{};
        }

        if(order->GetOrderType() == OrderType::FillAndKill && !CanMatch(order->GetSide(), order->GetPrice())){
            return {};
        }

        OrderPointers::iterator iterator;

        if(order->GetSide() == Side::Bid){
            auto& orders = bids_[order->GetPrice()];
            orders.push_back(order);
            iterator = std::next(orders.begin(), orders.size() - 1);
        }else{
            auto& orders = asks_[order->GetPrice()];
            orders.push_back(order);
            iterator = std::next(orders.begin(), orders.size() - 1);
        }

        orders_.insert({order->GetOrderId(), OrderEntry(order, iterator)});
        return MatchOrders();
    }

    void CancelOrder(OrderId order_id){
        if(!orders_.contains(order_id)){
            return;
        }
        
        const auto& [order, order_iterator] = orders_.at(order_id);
        orders_.erase(order_id);

        if(order->GetSide() == Side::Ask){
            auto price = order->GetPrice();
            auto& orders = asks_.at(price);
            orders.erase(order_iterator);
            if(orders.empty()){
                asks_.erase(price);
            }
        }else{
            auto price = order->GetPrice();
            auto& orders = bids_.at(price);
            if(orders.empty()){
                bids_.erase(price);
            }

        }
    }

};

int main(int, char**){
    std::cout << "Hello, from Orderbook!\n";
}
