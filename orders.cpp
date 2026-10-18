/*
 * orders.cpp  —  Shivam (C++)
 * Order Management: class, FIFO queue, processing, out-of-stock handling
 *
 * Run alone:
 *   gcc -c inventory.c graph.c
 *   g++ -DSTANDALONE orders.cpp inventory.o graph.o -o shivam_orders
 */
#include <iostream>
#include <iomanip>
#include <limits>
#include "orders.h"
#include "inventory.h"
#include "graph.h"

using namespace std;

/* ===================== Order class ===================== */
Order::Order(int oid, int pid, const string &pname, int qty)
    : orderId(oid), productId(pid), productName(pname), quantity(qty), status("Pending"), note("")
{
}

void Order::display() const
{
    cout << "\nOrder ID     : " << orderId;
    cout << "\nProduct      : " << productName << " (ID " << productId << ")";
    cout << "\nQuantity     : " << quantity;
    cout << "\nStatus       : " << status;
    if (!note.empty())
        cout << "\nDetails      : " << note;
    cout << "\n--------------------------------------";
}

/* ===================== OrderManager ===================== */
OrderManager::OrderManager(int startId) : nextOrderId(startId) {}

int OrderManager::addOrder(int productId, int quantity)
{
    if (quantity <= 0)
    {
        cout << "\nQuantity must be at least 1!\n";
        return -1;
    }

    Product *p = inv_search(productId, NULL, NULL);   // Parv's hash table lookup
    if (p == NULL)
    {
        cout << "\nProduct ID " << productId << " not found in inventory!\n";
        return -1;
    }

    Order newOrder(nextOrderId++, productId, p->name, quantity);
    pending.push(newOrder);

    cout << "\nOrder #" << newOrder.orderId << " added to the queue.";
    if (p->quantity < quantity)
        cout << "\nWarning: only " << p->quantity << " in stock right now.";
    cout << "\n";
    return newOrder.orderId;
}

void OrderManager::displayPending() const
{
    if (pending.empty())
    {
        cout << "\nNo Orders Waiting!\n";
        return;
    }

    queue<Order> temp = pending;
    int position = 1;

    cout << "\n===== ORDER QUEUE (front first) =====";
    while (!temp.empty())
    {
        cout << "\nPosition " << position;
        if (position == 1)
            cout << "  <- FRONT";
        temp.front().display();
        temp.pop();
        position++;
    }
    cout << endl;
}

void OrderManager::displayHistory() const
{
    if (history.empty())
    {
        cout << "\nNo Processed Orders Yet!\n";
        return;
    }

    cout << "\n===== ORDER HISTORY =====";
    for (size_t i = 0; i < history.size(); i++)
        history[i].display();
    cout << endl;
}

bool OrderManager::searchOrder(int orderId) const
{
    queue<Order> temp = pending;
    int position = 1;

    while (!temp.empty())
    {
        if (temp.front().orderId == orderId)
        {
            temp.front().display();
            cout << "\nQueue Position: " << position << " of " << pending.size() << endl;
            return true;
        }
        temp.pop();
        position++;
    }

    for (size_t i = 0; i < history.size(); i++)
    {
        if (history[i].orderId == orderId)
        {
            history[i].display();
            cout << endl;
            return true;
        }
    }

    cout << "\nOrder Not Found!\n";
    return false;
}

bool OrderManager::cancelOrder(int orderId)
{
    queue<Order> temp;
    bool found = false;

    while (!pending.empty())
    {
        Order current = pending.front();
        pending.pop();

        if (current.orderId == orderId && !found)
        {
            current.status = "Cancelled";
            current.note = "Cancelled by user";
            history.push_back(current);
            found = true;
        }
        else
        {
            temp.push(current);   // keeps original FIFO order
        }
    }
    pending = temp;

    cout << (found ? "\nOrder Cancelled!\n" : "\nOrder not waiting in queue!\n");
    return found;
}

