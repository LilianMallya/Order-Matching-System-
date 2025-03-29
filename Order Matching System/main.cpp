#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <queue>
#include <iomanip>
#include <memory>
#include <limits>
#include <iomanip>

// Abstract Base Class for Order
class Order {
protected:
    std::string id;
    char type; // B for buy, S for sell
    int quantity;
    double price;
    int timestamp;

public:
    // Constructor to initialize an order
    Order(const std::string& id, char type, int quantity, double price, int timestamp)
        : id(id), type(type), quantity(quantity), price(price), timestamp(timestamp) {}

    virtual ~Order() = default; // Virtual destructor

    const std::string& getId() const { return id; }
    char getType() const { return type; }
    int getQuantity() const { return quantity; }
    double getPrice() const { return price; }
    int getTimestamp() const { return timestamp; }

    void reduceQuantity(int amount) { quantity -= amount; }

    virtual std::string toString() const = 0;
    
    // Overload comparison operator
    bool operator<(const Order& other) const {
        if (type == 'B') {
            return (price < other.price) || (price == other.price && timestamp > other.timestamp);
        } else {
            return (price > other.price) || (price == other.price && timestamp > other.timestamp);
        }
    }
};

class BuyOrder : public Order {
public:
    BuyOrder(const std::string& id, int quantity, double price, int timestamp)
        : Order(id, 'B', quantity, price, timestamp) {}

    std::string toString() const override {
        return "Buy Order: " + id + " " + std::to_string(quantity) + " @ " + std::to_string(price);
    }
};

class SellOrder : public Order {
public:
    SellOrder(const std::string& id, int quantity, double price, int timestamp)
        : Order(id, 'S', quantity, price, timestamp) {}

    std::string toString() const override {
        return "Sell Order: " + id + " " + std::to_string(quantity) + " @ " + std::to_string(price);
    }
};

class OrderBook {
private:
    std::priority_queue<std::shared_ptr<Order>, std::vector<std::shared_ptr<Order>>, 
                        std::function<bool(const std::shared_ptr<Order>&, const std::shared_ptr<Order>&)>> buyQueue;
    std::priority_queue<std::shared_ptr<Order>, std::vector<std::shared_ptr<Order>>, 
                        std::function<bool(const std::shared_ptr<Order>&, const std::shared_ptr<Order>&)>> sellQueue;

    std::queue<std::shared_ptr<Order>> unmatchedBuyOrders;
    std::queue<std::shared_ptr<Order>> unmatchedSellOrders;

public:
    OrderBook()
        : buyQueue([](const std::shared_ptr<Order>& a, const std::shared_ptr<Order>& b) {
              return (*a < *b);
          }),
          sellQueue([](const std::shared_ptr<Order>& a, const std::shared_ptr<Order>& b) {
              return (*b < *a);
          }) {}
    
    // Method to add order
    void addOrder(const std::shared_ptr<Order>& order) {
        if (order->getType() == 'B') {
            buyQueue.push(order);
        } else {
            sellQueue.push(order);
        }
    }
    
    // Method for order display
    void displayOrders(double lastTradePrice) const {
        std::cout << "Last trading price: " << std::fixed << std::setprecision(2) << lastTradePrice << std::endl;
        std::cout << "Buy                       Sell" << std::endl;
        std::cout << "------------------------------------------" << std::endl;

        auto buyQueueCopy = buyQueue;
        auto sellQueueCopy = sellQueue;

        while (!buyQueueCopy.empty() || !sellQueueCopy.empty()) {
            std::string buyLine = "", sellLine = "";

            if (!buyQueueCopy.empty()) {
                auto order = buyQueueCopy.top();
                buyQueueCopy.pop();
                buyLine = order->getId() + " " + (order->getPrice() == -1 ? "M" : formatPrice(order->getPrice())) + " " + std::to_string(order->getQuantity());
            }

            if (!sellQueueCopy.empty()) {
                auto order = sellQueueCopy.top();
                sellQueueCopy.pop();
                sellLine = order->getId() + " " + (order->getPrice() == -1 ? "M" : formatPrice(order->getPrice())) + " " + std::to_string(order->getQuantity());
            }

            std::cout << std::left << std::setw(25) << buyLine << sellLine << std::endl;
        }
    }
    
