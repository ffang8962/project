#ifndef orderbook_h
#define orderbook_h

#include <exception>
#include <string>
#include <map>

using namespace std;

struct OrderException : public std::exception {
   OrderException(const std::string& msg) 
      : msg(msg) {}

   ~OrderException() throw() {}

   const char* what() const throw () { return msg.c_str(); }

   std::string msg;
};

class OrderBook
{
public:
    typedef pair<unsigned int, unsigned int> IDSize;  // OrderId and OrderSize pair
    typedef std::multimap<double, IDSize> PriceMap;        // multimap OrderPrice -> IDSize
    typedef PriceMap::iterator PriceMapIterator; 
    typedef std::map<unsigned int, PriceMapIterator> IDMap;  // map OrderId -> PriceMapIterator, unordered_map (hash_map) will be better

    void add(char side, unsigned int id, unsigned int size, double price);

    void mod(unsigned int id, unsigned int size, double price);

    void del(unsigned int id);

    bool empty() const;

    friend ostream& operator<<(ostream& os, const OrderBook& book);
    void print(ostream& os) const;

private:

    PriceMap::const_iterator printAsk(ostream& os, const PriceMap& priceMap, PriceMap::const_iterator iter) const;
    PriceMap::const_reverse_iterator printBid(ostream& os, const PriceMap& priceMap, PriceMap::const_reverse_iterator iter) const;

    void addSide(PriceMap& priceMap, IDMap& idMap, unsigned int id, unsigned int size, double price);
    bool modSide(PriceMap& priceMap, IDMap& idMap, unsigned int id, unsigned int size, double price);
    bool delSide(PriceMap& priceMap, IDMap& idMap, unsigned int id);

    PriceMap bidPriceMap;
    IDMap    bidIDMap;
    PriceMap askPriceMap;
    IDMap    askIDMap;
};

#endif
