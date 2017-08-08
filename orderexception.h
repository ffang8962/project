#include <iostream>
#include <fstream>
#include <exception>
#include <sstream> 
#include <string>
#include <map>
#include <iomanip>
#include <stdlib.h>
#include <vector>

using namespace std;

struct OrderException : public exception {
   OrderException(const string& msg) 
      : msg(msg) {}

   ~OrderException() throw() {}

   const char* what() const throw () { return msg.c_str(); }

   string msg;
};


class OrderBook
{
public:
    typedef pair<unsigned int, unsigned int> IDSize;  // OrderId and OrderSize pair
    typedef multimap<double, IDSize> PriceMap;        // multimap OrderPrice -> IDSize
    typedef PriceMap::iterator PriceMapIterator; 
    typedef map<unsigned int, PriceMapIterator> IDMap;  // map OrderId -> PriceMapIterator, unordered_map (hash_map) will be better

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

struct TrieNode
{
    TrieNode() 
      : children(26, static_cast<TrieNode*>(0))
      , orderBook(0)  
    {}

    ~TrieNode() { 
        for (unsigned int i = 0; i < children.size(); ++i) {
            if (children[i]) {
                delete children[i];
		children[i] = 0;
            }
        }
	if (orderBook) {
            delete orderBook;
	    orderBook = 0;
        } 
    }

    bool empty() const {
        for (unsigned int i = 0; i < children.size(); ++i) {
            if (children[i]) return false;
        }
	if (orderBook) return false;
	return true;
    }

    vector<TrieNode*> children;           
    OrderBook* orderBook;        // non-zero indicates that this is a leaf
};

class OrderBookManager  // implemented using Trie 
{
public:
    static OrderBookManager& instance();

    OrderBook* get(const string& symbol);         // create one if OrderBook does not exist, shared_ptr<OrderBook> will be better
    void remove(const string& symbol);
    const OrderBook* find(const string& symbol) const;
    
    void print(ostream& os, const string& symbol) const;  // print out one OrderBook
    void print(ostream& os) const;      // print out all OrderBook

private:
    TrieNode root;

    OrderBookManager() {}
    OrderBookManager(const OrderBookManager&);
    OrderBookManager& operator=(const OrderBookManager&);