    // Method to match orders
    void matchOrders(std::vector<std::string>& tradeLog, double& lastTradePrice) {
        while (!buyQueue.empty() && !sellQueue.empty()) {
            auto buyOrder = buyQueue.top();
            auto sellOrder = sellQueue.top();

            if (buyOrder->getPrice() >= sellOrder->getPrice() || buyOrder->getPrice() == -1 || sellOrder->getPrice() == -1) {
                int tradedQuantity = std::min(buyOrder->getQuantity(), sellOrder->getQuantity());
                double tradePrice;

                if (buyOrder->getPrice() == -1 && sellOrder->getPrice() == -1) {
                    tradePrice = lastTradePrice;
                } else if (buyOrder->getPrice() == -1) {
                    tradePrice = sellOrder->getPrice();
                } else if (sellOrder->getPrice() == -1) {
                    tradePrice = buyOrder->getPrice();
                } else {
                    tradePrice = buyOrder->getTimestamp() < sellOrder->getTimestamp() ? buyOrder->getPrice() : sellOrder->getPrice();
                }

                tradeLog.push_back("order " + buyOrder->getId() + " " + std::to_string(tradedQuantity) +
                                   " shares purchased at price " + formatPrice(tradePrice));
                tradeLog.push_back("order " + sellOrder->getId() + " " + std::to_string(tradedQuantity) +
                                   " shares sold at price " + formatPrice(tradePrice));

                lastTradePrice = tradePrice;
                buyOrder->reduceQuantity(tradedQuantity);
                sellOrder->reduceQuantity(tradedQuantity);

                if (buyOrder->getQuantity() == 0) buyQueue.pop();
                if (sellOrder->getQuantity() == 0) sellQueue.pop();

            } else {
                break;
            }
        }
    }
    
    // Method for unmatched orders
    void handleUnmatchedOrders(std::vector<std::string>& unmatchedLog) {
        while (!buyQueue.empty()) {
            unmatchedBuyOrders.push(buyQueue.top());
            buyQueue.pop();
        }

        while (!sellQueue.empty()) {
            unmatchedSellOrders.push(sellQueue.top());
            sellQueue.pop();
        }

        while (!unmatchedBuyOrders.empty()) {
            auto order = unmatchedBuyOrders.front();
            unmatchedBuyOrders.pop();
            unmatchedLog.push_back("order " + order->getId() + " " + std::to_string(order->getQuantity()) + " shares unexecuted");
        }

        while (!unmatchedSellOrders.empty()) {
            auto order = unmatchedSellOrders.front();
            unmatchedSellOrders.pop();
            unmatchedLog.push_back("order " + order->getId() + " " + std::to_string(order->getQuantity()) + " shares unexecuted");
        }
    }

    static std::string formatPrice(double price) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << price;
        return oss.str();
    }
};

// Parsing an order
std::shared_ptr<Order> parseOrder(const std::string& line, int timestamp) {
    std::istringstream iss(line);
    std::string id;
    char type;
    int quantity;
    double price = -1;
    iss >> id >> type >> quantity;
    if (!(iss >> price)) price = -1;

    if (type == 'B') {
        return std::make_shared<BuyOrder>(id, quantity, price, timestamp);
    } else {
        return std::make_shared<SellOrder>(id, quantity, price, timestamp);
    }
}

int main() {
    std::ifstream inputFile("sample_input_3.txt");
    if (!inputFile.is_open()) {
       
        return 1;
    }

    double marketPrice;
    inputFile >> marketPrice;
    inputFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    OrderBook orderBook;
    std::vector<std::string> tradeLog;
    std::vector<std::string> unmatchedLog;

    std::string line;
    int timestamp = 0;
    double lastTradePrice = marketPrice;

    while (std::getline(inputFile, line)) {
        if (!line.empty()) {
            auto order = parseOrder(line, timestamp++);
            orderBook.addOrder(order);
            orderBook.displayOrders(lastTradePrice);
            orderBook.matchOrders(tradeLog, lastTradePrice);
            orderBook.displayOrders(lastTradePrice);
        }
    }

    inputFile.close();
    orderBook.handleUnmatchedOrders(unmatchedLog);

    std::ofstream outputFile("output.txt");
    for (const auto& trade : tradeLog) {
        std::cout << trade << std::endl;
        outputFile << trade << std::endl;
    }
    for (const auto& unmatched : unmatchedLog) {
        std::cout << unmatched << std::endl;
        outputFile << unmatched << std::endl;
    }

    outputFile.close();
    return 0;
}
