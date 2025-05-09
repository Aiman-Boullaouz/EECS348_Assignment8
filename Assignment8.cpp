// EECS 348 Assignment 8
// CEO Email prioritization program
// Inputs: Test file names "Assignment8_Test_File.txt"
// Output: Email with highest priority
// Collaborators: DeepSeekAI, ChatGPT
// Other sources: Original code from canvas generated by ChatGPT
// Author: Aiman Boullaouz
// Creation date: 5/8/2025

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <map>
#include <ctime>
#include <fstream> //A Needed to read from file

using namespace std;  // Allow use of standard library names without 'std::'

int arrivalCounter = 0;  // Tracks the order emails arrive for tie-breaking

// Convert MM-DD-YYYY to time_t for easy comparison
time_t parseDate(const string& dateStr) {
    struct tm tm{};  // Create temporary time structure
    sscanf(dateStr.c_str(), "%d-%d-%d", &tm.tm_mon, &tm.tm_mday, &tm.tm_year);  // Split date string into parts
    tm.tm_mon -= 1;           // struct tm months start from 0 (January = 0)
    tm.tm_year -= 1900;       // struct tm years are counted since 1900
    //D return mktime(tm);
    return mktime(&tm); //A Fix: &tm instead of tm for pointer (convert to timestamp)
}

// Map sender category to priority (higher number = more important)
map<string, int> senderPriority = {  // Priority lookup table
    {"Boss", 5},         // Highest priority
    {"Subordinate", 4},  // Second highest
    {"Peer", 3},         // Medium priority
    {"ImportantPerson", 2},  // Low priority
    {"OtherPerson", 1}   // Lowest priority
};

class Email {
public:
    string senderCategory;  // Who sent the email (from priority map keys)
    string subject;        // Email subject line
    string dateStr;        // Original date string
    time_t date;           // Converted timestamp for comparison
    int arrivalOrder;      // Order email was received (lower = earlier)

    Email(string sender, string subj, string date_string) {  // Create new email
        senderCategory = sender;     // Set sender category
        subject = subj;              // Set email subject
        dateStr = date_string;       // Store original date string
        date = parseDate(date_string);  // Convert to timestamp
        arrivalOrder = arrivalCounter++;  // Set order and increment counter
    }

    Email() : senderCategory(""), subject(""), dateStr(""), date(0), arrivalOrder(0) {} //A Default constructor (empty email)

    // Compare by priority rules
    bool operator>(const Email& other) const {
        // First compare sender priorities
        if (senderPriority[senderCategory] != senderPriority[other.senderCategory])
            return senderPriority[senderCategory] > senderPriority[other.senderCategory];
        // If equal, compare dates (newer first)
        if (date != other.date)
            return date > other.date;
        // If dates equal, earlier arrivals come first
        return arrivalOrder < other.arrivalOrder;
    }

    void display() const {  // Show email details
        cout << "Sender: " << senderCategory << endl;  // Print sender
        cout << "Subject: " << subject << endl;        // Print subject
        cout << "Date: " << dateStr << endl;          // Print original date
    }
};

class MaxHeap {
private:
    vector<Email> heap;  // Store emails in heap structure

    // Move new emails up to correct position
    void heapifyUp(int idx) {
        // Compare with parent until root or correct position
        while (idx > 0 && heap[idx] > heap[(idx - 1) / 2]) {
            swap(heap[idx], heap[(idx - 1) / 2]);  // Swap with parent
            idx = (idx - 1) / 2;                   // Move to parent position
        }
    }

    // Move root down to correct position after removal
    void heapifyDown(int idx) {
        int size = heap.size();  // Get current heap size
        while (true) {
            int largest = idx;   // Assume current is largest
            int left = 2 * idx + 1;  // Calculate left child index
            int right = 2 * idx + 2; // Calculate right child index

            // Find which child is larger
            if (left < size && heap[left] > heap[largest]) largest = left;
            if (right < size && heap[right] > heap[largest]) largest = right;

            // Swap if needed, otherwise done
            if (largest != idx) {
                swap(heap[idx], heap[largest]);
                idx = largest;
            } else {
                break;
            }
        }
    }

public:
    void push(const Email& email) {  // Add new email to heap
        heap.push_back(email);       // Add to end of vector
        heapifyUp(heap.size() - 1);  // Adjust position upward
    }

    Email peek() {  // Look at top email without removing
        if (!heap.empty()) return heap[0];  // Return first element
        throw runtime_error("No emails to read.");  // Error if empty
    }

    void pop() {  // Remove top email
        if (heap.empty()) return;        // Do nothing if empty
        heap[0] = heap.back();         // Move last element to root
        heap.pop_back();               // Remove last element
        if (!heap.empty()) heapifyDown(0);  // Adjust root downward if needed
    }

    int size() const {  // Get number of emails
        return heap.size();
    }

    bool empty() const {  // Check if no emails left
        return heap.empty();
    }
};

class EmailManager {
private:
    MaxHeap heap;            // Main email storage
    bool hasCurrent = false; // Track previewed email
    Email currentEmail; //A Initialized with default constructor (empty email)

public:
    void processLine(const string& line) {  // Handle different commands
        if (line.rfind("EMAIL ", 0) == 0) {  // If line starts with "EMAIL"
            string rest = line.substr(6);    // Remove "EMAIL " prefix
            stringstream ss(rest);          // Split remaining text
            string sender, subject, date;

            // Extract comma-separated values
            getline(ss, sender, ',');
            getline(ss, subject, ',');
            getline(ss, date, ',');

            Email email(sender, subject, date);  // Create email object
            heap.push(email);                    // Add to priority heap
        } else if (line == "COUNT") {           // Count command
            cout << "There are " << heap.size() << " emails to read.\n" << endl;
        } else if (line == "NEXT") {           // Preview next email
            if (!hasCurrent) {                // If not already previewing
                if (!heap.empty()) {
                    currentEmail = heap.peek();  // Get top email
                    hasCurrent = true;         // Mark as previewed
                }
            }
            if (hasCurrent) {                 // Show previewed email
                cout << "Next email:" << endl;
                currentEmail.display();
                cout << endl;
            } else {
                cout << "No emails to read.\n" << endl;
            }
        } else if (line == "READ") {         // Mark email as read
            if (hasCurrent) {               // If previewing, clear preview
                heap.pop();
                hasCurrent = false;
            } else if (!heap.empty()) {     // Otherwise remove top email
                heap.pop();
            }
        }
    }
};

// Read from a file and process commands
void runFromFile(const string& filename) {
    ifstream infile(filename); //A Ensure file is opened correctly
    string line;
    EmailManager manager;      // Create email management system

    while (getline(infile, line)) {  // Read file line by line
        if (!line.empty()) {        // Skip empty lines
            manager.processLine(line);  // Process each command
        }
    }
    infile.close(); //A Close the file after use (good practice)
}

int main() {
    //D runFromFile("test.txt");
    runFromFile("Assignment8_Test_File.txt"); //A Required test file name
    return 0;  // End program
}