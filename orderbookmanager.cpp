#include <iostream>
#include <string>
#include <map>
#include <iomanip>
#include <stdlib.h>
#include <vector>
#include "orderbookmanager.h"

using namespace std;

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