    void getHelper(TrieNode*& node, const string& symbol, unsigned int i);
    bool removeHelper(TrieNode* node, const string& symbol, unsigned int i);
    const OrderBook* findHelper(const TrieNode* node, const string& symbol, unsigned int i) const;
    void printHelper(ostream& os, const TrieNode* node, const string& s) const; 
};


//-----------------------------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------------------------

OrderBookManager& OrderBookManager::instance()
{
   static OrderBookManager orderBookManager;
   return orderBookManager;
}

void OrderBookManager::getHelper(TrieNode*& node, const string& symbol, unsigned int i)
{
    vector<TrieNode*>& children = node->children;
    unsigned int index = static_cast<unsigned int>( symbol[i] - 'A' );

    if (children[index] == 0) children[index] = new TrieNode();

    node = children[index];

    if (i == symbol.size()-1) { // this is the word
        if (node->orderBook == 0) node->orderBook = new OrderBook();
    }
    else { 
        getHelper(node, symbol, i+1); 
    }
}

OrderBook* OrderBookManager::get(const string& symbol)
{
    if (symbol.empty()) throw OrderException("Empty symbol when get OrderBook");
   
    TrieNode* node(&root); 
    getHelper(node, symbol, 0);

    return node->orderBook;
}

bool OrderBookManager::removeHelper(TrieNode* node, const string& symbol, unsigned int i)
{
    vector<TrieNode*>& children = node->children;
    unsigned int index = static_cast<unsigned int>( symbol[i] - 'A' );

    bool res(false);
    if (children[index] != 0) {

        node = children[index];

        if (i == symbol.size()-1) { // this is the word
            if (node->orderBook) {
                delete node->orderBook;
		node->orderBook = 0;
            }
            res = true;
        }
        else { 
            res = removeHelper(node, symbol, i+1); 
        }

        if (res && node->empty()) {
            delete node;
	    children[index] = 0;	
	}	    
	else res = false;
    }    

    return res;
}

void OrderBookManager::remove(const string& symbol)
{
    if (!symbol.empty()) {
        TrieNode* node(&root); 
        removeHelper(node, symbol, 0);
    }
}
    
const OrderBook* OrderBookManager::findHelper(const TrieNode* node, const string& symbol, unsigned int i) const
{
    const vector<TrieNode*>& children = node->children;
    unsigned int index = static_cast<unsigned int>( symbol[i] - 'A' );

    const OrderBook* orderBook(0);
    if (children[index] != 0) {  

        node = children[index];

        if (i == symbol.size()-1) { // this is the word
            if (node->orderBook) orderBook = node->orderBook;
        }
        else { 
            orderBook = findHelper(node, symbol, i+1); 
        }
    } 

    return orderBook;
}

const OrderBook* OrderBookManager::find(const string& symbol) const
{
    const OrderBook* orderBook(0);
    if (!symbol.empty()) {
        const TrieNode* node(&root);
        orderBook = findHelper(node, symbol, 0);
    }
    return orderBook;
}

void OrderBookManager::print(ostream& os, const string& symbol) const
{
    const OrderBook* orderBook = find(symbol);
    if (orderBook) {
        os << "Ticker: " << symbol << endl;
        os << *orderBook;
    }
    else {
        os << "Ticker: " << symbol << " does not exist" << endl;
    }
}

void OrderBookManager::printHelper(ostream& os, const TrieNode* node, const string& s) const
{
    const vector<TrieNode*>& children = node->children;

    if (node->orderBook) {
        os << "Ticker: " << s  << endl << *(node->orderBook) << endl;
    }

    for (unsigned int i = 0; i < children.size(); ++i) {
        const TrieNode* node = children[i];
        if (node) printHelper(os, node, s + static_cast<char>('A' + i) );
    }
}

void OrderBookManager::print(ostream& os) const
{
    printHelper(os, &root, "");    
}

//-----------------------------------------------------------------------------------------------

string next(stringstream& ss) 
{
    string token;
    getline(ss, token, '|'); 
    if (token.empty()) throw OrderException("bad input line");
    return token;
}

void processLine(const string& line)
{
    stringstream ss(line);

    string symbol = next(ss);
    char op = next(ss)[0];
    char side(0);
    if (op == 'A') 
        side = next(ss)[0];
    unsigned int id = static_cast<unsigned int>(atoi( next(ss).c_str() ));
    unsigned int size(0);
    double price(0.0);
    if (op == 'A' || op == 'M') {
        size = static_cast<unsigned int>(atoi( next(ss).c_str() ));
        price = atof( next(ss).c_str() );
    }

    //cout << symbol << "|" << op << "|" << side << "|" << id << "|" << size << "|" << price << endl;

    OrderBook* orderBook(0);
    try {	 
        orderBook = OrderBookManager::instance().get(symbol);

        if (op == 'A')
            orderBook->add(side, id, size, price);
        else if (op == 'M')
            orderBook->mod(id, size, price);
        else if (op == 'D') {
            orderBook->del(id);

            // not sure if removal is necessary, but implemented here anyway 
	    if (orderBook->empty()) {
		orderBook = 0;
                OrderBookManager::instance().remove(symbol);
	    }		
	}
	else throw OrderException("bad operation");

        // This will printout OrderBook
	//if (orderBook) cout << *orderBook << endl;
    }
    catch (OrderException& ex) {
        cout << ex.what() << ":" << line << endl;
        if (orderBook) cout << *orderBook << endl;
    }
}

// g++ -Wall -Werror -o orderbook orderbook.cpp
int main(int argc, char *argv[])
{
    try {
        if (argc != 2) { 
            cout << "Need an input filename argument" << endl; 
	    return 1;
        }

        ifstream ifs(argv[1], std::ifstream::in);
        if (!ifs.good()) { 
            cout << "Can not open the file " << argv[1] << endl; 
	    return 1;
        }

        string line;
        while (getline(ifs, line)) {
            try {
                processLine(line);
            }
	    catch (exception& e) {
                cout << e.what() << ":" << line << endl;
            }
	}

        ifs.close();

        // This will printout one OrderBook
        //OrderBookManager::instance().print(cout, "ABB");

        // This will printout all OrderBooks
        //OrderBookManager::instance().print(cout);
    }
    catch (exception& e) {
        cout << "Exception: " << e.what() << endl;
        return 1;
    }
    catch (...) {
        cout << "Unexpected Exception" << endl;
        return 1;
    }
    
    return 0;
}
