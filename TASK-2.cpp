#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
using namespace std;

struct Student {
    int studentID;
    string name;
    float GPA;
    Student* left;
    Student* right;
    Student(int id, const string& n, float g) : studentID(id), name(n), GPA(g), left(nullptr), right(nullptr) {}
};

typedef vector<const Student*> List;

class StudentBST {
    Student* root;

    // inserted tells caller if new node was really added
    Student* insertRec(Student* node, int id, const string& name, float gpa, bool& inserted) {
        if (!node) { inserted = true; return new Student(id, name, gpa); }
        if (id < node->studentID) node->left = insertRec(node->left, id, name, gpa, inserted);
        else if (id > node->studentID) node->right = insertRec(node->right, id, name, gpa, inserted);
        return node;                           // same ID: inserted stays false
    }

    Student* searchRec(Student* node, int id) const {
        if (!node || node->studentID == id) return node;
        return searchRec(id < node->studentID ? node->left : node->right, id);
    }

    Student* findMin(Student* node) const {
        while (node->left) node = node->left;
        return node;
    }

    Student* deleteRec(Student* node, int id, bool& deleted) {
        if (!node) return nullptr;             // ID not found
        if (id < node->studentID) node->left = deleteRec(node->left, id, deleted);
        else if (id > node->studentID) node->right = deleteRec(node->right, id, deleted);
        else {
            deleted = true;
            if (!node->left || !node->right) { // zero or one child: replace node by its child
                Student* child = node->left ? node->left : node->right;
                delete node;
                return child;
            }
            // two children: copy inorder successor here, then delete it from right subtree
            Student* succ = findMin(node->right);
            node->studentID = succ->studentID;
            node->name = succ->name;
            node->GPA = succ->GPA;
            bool dummy = false;
            node->right = deleteRec(node->right, succ->studentID, dummy);
        }
        return node;
    }

    void inorderRec(Student* node, List& out) const {
        if (!node) return;
        inorderRec(node->left, out);
        out.push_back(node);
        inorderRec(node->right, out);
    }

    int countRec(Student* node) const { return node ? 1 + countRec(node->left) + countRec(node->right) : 0; }

    void freeRec(Student* node) {
        if (!node) return;
        freeRec(node->left);
        freeRec(node->right);
        delete node;
    }

    // all students having extreme GPA (ties are possible)
    List extremeGPA(bool highest) const {
        List all = inorder(), res;
        if (all.empty()) return res;
        float m = all[0]->GPA;
        for (auto s : all) m = highest ? max(m, s->GPA) : min(m, s->GPA);
        for (auto s : all) if (s->GPA == m) res.push_back(s);
        return res;
    }

public:
    StudentBST() : root(nullptr) {}
    ~StudentBST() { freeRec(root); }
    // copy blocked, otherwise double free
    StudentBST(const StudentBST&) = delete;
    StudentBST& operator=(const StudentBST&) = delete;

    bool insert(int id, const string& name, float gpa) {
        bool inserted = false;
        root = insertRec(root, id, name, gpa, inserted);
        return inserted;
    }
    const Student* search(int id) const { return searchRec(root, id); }
    bool remove(int id) {
        bool deleted = false;
        root = deleteRec(root, id, deleted);
        return deleted;
    }
    List inorder() const { List out; inorderRec(root, out); return out; }
    int count() const { return countRec(root); }
    List highestGPA() const { return extremeGPA(true); }
    List lowestGPA() const { return extremeGPA(false); }
    // strictly greater, so GPA equal to threshold is not included
    List aboveGPA(float th) const {
        List res;
        for (auto s : inorder()) if (s->GPA > th) res.push_back(s);
        return res;
    }
    // only for testing: ID of root (0 if empty)
    int rootID() const { return root ? root->studentID : 0; }
};

void printStudent(const Student* s) {
    cout << "   ID : " << s->studentID << " | Name : " << s->name
         << " | GPA : " << fixed << setprecision(2) << s->GPA << "\n";
}

