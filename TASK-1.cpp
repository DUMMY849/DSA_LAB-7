#include <iostream>
#include <string>
#include <stack>
#include <vector>
#include <algorithm>
#include <cctype>
using namespace std;
// Basically, this is one node of the character tree (labels like a, b, c ...)
struct Node {
    char data;
    Node* left;
    Node* right;
    Node(char c) : data(c), left(nullptr), right(nullptr) {}
};

// Separate node for the integer BST
struct IntNode {
    int key;
    IntNode* left;
    IntNode* right;
    IntNode(int k) : key(k), left(nullptr), right(nullptr) {}
};

//                                        helper functions
// Removing spaces from user input, otherwise "a (b, c)" will give unnecessary error
string removeSpaces(const string& s) {
    string out;
    for (char ch : s)
        if (!isspace((unsigned char)ch)) out += ch;
    return out;
}

// Joining vector of strings with single space ( for nice printing)
string joinWithSpace(const vector<string>& v) {
    string out;
    for (size_t i = 0; i < v.size(); i++) {
        if (i) out += " ";
        out += v[i];
    }
    return out;
}
//  STEP 5 : Validate the string WITHOUT building the tree
// Recursive grammar checker. 'i' is passed by reference so that it moves ahead as we consume characters. We never create any node here, only walking on string.
// 'optional' = true means empty subtree is allowed at this place (child position).
bool checkNode(const string& s, size_t& i, bool optional, string& err) {
    size_t n = s.size();

    // Empty place: either end of string, or next char is ',' or ')'
    if (i >= n || s[i] == ',' || s[i] == ')') {
        if (optional) return true;
        err = "node label missing at position " + to_string(i);
        return false;
    }

    // Label must be letter or digit only
    if (!isalnum((unsigned char)s[i])) {
        err = string("unexpected character '") + s[i] + "' at position " + to_string(i);
        return false;
    }
    i++;  // label consumed

    // If bracket is there, then it must follow:  ( left , right )
    if (i < n && s[i] == '(') {
        i++;
        if (!checkNode(s, i, true, err)) return false;   // left child

        if (i >= n || s[i] != ',') {
            err = "comma expected at position " + to_string(i) +
                  " (both child places are needed, leave empty for missing child)";
            return false;
        }
        i++;

        if (!checkNode(s, i, true, err)) return false;   // right child

        if (i >= n || s[i] != ')') {
            err = "closing bracket expected at position " + to_string(i);
            return false;
        }
        i++;
    }
    return true;
}

// Returns empty string if everything is fine, otherwise returns the reason of failure.
string findProblem(const string& s) {
    if (s.empty()) return "string is empty";

    // Check 1 : only allowed characters
    for (size_t k = 0; k < s.size(); k++) {
        char c = s[k];
        if (!(isalnum((unsigned char)c) || c == '(' || c == ')' || c == ','))
            return string("invalid character '") + c + "' at position " + to_string(k);
    }

    // Check 2 : brackets balanced. Simple counter is enough, no need of stack here.
    int open = 0;
    for (size_t k = 0; k < s.size(); k++) {
        if (s[k] == '(') open++;
        else if (s[k] == ')') {
            open--;
            if (open < 0) return "extra ')' found at position " + to_string(k);
        }
    }
    if (open > 0) return "brackets not balanced, " + to_string(open) + " '(' not closed";

    // Check 3 : correct child placement (grammar)
    size_t i = 0;
    string err;
    if (!checkNode(s, i, false, err)) return err;
    if (i != s.size()) return "extra characters after tree ends at position " + to_string(i);

    return "";  // no problem
}

bool isValidTreeString(const string& s) { return findProblem(s).empty(); }

//  STEP 1 : Build the tree from string (call only after validation)

Node* parseNode(const string& s, size_t& i) {
    // Empty child place -> nullptr
    if (i >= s.size() || s[i] == ',' || s[i] == ')') return nullptr;

    Node* node = new Node(s[i++]);          // take label

    if (i < s.size() && s[i] == '(') {
        i++;                                 // skip '('
        node->left = parseNode(s, i);        // left subtree
        i++;                                 // skip ','
        node->right = parseNode(s, i);       // right subtree
        i++;                                 // skip ')'
    }
    return node;
}

