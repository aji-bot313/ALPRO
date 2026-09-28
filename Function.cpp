#include <iostream>

// Required Macros
#define cin std::cin
#define cout std::cout
#define endl std::endl
// --
// Function Prototypes
void showMainMenu(char names[][50], int stocks[], int prices[], int &total);
void storeMenu(char names[][50], int stocks[], int prices[], int &total);
void calculatorMenu();

// Store Functions
void addItem(char names[][50], int stocks[], int prices[], int &total);
void removeItem(char names[][50], int stocks[], int prices[], int &total);
void editItemMenu(char names[][50], int stocks[], int prices[], int total);
void viewItems(char names[][50], int stocks[], int prices[], int total);

// Edit Sub-functions - Using Pass by Reference
void editName(char nameRef[]);
void editPrice(int &priceRef);
void addStock(int &stockRef);
void reduceStock(int &stockRef);

// Calculator Functions
void basicOperation();
int factorial(int n);

int main() {
    // All item data declared in main() - No global variables
    const int MAX = 100;
    char names[MAX][50];
    int stocks[MAX];
    int prices[MAX];
    int total = 0;

    showMainMenu(names, stocks, prices, total);
    
    return 0;
}

// MAIN MENU ALGORITHM
void showMainMenu(char names[][50], int stocks[], int prices[], int &total) {
    int choice;
    while (true) {
        cout << "\n===============================" << endl;
        cout << "      MODULE 4 - FUNCTION      " << endl;
        cout << "           MAIN MENU           " << endl;
        cout << "===============================" << endl;
        cout << "1. TIVAIZ Store" << endl;
        cout << "2. TIVAIZ Calculator" << endl;
        cout << "0. Exit" << endl;
        cout << "Choose menu: ";

        if (!(cin >> choice)) {
            cout << "[Failed] Input must be a number!" << endl;
            cin.clear(); 
            cin.ignore(1000, '\n');
            continue;
        }

        if (choice == 1) storeMenu(names, stocks, prices, total);
        else if (choice == 2) calculatorMenu();
        else if (choice == 0) break;
        else cout << "[Failed] Invalid menu! Choose between 0 and 2." << endl;
    }
}

// STORE MENU ALGORITHM
void storeMenu(char names[][50], int stocks[], int prices[], int &total) {
    int choice;
    while (true) {
        cout << "\nTIVAIZ STORE" << endl;
        cout << "1. Add Item" << endl;
        cout << "2. Remove Item" << endl;
        cout << "3. Edit Item" << endl;
        cout << "4. View Items" << endl;
        cout << "0. Back" << endl;
        cout << "Choose menu: ";

        if (!(cin >> choice)) {
            cout << "[Failed] Input must be a number!" << endl;
            cin.clear(); 
            cin.ignore(1000, '\n'); 
            continue;
        }

        if (choice == 1) addItem(names, stocks, prices, total);
        else if (choice == 2) removeItem(names, stocks, prices, total);
        else if (choice == 3) editItemMenu(names, stocks, prices, total);
        else if (choice == 4) viewItems(names, stocks, prices, total);
        else if (choice == 0) break;
        else cout << "[Failed] Invalid menu! Choose between 0 and 4." << endl;
    }
}

void addItem(char names[][50], int stocks[], int prices[], int &total) {
    cout << "\nItem Name: "; 
    cin.ignore(); 
    cin.getline(names[total], 50);
    cout << "Stock (unit): "; cin >> stocks[total];
    cout << "Price: "; cin >> prices[total];
    total++;
    cout << "[Success] Item added!" << endl;
}

void removeItem(char names[][50], int stocks[], int prices[], int &total) {
    if (total == 0) { cout << "Store is empty!" << endl; return; }
    viewItems(names, stocks, prices, total);
    int idx; 
    cout << "Choose item number to remove (1-" << total << "): "; 
    cin >> idx;

    if (idx < 1 || idx > total) {
        cout << "[Failed] Invalid item number!" << endl;
        return;
    }

    for (int i = idx - 1; i < total - 1; i++) {
        for (int j = 0; j < 50; j++) names[i][j] = names[i+1][j];
        stocks[i] = stocks[i+1];
        prices[i] = prices[i+1];
    }
    total--;
    cout << "[Success] Item removed!" << endl;
}