void printList(const List& v, const string& emptyMsg) {
    if (v.empty()) { cout << "   " << emptyMsg << "\n"; return; }
    cout << "   " << left << setw(10) << "ID" << setw(25) << "Name" << "GPA\n   " << string(45, '-') << "\n";
    for (auto s : v)
        cout << "   " << left << setw(10) << s->studentID << setw(25) << s->name << fixed << setprecision(2) << s->GPA << "\n";
}

bool readLine(const string& prompt, string& line) {
    cout << prompt;
    return static_cast<bool>(getline(cin, line));
}

// works for int and float, asks again till full line is a valid number, false only on EOF
template <class T>
bool readNum(const string& prompt, T& value) {
    string line;
    while (readLine(prompt, line)) {
        istringstream in(line);
        char extra;
        if ((in >> value) && !(in >> extra)) return true;
        cout << "   Invalid number, please try again.\n";
    }
    return false;
}

bool readGPA(const string& prompt, float& gpa) {
    while (readNum(prompt, gpa)) {
        if (gpa >= 0.0f && gpa <= 4.0f) return true;
        cout << "   GPA should be between 0.00 and 4.00 only.\n";
    }
    return false;
}

void loadSampleData(StudentBST& bst) {
    bst.insert(50, "ABDUL MOIZ", 3.45f);
    bst.insert(30, "NAWAZ SHARIF", 2.80f);
    bst.insert(70, "SAMANDAR KHAN", 3.90f);
    bst.insert(20, "DARYA KHAN", 2.10f);
    bst.insert(40, "SETHI", 3.00f);
    bst.insert(60, "FAEZA", 3.67f);
    bst.insert(80, "ASIM MUNIR", 3.95f);
}

// ---------------- Built-in test cases ----------------
static int passed = 0, failed = 0;

void check(const string& name, bool ok) {
    (ok ? passed : failed)++;
    cout << (ok ? "  [PASS] " : "  [FAIL] ") << name << "\n";
}

// list of students to string of IDs, e.g. "20 30 40"
string idsOf(const List& v) {
    string out;
    for (size_t i = 0; i < v.size(); i++) out += (i ? " " : "") + to_string(v[i]->studentID);
    return out;
}

bool sameFloat(float a, float b) { return fabs(a - b) < 0.001f; }

