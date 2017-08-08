#include <iostream>
#include <sstream> 
#include <string>
#include <map>
#include <iomanip>
#include <stdlib.h>
#include <vector>
#include "orderbook.h"

using namespace std;

ostream& operator<<(ostream& os, const OrderBook& book)
{
    book.print(os);
    return os;
}

OrderBook::PriceMap::const_iterator OrderBook::printAsk(ostream& os, const PriceMap& priceMap, PriceMap::const_iterator iter) const
{
    double price = iter->first;
    unsigned int orderCount(0);
    unsigned int totalSize(0);
    for ( ; iter != priceMap.end(); iter++) {
        if (price == iter->first)  {
            ++orderCount;
            totalSize += iter->second.second;
	}
	else break;
    }

    os << setw(15) << fixed << setprecision(4) << price << setw(15) << totalSize  << setw(15) << orderCount;
    return iter;
}

OrderBook::PriceMap::const_reverse_iterator OrderBook::printBid(ostream& os, const PriceMap& priceMap, PriceMap::const_reverse_iterator iter) const
{
    double price = iter->first;
    unsigned int orderCount(0);
    unsigned int totalSize(0);
    for ( ; iter != priceMap.rend(); iter++) {
        if (price == iter->first)  {
            ++orderCount;
            totalSize += iter->second.second;
	}
	else break;
    }

    os << setw(15) << orderCount << setw(15) << totalSize  << setw(15) << fixed << setprecision(4) << price;
    return iter;
}

void OrderBook::print(ostream& os) const
{
    os << setw(45) << "Bid" << setw(13) <<  "Ask" << endl;
    os << setw(15) << "OrderCount" << setw(15) << "TotalSize"  << setw(15) << "Price"
       << setw(15) << "-     Price" << setw(15) << "TotalSize"  << setw(15) << "OrderCount" << endl;

    PriceMap::const_reverse_iterator bidIter = bidPriceMap.rbegin();
    PriceMap::const_iterator askIter = askPriceMap.begin();

    while (bidIter != bidPriceMap.rend()) {
        bidIter = printBid(os, bidPriceMap, bidIter);
        if (askIter != askPriceMap.end()) {
            askIter = printAsk(os, askPriceMap, askIter);
	}	
	os << endl;
    } 
    while (askIter != askPriceMap.end()) {
        os << setw(45) << " ";
        askIter = printAsk(os, askPriceMap, askIter);
	os << endl;
    }
}

void OrderBook::add(char side, unsigned int id, unsigned int size, double price)
{
    if (bidIDMap.find(id) != bidIDMap.end() || askIDMap.find(id) != askIDMap.end()) {
        throw OrderException("OrderID already existed when Add");
    } 

    if (side == 'B') addSide(bidPriceMap, bidIDMap, id, size, price);
    else if (side == 'S') addSide(askPriceMap, askIDMap, id, size, price);
    else throw OrderException("Bad side when Add");
}

void OrderBook::addSide(PriceMap& priceMap, IDMap& idMap, unsigned int id, unsigned int size, double price)
{
    PriceMap::iterator iter = priceMap.insert( make_pair( price, make_pair(id, size) ) );
    idMap[id] = iter;
}

void OrderBook::mod(unsigned int id, unsigned int size, double price)
{
    if ( modSide(bidPriceMap, bidIDMap, id, size, price) ||
         modSide(askPriceMap, askIDMap, id, size, price) ) ;
    else throw OrderException("OrderID did not exist when Mod");
}

bool OrderBook::modSide(PriceMap& priceMap, IDMap& idMap, unsigned int id, unsigned int size, double price)
{
    IDMap::iterator idIter = idMap.find(id);
    if (idIter != idMap.end()) {
       
	PriceMap::iterator priceIter = idIter->second;

        double oldPrice = priceIter->first;
        if (oldPrice != price) {
       
            priceMap.erase(priceIter);
	    priceIter = priceMap.insert(make_pair(price, make_pair(id, size)));
            idMap[id] = priceIter;
	}
        else {
            priceIter->second.second = size;
	}

	return true;
    }	
    return false;
}

void OrderBook::del(unsigned int id)
{
    if ( delSide(bidPriceMap, bidIDMap, id) ||
         delSide(askPriceMap, askIDMap, id) ) ;
    else throw OrderException("OrderID did not exist when Del");
}

bool OrderBook::delSide(PriceMap& priceMap, IDMap& idMap, unsigned int id)
{
    IDMap::iterator idIter = idMap.find(id);
    if (idIter != idMap.end()) {
     
        PriceMap::iterator priceIter = idIter->second;
        priceMap.erase(priceIter);
        idMap.erase(idIter);
	
	return true;
    }	
    return false;

}

bool OrderBook::empty() const
{
    return bidPriceMap.empty() && bidIDMap.empty() && askPriceMap.empty() && askIDMap.empty();
}