// EDIT MENU
void editItemMenu(char names[][50], int stocks[], int prices[], int total) {
    if (total == 0) { cout << "Nothing to edit!" << endl; return; }
    viewItems(names, stocks, prices, total);
    
    int idx, opt;
    cout << "Choose item number to edit: "; 
    if (!(cin >> idx) || idx < 1 || idx > total) {
        cout << "[Failed] Invalid item number!" << endl;
        cin.clear(); 
        cin.ignore(1000, '\n');
        return;
    }
    
    int i = idx - 1;
    bool editing = true;
    while (editing) {
        cout << "\nEDIT ITEM" << endl;
        cout << "Item: " << names[i] << endl;
        cout << "1. Edit name\n2. Edit Price\n3. Add Stock\n4. Reduce Stock\n0. Back" << endl;
        cout << "Choice: ";
        
        if (!(cin >> opt)) {
            cout << "[Failed] Input must be a number!" << endl;
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }

        if (opt == 1) editName(names[i]);
        else if (opt == 2) editPrice(prices[i]);
        else if (opt == 3) addStock(stocks[i]);
        else if (opt == 4) reduceStock(stocks[i]);
        else if (opt == 0) editing = false;
        else cout << "[Failed] Invalid option! Choose between 0 and 4." << endl;
    }
}

void editName(char nameRef[]) {
    cout << "New Name: ";
    cin.ignore();
    cin.getline(nameRef, 50);
    cout << "[Success] Name updated." << endl;
}

void editPrice(int &priceRef) {
    cout << "New Price: "; cin >> priceRef;
    cout << "[Success] Price updated." << endl;
}

void addStock(int &stockRef) {
    int amt; cout << "Amount to add: "; cin >> amt;
    if (amt <= 0) {
        cout << "[Failed] Amount must be positive!" << endl;
    } else {
        stockRef += amt;
        cout << "[Success] Stock added." << endl;
    }
}
void reduceStock(int &stockRef) {
    int amt; cout << "Amount to reduce: "; cin >> amt;
    if (amt <= 0) {
        cout << "[Failed] Amount must be positive!" << endl;
    } else if (stockRef - amt < 1) {
        cout << "[Failed] Minimum stock is 1. Current stock: " << stockRef << endl;
    } else { 
        stockRef -= amt; 
        cout << "[Success] Stock reduced." << endl; 
    }
}

void viewItems(char names[][50], int stocks[], int prices[], int total) {
    if (total == 0) {
        cout << "\nNo items in store!" << endl;
        return;
    }
    cout << "\nTIVAIZ STORE - VIEW ITEMS" << endl;
    cout << "--------------------------------------------------------" << endl;
    for (int i = 0; i < total; i++) {
        cout << "No         : " << i + 1 << endl;
        cout << "Item Name  : " << names[i] << endl;
        cout << "Stock      : " << stocks[i] << endl;
        cout << "Price/Unit : " << prices[i] << endl;
        cout << "Total Price: " << stocks[i] * prices[i] << endl; 
        cout << "--------------------------------------------------------" << endl;
    }
    cout << "Total items: " << total << " types" << endl;
}

void calculatorMenu() {
    int choice;
    while (true) {
        cout << "\nCalculator Menu" << endl;
        cout << "1. Basic Operation (+, -, *, /)\n2. Factorial\n0. Back" << endl;
        cout << "Choose menu: ";
        if (!(cin >> choice)) {
            cout << "[Failed] Input must be a number!" << endl;
            cin.clear(); 
            cin.ignore(1000, '\n'); 
            continue;
        }
        if (choice == 1) basicOperation();
        else if (choice == 2) {
            int n; 
            cout << "Enter number: "; 
            cin >> n;
            if (n < 0) {
                cout << "Factorial is not defined for negative numbers!" << endl;
            } else {
                cout << "Result: " << factorial(n) << endl;
            }
        }
        else if (choice == 0) break;
        else cout << "[Failed] Invalid menu! Choose between 0 and 2." << endl;
    }
}

void basicOperation() {
    double firstnumber, secondnumber; 
    char op;
    cout << "Enter operation (e.g., 5 + 3): "; 
    cin >> firstnumber >> op >> secondnumber;
    cout << "Result: ";
    if (op == '+') cout << firstnumber + secondnumber << endl;
    else if (op == '-') cout << firstnumber - secondnumber << endl;
    else if (op == '*') cout << firstnumber * secondnumber << endl;
    else if (op == '/') {
        if (secondnumber != 0) 
            cout << firstnumber / secondnumber << endl;
        else 
            cout << "Error: Division by zero!" << endl;
    }
    else cout << "Invalid operator!" << endl;
}

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}
