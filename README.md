# Order-Matching-System

A stock exchange-style order book implementation that matches buy/sell orders using price-time priority.

## Features
- **Order Types**  
  - Limit Orders (Buy/Sell with specified price)  
  - Market Orders (Buy/Sell at best available price)  
- **Matching Algorithm**  
  - Price-Time Priority (best price first, then earliest timestamp)  
  - Partial order fulfillment support  
  - Last traded price tracking for market orders  
- **Logging**  
  - Trade execution records  
  - Unmatched order reporting  

## Build Instructions

### Requirements
- C++17 compiler (g++/clang/MSVC)