void runTests() {
    cout << "===== TEST GROUP A : Empty tree =====\n";
    {
        StudentBST t;
        check("count on empty tree = 0", t.count() == 0);
        check("inorder on empty tree is empty", t.inorder().empty());
        check("search on empty tree returns null", t.search(10) == nullptr);
        check("delete on empty tree returns false", t.remove(10) == false);
        check("highest GPA on empty tree is empty", t.highestGPA().empty());
        check("lowest GPA on empty tree is empty", t.lowestGPA().empty());
        check("above-threshold on empty tree is empty", t.aboveGPA(3.0f).empty());
    }

    cout << "\n===== TEST GROUP B : Insert + inorder + count =====\n";
    {
        StudentBST t;
        loadSampleData(t);
        check("7 students inserted, count = 7", t.count() == 7);
        check("inorder IDs sorted = 20 30 40 50 60 70 80", idsOf(t.inorder()) == "20 30 40 50 60 70 80");
        check("first inserted student (50) is root", t.rootID() == 50);
        check("duplicate ID 40 is rejected", t.insert(40, "Duplicate Name", 1.0f) == false);
        check("count still 7 after duplicate", t.count() == 7);
        check("original record of ID 40 not overwritten", t.search(40)->name == "SETHI");
        check("new ID 55 accepted", t.insert(55, "Hina Malik", 3.20f) == true);
        check("count = 8 after adding 55", t.count() == 8);
        check("inorder still sorted after adding 55", idsOf(t.inorder()) == "20 30 40 50 55 60 70 80");
    }
    {
        // sorted order insertion => skewed tree, must still work
        StudentBST t;
        for (int id = 1; id <= 5; id++) t.insert(id, "S" + to_string(id), 3.0f);
        check("ascending insertion (skewed tree) inorder = 1 2 3 4 5", idsOf(t.inorder()) == "1 2 3 4 5");
        check("ascending insertion count = 5", t.count() == 5);
    }

    cout << "\n===== TEST GROUP C : Search =====\n";
    {
        StudentBST t;
        loadSampleData(t);
        const Student* s = t.search(40);
        check("search 40 -> found", s != nullptr);
        check("search 40 -> name SETHI", s && s->name == "SETHI");
        check("search 40 -> GPA 3.00", s && sameFloat(s->GPA, 3.00f));
        check("search root 50 -> found", t.search(50) != nullptr);
        check("search leaf 80 -> found", t.search(80) != nullptr);
        check("search 99 (not present) -> null", t.search(99) == nullptr);
        check("search 0 -> null", t.search(0) == nullptr);
        check("search negative ID -> null", t.search(-5) == nullptr);
    }

    cout << "\n===== TEST GROUP D : Analytics (highest, lowest, threshold) =====\n";
    {
        StudentBST t;
        loadSampleData(t);
        check("highest GPA = student 80 (3.95)", idsOf(t.highestGPA()) == "80");
        check("lowest GPA = student 20 (2.10)", idsOf(t.lowestGPA()) == "20");
        check("GPA above 3.0 = 50 60 70 80 (ID 40 with exact 3.00 excluded)", idsOf(t.aboveGPA(3.0f)) == "50 60 70 80");
        check("GPA above 3.5 = 60 70 80", idsOf(t.aboveGPA(3.5f)) == "60 70 80");
        check("GPA above 4.0 = nobody", t.aboveGPA(4.0f).empty());
        check("GPA above 0.0 = all 7 students", t.aboveGPA(0.0f).size() == 7);
        check("GPA above 3.90 = only 80 (70 with exact 3.90 excluded)", idsOf(t.aboveGPA(3.90f)) == "80");

        // tie cases
        t.insert(90, "Imran Tie", 3.95f);
        t.insert(10, "Jawad Tie", 2.10f);
        check("tie for highest GPA -> 80 and 90 both reported", idsOf(t.highestGPA()) == "80 90");
        check("tie for lowest GPA -> 10 and 20 both reported", idsOf(t.lowestGPA()) == "10 20");
    }
    {
        StudentBST t;
        t.insert(1, "Only One", 2.75f);
        check("single student is both highest and lowest", idsOf(t.highestGPA()) == "1" && idsOf(t.lowestGPA()) == "1");
    }

    cout << "\n===== TEST GROUP E : Delete (all cases) =====\n";
    {
        // Tree shape:        50
        //                 30    70
        //               20 40  60 80
        StudentBST t;
        loadSampleData(t);

        check("delete leaf 20 -> true", t.remove(20) == true);
        check("after deleting 20: inorder = 30 40 50 60 70 80", idsOf(t.inorder()) == "30 40 50 60 70 80");
        check("after deleting 20: count = 6", t.count() == 6);
        check("deleted ID 20 cannot be searched now", t.search(20) == nullptr);

        // now 30 has only right child (40)
        check("delete node having ONE child (30) -> true", t.remove(30) == true);
        check("after deleting 30: inorder = 40 50 60 70 80", idsOf(t.inorder()) == "40 50 60 70 80");
        check("student 40 still reachable after parent deleted", t.search(40) != nullptr);

        // 70 has two children (60, 80)
        check("delete node having TWO children (70) -> true", t.remove(70) == true);
        check("after deleting 70: inorder = 40 50 60 80", idsOf(t.inorder()) == "40 50 60 80");
        check("after deleting 70: 80 and 60 both still present", t.search(80) && t.search(60));

        // root deletion (two children: 40 and 80)
        check("delete ROOT 50 -> true", t.remove(50) == true);
        check("after deleting root: inorder = 40 60 80", idsOf(t.inorder()) == "40 60 80");
        check("new root is inorder successor (60)", t.rootID() == 60);

        check("delete non-existing ID 999 -> false", t.remove(999) == false);
        check("count unchanged (3) after failed delete", t.count() == 3);
        check("delete same ID again (50) -> false", t.remove(50) == false);

        // delete everything, tree must become empty
        check("delete 40", t.remove(40));
        check("delete 60", t.remove(60));
        check("delete 80", t.remove(80));
        check("tree is empty after deleting all", t.count() == 0 && t.inorder().empty());
        check("can insert again after becoming empty", t.insert(5, "Fresh Start", 3.3f) && t.count() == 1);
    }
    {
        // deleting max GPA student must update analytics
        StudentBST t;
        loadSampleData(t);
        t.remove(80);
        check("after deleting top student, new highest = 70 (3.90)", idsOf(t.highestGPA()) == "70");
        t.remove(20);
        check("after deleting lowest student, new lowest = 30 (2.80)", idsOf(t.lowestGPA()) == "30");
    }

    cout << "\n==================================================\n";
    cout << "TOTAL PASSED : " << passed << "\nTOTAL FAILED : " << failed << "\n";
}

