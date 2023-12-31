#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <algorithm>

using namespace std;

class Order_Book{
public :
    string client_id ;
    string order_id;
    string instru;
    int side ;
    string status ;
    int quantity;
    double price;
    string reason = "None";

public :
    Order_Book(const string& client_id, string instru, int side, int quantity, double price) :
        client_id(client_id),
        instru(instru),
        side(side),
        quantity(quantity),
        price(price)
        {}

};

string getTime()

{

    //This function output the current time and date

    auto currentTime = chrono::system_clock::now();

    time_t currentTimeT = chrono::system_clock::to_time_t(currentTime);

    // Format of the time "YYYY.MM.DD - HH.MM.SS.sss"
    tm *timeInfo = localtime(&currentTimeT);
    char formattedTime[24];
    strftime(formattedTime, sizeof(formattedTime), "%Y.%m.%d - %H.%M.%S", timeInfo);

    auto milliseconds = chrono::duration_cast<chrono::milliseconds>(currentTime.time_since_epoch()) % 1000;

    stringstream ss;
    ss << formattedTime << "." << setfill('0') << setw(3) << milliseconds.count();

    return ss.str();
}

bool buyAscending(const Order_Book &order1 , const Order_Book &order2) {

    /*This function compares two buy orders' prices
    and will be used in sort() function*/

    if (order1.price > order2.price) {return true ;}
    else {return false ;}   
}

bool sellDescending(const Order_Book &order1 , const Order_Book &order2) {

    /*This function compares two sell orders' prices
    and will be used in sort() function*/
    if (order1.price < order2.price) {return true ;}
    else {return false ;}
}


