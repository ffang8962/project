#include <iostream>
#include <fstream>
#include <exception>
#include <sstream> 
#include <string>
#include <map>
#include <iomanip>
#include <stdlib.h>
#include <vector>
#include "orderbook.h"
#include "orderbookmanager.h"

using namespace std;

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

// g++ -Wall -Werror -o orderprocessor orderprocessor.cpp
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
