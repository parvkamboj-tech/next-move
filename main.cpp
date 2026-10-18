/*
 * main.cpp  —  Smart Warehouse Management System (Integrated)
 *
 *   Parv     -> inventory.c / inventory.h   (C:   Hash Table, Linked List, BST)
 *   Siddhant -> graph.c     / graph.h       (C:   Graph, BFS, DFS, Dijkstra)
 *   Shivam   -> orders.cpp  / orders.h      (C++: Order class, Queue processing)
 *
 * Build (run inside the project folder):
 *   gcc -c inventory.c graph.c
 *   g++ main.cpp orders.cpp inventory.o graph.o -o warehouse
 *   ./warehouse          (Windows: warehouse.exe)
 */
#include <iostream>
#include <string>
#include <limits>
#include "inventory.h"
#include "graph.h"
#include "orders.h"

using namespace std;

const int LOW_STOCK_LIMIT = 10;

/* ===================== Input helpers ===================== */
int readInt(const string &prompt)
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

string readLine(const string &prompt)
{
    string s;
    cout << prompt;
    cin >> ws;              // skip leftover newline
    getline(cin, s);
    return s;
}

/* ===================== Sample data ===================== */
void loadSampleData(OrderManager &manager)
{
    graph_add_location("Dispatch dock");   // 1
    graph_add_location("Aisle A");         // 2
    graph_add_location("Aisle B");         // 3
    graph_add_location("Aisle C");         // 4
    graph_add_location("Cold store");      // 5
    graph_add_location("Returns bay");     // 6

    graph_add_path(1, 2, 4);
    graph_add_path(1, 3, 5);
    graph_add_path(2, 4, 6);
    graph_add_path(3, 4, 3);
    graph_add_path(2, 5, 9);
    graph_add_path(4, 5, 4);
    graph_add_path(4, 6, 5);
    graph_add_path(3, 6, 10);

    inv_add(150, "Cable ties", 85, 3);
    inv_add(101, "Copper wire roll", 40, 2);
    inv_add(205, "LED panel", 12, 4);
    inv_add(58, "Pallet wrap", 30, 3);
    inv_add(117, "Safety gloves", 120, 2);
    inv_add(342, "Vaccine cooler box", 6, 5);
    inv_add(263, "Returned drill", 2, 6);

    cout << "\nLoading sample orders...";
    manager.addOrder(205, 4);
    manager.addOrder(342, 10);   // will be Out of Stock
    manager.addOrder(117, 20);
}

/* ===================== Dashboard ===================== */
void showDashboard(const OrderManager &manager)
{
    cout << "\n=========== WAREHOUSE DASHBOARD ===========";
    cout << "\nProducts           : " << inv_count();
    cout << "\nUnits in stock     : " << inv_total_units();
    cout << "\nLow-stock products : " << inv_low_stock_count(LOW_STOCK_LIMIT);
    cout << "\nOrders waiting     : " << manager.pendingCount();
    cout << "\nOrders processed   : " << manager.processedCount();
    cout << "\nWarehouse locations: " << graph_location_count();
    cout << "\n===========================================\n";
    inv_display_low_stock(LOW_STOCK_LIMIT);
}

void listLocations()
{
    cout << "\nLocations: ";
    for (int i = 1; i <= graph_location_count(); i++)
        cout << i << "." << graph_location_name(i) << "  ";
    cout << "\n";
}

/* ===================== Inventory menu (Parv) ===================== */
void inventoryMenu()
{
    int choice;
    do
    {
        cout << "\n\n----- INVENTORY (Parv) -----";
        cout << "\n1. Add Product";
        cout << "\n2. Search Product";
        cout << "\n3. Update Quantity";
        cout << "\n4. Delete Product";
        cout << "\n5. Display Sorted (BST)";
        cout << "\n6. Display Hash Table";
        cout << "\n7. Low Stock Report";
        cout << "\n0. Back";
        choice = readInt("\nEnter Choice: ");

        switch (choice)
        {
        case 1:
        {
            int id = readInt("\nProduct ID: ");
            string name = readLine("Product Name: ");
            int qty = readInt("Quantity: ");
            listLocations();
            int loc = readInt("Store at Location: ");
            if (!graph_is_valid(loc))
            {
                cout << "\nInvalid Location!\n";
                break;
            }
            int r = inv_add(id, name.c_str(), qty, loc);
            if (r == 1) cout << "\nProduct Added! (hash bucket " << id % HASH_SIZE << ")\n";
            else if (r == 0) cout << "\nProduct ID already exists!\n";
            else cout << "\nInvalid input!\n";
            break;
        }
        case 2:
        {
            int bucket, steps;
            Product *p = inv_search(readInt("\nProduct ID: "), &bucket, &steps);
            if (p)
                cout << "\nFound: " << p->name << " | Qty: " << p->quantity
                     << " | At: " << graph_location_name(p->location)
                     << " | Bucket " << bucket << ", " << steps << " node(s) checked\n";
            else
                cout << "\nProduct Not Found!\n";
            break;
        }
        case 3:
        {
            int id = readInt("\nProduct ID: ");
            int change = readInt("Change (+ add / - remove): ");
            int r = inv_update_quantity(id, change);
            if (r == 1) cout << "\nQuantity Updated!\n";
            else if (r == 0) cout << "\nProduct Not Found!\n";
            else cout << "\nStock cannot go below zero!\n";
            break;
        }
        case 4:
            cout << (inv_delete(readInt("\nProduct ID: ")) ? "\nProduct Deleted!\n" : "\nProduct Not Found!\n");
            break;
        case 5: inv_display_sorted(); break;
        case 6: inv_display_hash_table(); break;
        case 7: inv_display_low_stock(LOW_STOCK_LIMIT); break;
        case 0: break;
        default: cout << "\nInvalid Choice!";
        }
    } while (choice != 0);
}