Node* buildTree(const string& s) {
    size_t i = 0;
    return parseNode(s, i);
}

// Memory cleanup, otherwise leak will happen. Post-order delete is the safest.
void freeTree(Node* root) {
    if (!root) return;
    freeTree(root->left);
    freeTree(root->right);
    delete root;
}

//       Preorder (recursive)
// Plain preorder: root, left, right
void preorder(Node* root, vector<string>& out) {
    if (!root) return;
    out.push_back(string(1, root->data));
    preorder(root->left, out);
    preorder(root->right, out);
}

// Preorder in the "string format" (same style as input). Round trip should give same string.
string toTreeString(Node* root) {
    if (!root) return "";
    if (!root->left && !root->right) return string(1, root->data);   // leaf, no bracket
    return string(1, root->data) + "(" + toTreeString(root->left) + "," +
           toTreeString(root->right) + ")";
}
//Inorder using stack (no recursion)
void inorderIterative(Node* root, vector<string>& out) {
    stack<Node*> st;
    Node* cur = root;

    while (cur != nullptr || !st.empty()) {
        // Go as left as possible, pushing everything on the way
        while (cur != nullptr) {
            st.push(cur);
            cur = cur->left;
        }
        // Now top of stack is the leftmost pending node, visit it
        cur = st.top();
        st.pop();
        out.push_back(string(1, cur->data));
        // After visiting, go to right subtree
        cur = cur->right;
    }
}

//   Postorder iterative (single stack + 'last' pointer)
void postorderIterative(Node* root, vector<string>& out) {
    stack<Node*> st;
    Node* cur = root;
    Node* lastVisited = nullptr;

    while (cur != nullptr || !st.empty()) {
        while (cur != nullptr) {
            st.push(cur);
            cur = cur->left;
        }
        Node* top = st.top();

        // If right child exists and not yet visited, go there first
        if (top->right != nullptr && lastVisited != top->right) {
            cur = top->right;
        } else {
            // Both children are done, so now visit this node
            out.push_back(string(1, top->data));
            lastVisited = top;
            st.pop();
        }
    }
}   


//  Height and depth

// Height measured in EDGES. Single node = 0, empty tree = -1
int height(Node* root) {
    if (!root) return -1;
    return 1 + max(height(root->left), height(root->right));
}

// Depth of every node = distance from root (root has depth 0).
// We pass depth downwards while going recursively.
void collectDepths(Node* root, int depth, vector<string>& out) {
    if (!root) return;
    out.push_back(string(1, root->data) + ":" + to_string(depth));
    collectDepths(root->left, depth + 1, out);
    collectDepths(root->right, depth + 1, out);
}

void printDepthTable(Node* root, int depth = 0) {
    if (!root) return;
    cout << "   Node " << root->data << "  ->  depth = " << depth
         << ",  height = " << height(root) << "\n";
    printDepthTable(root->left, depth + 1);
    printDepthTable(root->right, depth + 1);
}

// BST from sequence of integers
// Standard BST insert. Duplicate keys are ignored (we keep only first one).
IntNode* bstInsert(IntNode* root, int key) {
    if (!root) return new IntNode(key);
    if (key < root->key) root->left = bstInsert(root->left, key);
    else if (key > root->key) root->right = bstInsert(root->right, key);
    // equal -> do nothing
    return root;
}

IntNode* buildBST(const vector<int>& values) {
    IntNode* root = nullptr;
    for (int v : values) root = bstInsert(root, v);   // first element automatically becomes root
    return root;
}

void bstInorder(IntNode* root, vector<string>& out) {
    if (!root) return;
    bstInorder(root->left, out);
    out.push_back(to_string(root->key));
    bstInorder(root->right, out);
}

int bstHeight(IntNode* root) {
    if (!root) return -1;
    return 1 + max(bstHeight(root->left), bstHeight(root->right));
}

void freeBST(IntNode* root) {
    if (!root) return;
    freeBST(root->left);
    freeBST(root->right);
    delete root;
}

