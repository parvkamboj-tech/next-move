/*
 * orders.h  —  Shivam (C++)
 * Order class + OrderManager (FIFO queue processing)
 * Integrates with Parv's inventory (stock check) and Siddhant's graph (pick route)
 */
#ifndef ORDERS_H
#define ORDERS_H

#include <queue>
#include <string>
#include <vector>

const int DISPATCH_DOCK = 1;   // location 1 in the graph

class Order
{
public:
    int orderId;
    int productId;
    std::string productName;
    int quantity;
    std::string status;   // Pending, Processed, Out of Stock, Cancelled
    std::string note;     // pick route or reason

    Order(int oid, int pid, const std::string &pname, int qty);
    void display() const;
};

class OrderManager
{
private:
    std::queue<Order> pending;     // FIFO queue
    std::vector<Order> history;    // processed / closed orders
    int nextOrderId;

public:
    explicit OrderManager(int startId = 1001);

    int  addOrder(int productId, int quantity);   // returns order ID, or -1
    void displayPending() const;
    void displayHistory() const;
    bool searchOrder(int orderId) const;
    bool cancelOrder(int orderId);
    bool processNextOrder();
    void checkAvailability(int productId, int quantity) const;

    int pendingCount() const;
    int processedCount() const;
};

#endif