void processing(vector<Order_Book> &sell,
                vector<Order_Book> &buy, 
                const string &client_id,
                const string &instrument ,
                int side,
                int quantity,
                double price,
                Order_Book order,
                ofstream &output,
                int ID
                ) {
        
        //This function process an order an output it's process to the CSV file
 
    if (side == 1)   // Buy order
    {

        if ((sell.empty()) || sell[0].price > price) // Sell book empty or Price is not matching
        {
            order.status = "New";
            output << "Ord" << ID <<","<< client_id << "," << instrument << "," << side << "," << order.status << "," << quantity << "," << price << ", " << "      - ," << getTime() << endl;
            buy.push_back(order);
            sort(buy.begin(), buy.end(), buyAscending) ;
        }

        else {

            int isBreak = 0 ;

            while ((!sell.empty()) && (sell[0].price <= price)) {

                if (sell[0].quantity == quantity) {  // two orders' quantity equal

                    order.status = "Fill" ;
                    sell[0].status = "Fill" ;

                    output << "Ord" << ID <<","<< client_id << "," << instrument << "," << side << "," << order.status << "," << quantity << "," << sell[0].price << ", " << "      - ," << getTime() << endl;
                    output << "Ord" << sell[0].order_id <<","<< sell[0].client_id << "," << instrument << "," << sell[0].side << "," << sell[0].status << "," << quantity << "," << sell[0].price << ", " <<  "      - ," << getTime() << endl;

                    sell.erase(sell.begin()) ;
                    isBreak = 1 ;
                    break ;

                }

                else if (sell[0].quantity > quantity) {   // Lowest sell order quantity higher than buy order quantity

                    order.status = "Fill" ;
                    sell[0].status = "Pfill" ;
                    sell[0].quantity = sell[0].quantity - quantity ;

                    output << "Ord" << ID <<","<< client_id << "," << instrument << "," << side << "," << order.status << "," << quantity << "," << sell[0].price << ", " << "      - ," <<  getTime() << endl;
                    output << "Ord" << sell[0].order_id <<","<< sell[0].client_id << "," << instrument << "," << sell[0].side << "," << sell[0].status << "," << quantity << "," << sell[0].price << ", " <<  "      - ," << getTime() << endl;
                    isBreak = 1 ;
                    break ;
                }

                else {   // Lowest sell order quantity less than buy order quantity

                order.status = "Pfill" ;
                sell[0].status = "Fill" ;
                quantity = quantity - sell[0].quantity ;

                output << "Ord" << ID <<","<< client_id << "," << instrument << "," << side << "," << order.status << "," << sell[0].quantity << "," << sell[0].price << ", " <<  "      - ," << getTime() << endl;
                output << "Ord" << sell[0].order_id <<","<< sell[0].client_id << "," << instrument << "," << sell[0].side << "," << sell[0].status << "," << quantity << "," << sell[0].price << ", " <<  "      - ," << getTime() << endl;
                
                sell.erase(sell.begin()) ;
                }
            }

            if (!isBreak) {      
                
                 /* This block will run if the sell order book get empty 
                    or if find no matching order while processing the order*/

                order.status = "New";
                buy.push_back(order);
                sort(buy.begin(), buy.end(), buyAscending) ;
            }
        }

    }

    else {   // Sell order

        if ((buy.empty()) || buy[0].price < price) // Buy book empty or Price is not matching
        {
            order.status = "New";
            output << "Ord" << ID <<","<< client_id << "," << instrument << "," << side << "," << order.status << "," << quantity << "," << price << ", " <<  "      - ," << getTime() << endl;
            sell.push_back(order);
            sort(sell.begin(), sell.end(), sellDescending) ;
        }

        else {

            int isBreak = 0 ;

            while ((!buy.empty()) && (buy[0].price >= price)) {

                if (buy[0].quantity == order.quantity) {   // Two orders' quantity equal

                    order.status = "Fill" ;
                    buy[0].status = "Fill" ;

                    output << "Ord" << ID <<","<< client_id << "," << instrument << "," << side << "," << order.status << "," << quantity << "," << buy[0].price << ", " << "      - ," <<  getTime() << endl;
                    output << "Ord" << buy[0].order_id <<","<< buy[0].client_id << "," << instrument << "," << buy[0].side << "," << buy[0].status << "," << quantity << "," << buy[0].price << ", " <<  "      - ," << getTime() << endl;

                    buy.erase(buy.begin()) ;
                    isBreak = 1 ;
                    break ;

                }

                else if (buy[0].quantity > order.quantity) {  // Highest buy order quantity higher than sell order quantity

                    order.status = "Fill" ;
                    buy[0].status = "Pfill" ;
                    buy[0].quantity = buy[0].quantity - quantity ;

                    output << "Ord" << ID <<","<< client_id << "," << instrument << "," << side << "," << order.status << "," << order.quantity << "," << buy[0].price << ", " << "      - ," <<  getTime() << endl;
                    output << "Ord" << buy[0].order_id <<","<< buy[0].client_id << "," << instrument << "," << buy[0].side << "," << buy[0].status << "," << order.quantity << "," << buy[0].price << ", " <<  "      - ," << getTime() << endl;
                    isBreak = 1 ;
                    break ;
                }

                else {     // Highest buy order quantity less than sell order quantity

                order.status = "Pfill" ;
                buy[0].status = "Fill" ;
                order.quantity = order.quantity - buy[0].quantity ;

                output << "Ord" << ID <<","<< client_id << "," << instrument << "," << side << "," << order.status << "," << buy[0].quantity << "," << buy[0].price << ", " <<  "      - ," << getTime() << endl;
                output << "Ord" << buy[0].order_id <<","<< buy[0].client_id << "," << instrument << "," << buy[0].side << "," << buy[0].status << "," << buy[0].quantity << "," << buy[0].price << ", " <<  "      - ," << getTime() << endl;
                
                buy.erase(buy.begin()) ;
                }
            }

            if (!isBreak) {

                /* This block will run if the buy order book get empty 
                    or if find no matching order while processing*/

                order.status = "New";
                sell.push_back(order);
                sort(sell.begin(), sell.end(), sellDescending) ;
            }
        }

    }

}