//  Demo (all steps one by one)
void runAllSteps(const string& input) {
    cout << "\nInput string : " << input << "\n";

    // Step 5 first, because no point in building an invalid tree
    string problem = findProblem(input);
    if (!problem.empty()) {
        cout << "Validation   : INVALID  (" << problem << ")\n";
        return;
    }
    cout << "Validation   : VALID\n";

    Node* root = buildTree(input);
    cout << "Tree built successfully.\n\n";

    vector<string> v;

    preorder(root, v);
    cout << "Preorder (recursive)        : " << joinWithSpace(v) << "\n";
    cout << "Preorder (string format)    : " << toTreeString(root) << "\n";

    v.clear();
    inorderIterative(root, v);
    cout << "Inorder (stack, iterative)  : " << joinWithSpace(v) << "\n";

    v.clear();
    postorderIterative(root, v);
    cout << "Postorder (stack, iterative): " << joinWithSpace(v) << "\n";

    cout << "\nHeight of tree (in edges)   : " << height(root) << "\n";
    cout << "Depth / height of each node :\n";
    printDepthTable(root);

    freeTree(root);
}

void runBSTDemo() {
    vector<int> values = {50, 30, 70, 20, 40, 60, 80};
    cout << "\n=== BST from integer sequence ===\nInput list : ";
    for (int x : values) cout << x << " ";
    IntNode* root = buildBST(values);
    vector<string> v;
    bstInorder(root, v);
    cout << "\nInorder    : " << joinWithSpace(v) << "   (should come sorted)\n";
    cout << "BST height : " << bstHeight(root) << "\n";
    freeBST(root);
}
//  test cases
static int passed = 0, failed = 0;

void check(const string& name, bool ok) {
    if (ok) { passed++; cout << "  [PASS] " << name << "\n"; }
    else    { failed++; cout << "  [FAIL] " << name << "\n"; }
}

