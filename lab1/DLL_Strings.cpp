#include <iostream>
#include <cstring>
using namespace std;


void pushString(char stringIn[1000]);
void printList();
void deleteElement(int entryToDelete);

class DLElement {
    public: 
    char stringData[1000]; 
    DLElement *prev;
    DLElement *next;

    DLElement() { //constructor
        next = NULL;
        prev = NULL;
        for (int i = 0; i < 1000; i++) {
            stringData[i] = 0;
        }
    }
};


DLElement *head = NULL;

int main() {

    int userSelect = 0;
    char newString[1000];
    int deletedSelection = 0;
    while (userSelect != 4) {
        cout << "Select an execution from the following list:\n\n"
             << "1. Push string\n" << "2. Print list\n" << "3. Delete item\n" << "4. End program\n\n";
        cin >> userSelect;

        switch (userSelect) {
            case 1: // Push string
                cout << "Enter a string:\n";
                cin >> newString;
                pushString(newString); //default passed by reference
                break;

            case 2: // Print list
                cout << "The list in order:\n"; 
                printList();
                break;
                
            case 3: //Delete Item
                cout << "Select an item to be deleted (1 corresponds to the first item):\n"; 
                cin >> deletedSelection;
                deleteElement(deletedSelection);
                break;

            case 4: // end program:
                cout << "User terminated the program...\n"; break;
            default: // not a valid selection
                cout << "Not a valid selection\n";
        }
    }
    return 0;
}

void deleteElement(int entryToDelete) {

    DLElement *deleteElement = head; // create pointer to point to the element
    for (int i = 1; i < entryToDelete; i++) {
        deleteElement = deleteElement-> next;
    }

    if ((deleteElement->prev)&&(deleteElement->next)) { // if between two elements
        deleteElement->prev->next = deleteElement->next; //set next of prev element to next element
        deleteElement->next->prev = deleteElement->prev;
    }
    else if ((entryToDelete == 1)&&(deleteElement->next)) { // if deleting first element (more than one)
        head-> next -> prev = NULL; // set next in chain prev pointer to NULL
        head = head-> next; // set head to next list in chain (could be NULL)
    }
    else if ((deleteElement->prev)&&(deleteElement->next == NULL)) { // last element
        deleteElement->prev->next = NULL; // Set the previous element's next ptr to NULL
    }
    else { // only one entry
        head = NULL;
    }
    delete[] deleteElement;

}

void printList() {

    for (DLElement *itemPtr = head; itemPtr != NULL; itemPtr = itemPtr->next) {
        cout << itemPtr-> stringData << endl;
    }
}


void pushString(char stringIn[1000]) {

    DLElement *itemPtr = new DLElement; // creating dynamic memory element on heap
    //itemPtr->stringData = stringIn; // Assign new data to user string input
    strcpy(itemPtr->stringData, stringIn);

    if (!head) { // if adding the first element, i.e. head = NULL
        head = itemPtr; // set head to first element
    }
    else {
        DLElement *lastElement = NULL;
        for (lastElement = head; lastElement->next != NULL; lastElement = lastElement-> next) {};   
        lastElement->next = itemPtr; // point to new last element
        itemPtr->prev = lastElement; // point new item prev pointer to previous last element
        // itemPtr->next = NULL; UNNECESSARY

        lastElement = itemPtr;
    }
}
