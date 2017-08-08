#ifndef orderbookmanager_h
#define orderbookmanager_h

#include <string>
#include <iostream>
#include <vector>
#include "orderbook.h"

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

    std::vector<TrieNode*> children;           
    OrderBook* orderBook;        // non-zero indicates that this is a leaf
};

class OrderBookManager  // implemented using Trie 
{
public:
    static OrderBookManager& instance();

    OrderBook* get(const std::string& symbol);         // create one if OrderBook does not exist, shared_ptr<OrderBook> will be better
    void remove(const std::string& symbol);
    const OrderBook* find(const std::string& symbol) const;
    
    void print(std::ostream& os, const std::string& symbol) const;  // print out one OrderBook
    void print(std::ostream& os) const;      // print out all OrderBook

private:
    TrieNode root;

    OrderBookManager() {}
    OrderBookManager(const OrderBookManager&);
    OrderBookManager& operator=(const OrderBookManager&);

    void getHelper(TrieNode*& node, const std::string& symbol, unsigned int i);
    bool removeHelper(TrieNode* node, const std::string& symbol, unsigned int i);
    const OrderBook* findHelper(const TrieNode* node, const std::string& symbol, unsigned int i) const;
    void printHelper(std::ostream& os, const TrieNode* node, const std::string& s) const; 
};

#endif