// ---------------- Menu driven main ----------------
void showMenu() {
    cout << "\n=========== STUDENT RECORD MANAGEMENT (BST) ===========\n"
         << " 1. Insert new student\n"
         << " 2. Search student by ID\n"
         << " 3. Delete student by ID\n"
         << " 4. Display all students (sorted by ID)\n"
         << " 5. Count total students\n"
         << " 6. Show highest and lowest GPA students\n"
         << " 7. Show students with GPA above a threshold\n"
                 << " 0. Exit\n";
}

int main(int argc, char* argv[]) {
    if (argc > 1 && string(argv[1]) == "test") {
        runTests();
        return failed == 0 ? 0 : 1;
    }

    StudentBST bst;
    loadSampleData(bst);                       // sample students are ready at start, more can be added with option 1
    int choice = -1;

    while (choice != 0) {
        showMenu();
        if (!readNum("Enter your choice : ", choice)) break;   // EOF -> exit

        if (choice == 1) {
            int id; string name; float gpa;
            if (!readNum("   Student ID : ", id)) break;
            if (id <= 0) { cout << "   ID should be a positive number.\n"; continue; }
            if (!readLine("   Name       : ", name)) break;
            if (name.empty()) { cout << "   Name cannot be empty.\n"; continue; }
            if (!readGPA("   GPA (0-4)  : ", gpa)) break;
            if (bst.insert(id, name, gpa)) cout << "   Student added successfully.\n";
            else cout << "   Student with ID " << id << " already exists. Not added.\n";

        } else if (choice == 2) {
            int id;
            if (!readNum("   Enter ID to search : ", id)) break;
            const Student* s = bst.search(id);
            if (s) { cout << "   Record found:\n"; printStudent(s); }
            else cout << "   No student found with ID " << id << ".\n";

        } else if (choice == 3) {
            int id;
            if (!readNum("   Enter ID to delete : ", id)) break;
            if (bst.remove(id)) cout << "   Student " << id << " deleted successfully.\n";
            else cout << "   No student found with ID " << id << ". Nothing deleted.\n";

        } else if (choice == 4) {
            printList(bst.inorder(), "No records available.");

        } else if (choice == 5) {
            cout << "   Total students : " << bst.count() << "\n";

        } else if (choice == 6) {
            if (bst.count() == 0) { cout << "   No records available.\n"; continue; }
            cout << "   Highest GPA:\n"; printList(bst.highestGPA(), "");
            cout << "   Lowest GPA:\n";  printList(bst.lowestGPA(), "");

        } else if (choice == 7) {
            float th;
            if (!readGPA("   Enter GPA threshold (e.g. 3.0) : ", th)) break;
            cout << "   Students with GPA above " << fixed << setprecision(2) << th << ":\n";
            printList(bst.aboveGPA(th), "Nobody has GPA above this threshold.");

        } else if (choice != 0) {
            cout << "   Wrong choice, please select from 0 to 7.\n";
        }
    }

    cout << "\nExiting. Allah Hafiz!\n";
    return 0;
}