/* ---- INTEGRATION: inventory check + stock deduction + shortest route ---- */
bool OrderManager::processNextOrder()
{
    if (pending.empty())
    {
        cout << "\nNo Order to Process!\n";
        return false;
    }

    Order current = pending.front();
    pending.pop();

    Product *p = inv_search(current.productId, NULL, NULL);

    if (p == NULL)
    {
        current.status = "Cancelled";
        current.note = "Product no longer in inventory";
    }
    else if (p->quantity < current.quantity)
    {
        current.status = "Out of Stock";
        current.note = "Needed " + to_string(current.quantity) +
                       ", only " + to_string(p->quantity) + " available";
    }
    else
    {
        inv_update_quantity(current.productId, -current.quantity);
        current.status = "Processed";

        int path[MAX_LOCATIONS], length = 0;
        int distance = graph_shortest_path(DISPATCH_DOCK, p->location, path, &length);

        if (distance < 0)
        {
            current.note = "Stock deducted, but no open route from dock to location " +
                           to_string(p->location);
        }
        else
        {
            string route = "Pick route: ";
            for (int i = 0; i < length; i++)
            {
                if (i) route += " -> ";
                route += graph_location_name(path[i]);
            }
            route += " (" + to_string(distance) + " m)";
            current.note = route;
        }
    }

    history.push_back(current);

    cout << "\n===== ORDER PROCESSED =====";
    current.display();
    cout << endl;
    return true;
}

void OrderManager::checkAvailability(int productId, int quantity) const
{
    Product *p = inv_search(productId, NULL, NULL);

    if (p == NULL)
    {
        cout << "\nProduct Not Found!\n";
        return;
    }

    cout << "\n" << p->name << ": " << p->quantity << " in stock at "
         << graph_location_name(p->location) << ".";

    if (p->quantity >= quantity)
        cout << "\nAvailable! Order of " << quantity << " can be processed.\n";
    else
        cout << "\nInsufficient Stock! Short by " << (quantity - p->quantity) << ".\n";
}

int OrderManager::pendingCount() const { return (int)pending.size(); }

int OrderManager::processedCount() const
{
    int count = 0;
    for (size_t i = 0; i < history.size(); i++)
        if (history[i].status == "Processed")
            count++;
    return count;
}

/* ===================== STANDALONE MENU (Shivam's demo) ===================== */
#ifdef STANDALONE
static int readInt(const string &prompt)
{
    int x;
    cout << prompt;
    while (!(cin >> x))
    {
        if (cin.eof()) exit(0);
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Please enter a number: ";
    }
    return x;
}

int main()
{
    // small sample so orders can be tested on their own
    inv_init();
    graph_init();
    graph_add_location("Dispatch dock");
    graph_add_location("Aisle A");
    graph_add_location("Aisle B");
    graph_add_path(1, 2, 4);
    graph_add_path(1, 3, 5);
    graph_add_path(2, 3, 3);
    inv_add(101, "Copper wire roll", 40, 2);
    inv_add(205, "LED panel", 12, 3);
    inv_add(342, "Cooler box", 6, 3);

    OrderManager manager;
    int choice;

    do
    {
        cout << "\n\n===== ORDER MANAGEMENT SYSTEM (Shivam) =====";
        cout << "\nProducts for testing: 101, 205, 342";
        cout << "\n1. Add Order";
        cout << "\n2. Display Order Queue";
        cout << "\n3. Search Order";
        cout << "\n4. Cancel Order";
        cout << "\n5. Process Next Order";
        cout << "\n6. Check Product Availability";
        cout << "\n7. Order History";
        cout << "\n8. Exit";
        choice = readInt("\n\nEnter Choice: ");

        switch (choice)
        {
        case 1: { int pid = readInt("\nEnter Product ID: "); int q = readInt("Enter Quantity: "); manager.addOrder(pid, q); break; }
        case 2: manager.displayPending(); break;
        case 3: manager.searchOrder(readInt("\nEnter Order ID: ")); break;
        case 4: manager.cancelOrder(readInt("\nEnter Order ID: ")); break;
        case 5: manager.processNextOrder(); break;
        case 6: { int pid = readInt("\nEnter Product ID: "); int q = readInt("Enter Required Quantity: "); manager.checkAvailability(pid, q); break; }
        case 7: manager.displayHistory(); break;
        case 8: cout << "\nExiting...\n"; break;
        default: cout << "\nInvalid Choice!";
        }
    } while (choice != 8);

    inv_free();
    return 0;
}
#endif