void runTests() {
    cout << "===== TEST GROUP A : Validation (without building tree) =====\n";
    struct VCase { string s; bool expected; string why; };
    vector<VCase> vc = {
        {"a(b(d,e),c(,f(g,)))", true,  "given string from lab"},
        {"a",                   true,  "single node only"},
        {"a(b,c)",              true,  "simple full tree"},
        {"a(b,)",               true,  "only left child"},
        {"a(,c)",               true,  "only right child"},
        {"a(,)",                true,  "empty brackets with both children missing"},
        {"a(b(c(d,),),)",       true,  "left skewed tree"},
        {"a(,b(,c))",           true,  "right skewed tree"},
        {"1(2,3)",              true,  "digits as labels"},
        {"",                    false, "empty string"},
        {"a(b,c",               false, "closing bracket missing"},
        {"a(b,c))",             false, "extra closing bracket"},
        {"a)(b,c)",             false, "closing comes before opening"},
        {"(b,c)",               false, "root label missing"},
        {"a(b)",                false, "comma missing, child placement wrong"},
        {"a(b,,c)",             false, "too many commas"},
        {"a(b,c,d)",            false, "three children not allowed"},
        {"ab",                  false, "two labels without bracket"},
        {"a(b,c)d",             false, "extra character after tree"},
        {"a(b,c)(d,e)",         false, "two bracket groups for one node"},
        {"a(b,c$)",             false, "invalid character"},
        {"a(b(d,e),c(,f(g,))",  false, "one bracket missing in long string"},
        {"a(b c,d)",            false, "space inside (spaces are removed only in interactive mode)"},
    };
    for (auto& t : vc) {
        bool got = isValidTreeString(t.s);
        check("\"" + t.s + "\"  ->  " + (t.expected ? "valid" : "invalid") + "   (" + t.why + ")",
              got == t.expected);
    }

    cout << "\n===== TEST GROUP B : Build + traversals + height + round trip =====\n";
    struct TCase { string s, pre, in, post, depths, roundTrip; int h; };
    vector<TCase> tc = {
        {"a(b(d,e),c(,f(g,)))", "a b d e c f g", "d b e a c g f", "d e b g f c a",
         "a:0 b:1 d:2 e:2 c:1 f:2 g:3", "a(b(d,e),c(,f(g,)))", 3},
        {"a", "a", "a", "a", "a:0", "a", 0},
        {"a(b,c)", "a b c", "b a c", "b c a", "a:0 b:1 c:1", "a(b,c)", 1},
        {"a(b(c(d,),),)", "a b c d", "d c b a", "d c b a", "a:0 b:1 c:2 d:3", "a(b(c(d,),),)", 3},
        {"a(,b(,c))", "a b c", "a b c", "c b a", "a:0 b:1 c:2", "a(,b(,c))", 2},
        {"a(,)", "a", "a", "a", "a:0", "a", 0},   // round trip becomes simple 'a'
    };
    for (auto& t : tc) {
        Node* root = buildTree(t.s);
        vector<string> p, i, q, d;
        preorder(root, p);
        inorderIterative(root, i);
        postorderIterative(root, q);
        collectDepths(root, 0, d);
        check(t.s + "  preorder   = " + t.pre,   joinWithSpace(p) == t.pre);
        check(t.s + "  inorder    = " + t.in,    joinWithSpace(i) == t.in);
        check(t.s + "  postorder  = " + t.post,  joinWithSpace(q) == t.post);
        check(t.s + "  depths     = " + t.depths, joinWithSpace(d) == t.depths);
        check(t.s + "  height     = " + to_string(t.h), height(root) == t.h);
        check(t.s + "  round trip = " + t.roundTrip, toTreeString(root) == t.roundTrip);
        freeTree(root);
    }
    check("empty tree (nullptr) height = -1", height(nullptr) == -1);
    {
        vector<string> e;
        inorderIterative(nullptr, e);
        postorderIterative(nullptr, e);
        preorder(nullptr, e);
        check("empty tree gives no output in all three traversals", e.empty());
    }

    cout << "\n===== TEST GROUP C : BST from integer list =====\n";
    struct BCase { vector<int> in; string expected; int h; string why; };
    vector<BCase> bc = {
        {{50, 30, 70, 20, 40, 60, 80}, "20 30 40 50 60 70 80", 2, "example from lab (balanced)"},
        {{1, 2, 3, 4},                 "1 2 3 4",              3, "ascending input, skewed right"},
        {{4, 3, 2, 1},                 "1 2 3 4",              3, "descending input, skewed left"},
        {{5, 3, 5, 3, 8},              "3 5 8",                1, "duplicates ignored"},
        {{42},                         "42",                   0, "single element"},
        {{},                           "",                    -1, "empty list"},
        {{-10, 0, 10, -20},            "-20 -10 0 10",         2, "negative numbers"},
    };
    for (auto& t : bc) {
        IntNode* r = buildBST(t.in);
        vector<string> o;
        bstInorder(r, o);
        check("BST inorder = [" + t.expected + "]   (" + t.why + ")", joinWithSpace(o) == t.expected);
        check("BST height  = " + to_string(t.h) + "   (" + t.why + ")", bstHeight(r) == t.h);
        freeBST(r);
    }
    {
        IntNode* r = buildBST({50, 30, 70});
        check("first element becomes root (50)", r != nullptr && r->key == 50);
        freeBST(r);
    }

    cout << "\n==================================================\n";
    cout << "TOTAL PASSED : " << passed << "\nTOTAL FAILED : " << failed << "\n";
}
//  main
int main(int argc, char* argv[]) {
    if (argc > 1 && string(argv[1]) == "test") {
        runTests();
        return failed == 0 ? 0 : 1;
    }

    cout << "********** TASK 1 (Binary Tree) **********\n";

    // Given string from the lab manual
    runAllSteps("a(b(d,e),c(,f(g,)))");

    // Last step of the task : BST from list of integers
    runBSTDemo();

    // Now user can try own strings
    cout << "\n=== Try your own string (type q to quit) ===\n";
    string line;
    while (true) {
        cout << "\nEnter tree string : ";
        if (!getline(cin, line)) break;       // handles Ctrl+D also
        line = removeSpaces(line);
        if (line == "q" || line == "Q") break;
        runAllSteps(line);
    }
    cout << "\nBye bye!\n";
    return 0;
}