int main() {

    vector<Order_Book> RoseSell ;
    vector<Order_Book> RoseBuy ;

    vector<Order_Book> LavenderSell ;
    vector<Order_Book> LavenderBuy ;

    vector<Order_Book> LotusSell ;
    vector<Order_Book> LotusBuy ;

    vector<Order_Book> TulipSell ;
    vector<Order_Book> TulipBuy ;

    vector<Order_Book> OrchidSell ;
    vector<Order_Book> OrchidBuy ;

    ifstream inputFile;      // Open the order.csv file as inputFile
    inputFile.open("order.csv"); 

    if (!inputFile.is_open())
    {
        cout << "Openning File Failed !!" << endl;
        return 1;
    }

    string header;
    getline(inputFile, header);

    ofstream output;      // Create the output csv file
    output.open("execution_rep.csv");

    if (!output.is_open())
    {
        cout << "Creating File Fsiled !!" << endl;
        return 1;
    }

    // Output file shape
    output << "Order ID, Cl. Ord. ID, Instrument, Side, Exec Status, Quantity, Price, Reason, Transaction Time" << endl;

    // Order number
    int ID = 1 ; 

    string line;
    while (getline(inputFile, line)) {
        vector<string> row;   // Adding values of a row into a vector
        istringstream ss(line);
        string cell;

        while (getline(ss, cell, ',')) {
            row.push_back(cell);
        }

        string client_id = row[0] ;
        string instrument = row[1] ;
        int side ;
        int quantity ;
        double price ;

        try {    // Side
            side = stoi(row[2]) ;
        }
        catch (const invalid_argument& e) {
            output << "Ord" << ID <<","<< client_id << "," << instrument << "," << row[2] << "," << "Reject" << "," << row[3] << "," << row[4] << ", " << "Invalid Side," <<getTime() << endl;
            ID++ ;
            continue ;
        }

        try {   
            if (!( side == 1 || side == 2)) {
                throw runtime_error("Invalid 'side' value");
            }
        }
        catch (const runtime_error& e) {
            output << "Ord" << ID <<","<< client_id << "," << instrument << "," << row[2] << "," << "Reject" << "," << row[3] << "," << row[4] << ", " << "Invalid Side," <<getTime() << endl;
            ID++ ;
            continue ;
        }

        try {     // Quantity
            quantity = stoi(row[3]);
        }
        catch (const invalid_argument& e) {
            output << "Ord" << ID <<","<< client_id << "," << instrument << "," << row[2] << "," << "Reject" << "," << row[3] << "," << row[4] << ", " << "Invalid Size," <<getTime() << endl;
            ID++ ;
            continue ;
        }

        try{
            if ((quantity % 10 != 0) || (quantity < 10) || (quantity > 1000)) {
                throw runtime_error("Invalid 'size' value") ;
            }
        }
        catch (const runtime_error& e) {
            output << "Ord" << ID <<","<< client_id << "," << instrument << "," << row[2] << "," << "Reject" << "," << row[3] << "," << row[4] << ", " << "Invalid Size," <<getTime() << endl;
            ID++ ;
            continue ;
        }

        try {    // Price
            price = stod(row[4]);
        }
        catch (const invalid_argument& e) {
            output << "Ord" << ID <<","<< client_id << "," << instrument << "," << row[2] << "," << "Reject" << "," << row[3] << "," << row[4] << ", " << "Invalid Price," <<getTime() << endl;
            ID++ ;
            continue ;
        }

        try{
            if (price <= 0) {
                throw runtime_error("Invalid 'size' valye") ;
            }
        }
        catch (const runtime_error& e) {
            output << "Ord" << ID <<","<< client_id << "," << instrument << "," << row[2] << "," << "Reject" << "," << row[3] << "," << row[4] << ", " << "Invalid Price," <<getTime() << endl;
            ID++ ;
            continue ;
        }

        Order_Book order(client_id, instrument, side, quantity, price) ;
        order.order_id = to_string(ID) ;

        if (order.client_id.empty()) {
            order.status = "Reject" ;
            output << "Ord" << ID <<","<< client_id << "," << instrument << "," << row[2] << "," << "Reject" << "," << row[3] << "," << row[4] << ", " << "Invalid Client ID," <<getTime() << endl;
            ID++ ;
            continue ;
        }

        else if (order.instru.empty() || (instrument != "Rose" && instrument != "Lavender" && instrument != "Lotus" && instrument != "Tulip" && instrument != "Orchid")){
            order.status = "Reject" ;
            output << "Ord" << ID <<","<< client_id << "," << instrument << "," << row[2] << "," << "Reject" << "," << row[3] << "," << row[4] << ", " << "Invalid Instrument," <<getTime() << endl;
            ID++ ;
            continue ;
        }

        if (instrument == "Rose")
            {processing(RoseSell, RoseBuy, client_id, instrument, side, quantity, price, order, output, ID);
            ID++;
            continue;}

        else if (instrument == "Lavender"){
            processing(LavenderSell, LavenderBuy, client_id, instrument, side, quantity, price, order, output, ID);
            ID++;
            continue;}

        else if (instrument == "Lotus"){
            processing(LotusSell, LotusBuy, client_id, instrument, side, quantity, price, order, output,ID);
            ID++;
            continue;}

        else if (instrument == "Tulip") {
            processing(TulipSell, TulipBuy, client_id, instrument, side, quantity, price, order, output, ID);
            ID++;
            continue;}

        else if (instrument == "Orchid"){
            processing(OrchidSell, OrchidBuy, client_id, instrument, side, quantity, price, order, output,ID);
            ID++;
            continue;}
      
    }
    
    inputFile.close();
    output.close();

    
    return 0; 
} 