/* ===================== Orders menu (Shivam) ===================== */
void ordersMenu(OrderManager &manager)
{
    int choice;
    do
    {
        cout << "\n\n----- ORDERS (Shivam) -----";
        cout << "\n1. Add Order";
        cout << "\n2. Display Order Queue";
        cout << "\n3. Process Next Order";
        cout << "\n4. Search Order";
        cout << "\n5. Cancel Order";
        cout << "\n6. Check Product Availability";
        cout << "\n7. Order History";
        cout << "\n0. Back";
        choice = readInt("\nEnter Choice: ");

        switch (choice)
        {
        case 1:
        {
            inv_display_sorted();
            int pid = readInt("\nProduct ID: ");
            int qty = readInt("Quantity: ");
            manager.addOrder(pid, qty);
            break;
        }
        case 2: manager.displayPending(); break;
        case 3: manager.processNextOrder(); break;
        case 4: manager.searchOrder(readInt("\nOrder ID: ")); break;
        case 5: manager.cancelOrder(readInt("\nOrder ID: ")); break;
        case 6:
        {
            int pid = readInt("\nProduct ID: ");
            int qty = readInt("Required Quantity: ");
            manager.checkAvailability(pid, qty);
            break;
        }
        case 7: manager.displayHistory(); break;
        case 0: break;
        default: cout << "\nInvalid Choice!";
        }
    } while (choice != 0);
}

/* ===================== Map menu (Siddhant) ===================== */
void mapMenu()
{
    int choice;
    do
    {
        cout << "\n\n----- WAREHOUSE MAP (Siddhant) -----";
        cout << "\n1. Display Map";
        cout << "\n2. Add Location";
        cout << "\n3. Add / Update Path";
        cout << "\n4. Remove Path";
        cout << "\n5. BFS Traversal";
        cout << "\n6. DFS Traversal";
        cout << "\n7. Shortest Path (Dijkstra)";
        cout << "\n8. Block / Reopen Path (Rerouting)";
        cout << "\n0. Back";
        choice = readInt("\nEnter Choice: ");

        switch (choice)
        {
        case 1: graph_display(); break;
        case 2:
        {
            string name = readLine("\nLocation Name: ");
            int r = graph_add_location(name.c_str());
            if (r) cout << "\nLocation " << r << " Added!\n";
            else cout << "\nMaximum " << MAX_LOCATIONS << " locations reached!\n";
            break;
        }
        case 3:
        {
            listLocations();
            int a = readInt("From: ");
            int b = readInt("To: ");
            int d = readInt("Distance (m): ");
            cout << (graph_add_path(a, b, d) ? "\nPath Saved!\n" : "\nInvalid Path!\n");
            break;
        }
        case 4:
        {
            listLocations();
            int a = readInt("From: ");
            int b = readInt("To: ");
            cout << (graph_remove_path(a, b) ? "\nPath Removed!\n" : "\nNo Such Path!\n");
            break;
        }
        case 5: listLocations(); graph_bfs(readInt("Start: ")); break;
        case 6: listLocations(); graph_dfs(readInt("Start: ")); break;
        case 7:
        {
            listLocations();
            int a = readInt("From: ");
            int b = readInt("To: ");
            graph_print_shortest_path(a, b);
            break;
        }
        case 8:
        {
            listLocations();
            int a = readInt("From: ");
            int b = readInt("To: ");
            int r = graph_toggle_block(a, b);
            if (r == 1) cout << "\nPath BLOCKED. Shortest paths will reroute around it.\n";
            else if (r == 0) cout << "\nPath Reopened.\n";
            else cout << "\nNo Such Path!\n";
            break;
        }
        case 0: break;
        default: cout << "\nInvalid Choice!";
        }
    } while (choice != 0);
}

/* ===================== MAIN ===================== */
int main()
{
    inv_init();
    graph_init();

    OrderManager manager;
    loadSampleData(manager);

    int choice;
    do
    {
        cout << "\n\n======= SMART WAREHOUSE MANAGEMENT SYSTEM =======";
        cout << "\n1. Dashboard";
        cout << "\n2. Inventory      (Parv)";
        cout << "\n3. Orders         (Shivam)";
        cout << "\n4. Warehouse Map  (Siddhant)";
        cout << "\n0. Exit";
        choice = readInt("\n\nEnter Choice: ");

        switch (choice)
        {
        case 1: showDashboard(manager); break;
        case 2: inventoryMenu(); break;
        case 3: ordersMenu(manager); break;
        case 4: mapMenu(); break;
        case 0: cout << "\nExiting... Goodbye!\n"; break;
        default: cout << "\nInvalid Choice!";
        }
    } while (choice != 0);

    inv_free();
    return 0;
}
