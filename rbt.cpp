
#include <memory>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <cassert>
using namespace std;

template <typename Tkey, typename Tval>
struct RedBlackTree {

private:

    enum Color {
        RED = 0,
        BLACK = 1,
        DOUBLE_BLACK = 2
    };

    struct Node {
        Tkey key;
        Tval val;
        Color color;
        shared_ptr<Node> left;
        shared_ptr<Node> right;
    };

    shared_ptr<Node> root;
    shared_ptr<Node> doubleBlackNilNode;

    static bool is_red(shared_ptr<Node> n) {
        return n != nullptr && n->color == RED;
    }

    static bool is_black(shared_ptr<Node> n) {
        return n == nullptr || n->color == BLACK;
    }

    bool is_double_black(shared_ptr<Node> n) {
        return n != nullptr && (n == doubleBlackNilNode || n->color == DOUBLE_BLACK);
    }

    static void increment_color(shared_ptr<Node> g) {
        assert(g->color == RED || g->color == BLACK);
        g->color = static_cast<Color>(static_cast<int>(g->color) + 1);
    }

    static void decrement_color(shared_ptr<Node> g) {
        assert(g->color == BLACK || g->color == DOUBLE_BLACK);
        g->color = static_cast<Color>(static_cast<int>(g->color) - 1);
    }

    static void push_black(shared_ptr<Node> g) {
        decrement_color(g);
        increment_color(g->left);
        increment_color(g->right);
    }

    void pull_black(shared_ptr<Node> g) {
        increment_color(g);
        if (g->left == doubleBlackNilNode) {
            g->left = nullptr;
        } else {
            decrement_color(g->left);
        }
        if (g->right == doubleBlackNilNode) {
            g->right = nullptr;
        } else {
            decrement_color(g->right);
        }
    }

    static shared_ptr<Node> rotate_right(shared_ptr<Node> g) {
        shared_ptr<Node> u = g->left;
        g->left = u->right;
        u->right = g;
        return u;
    }

    static shared_ptr<Node> flip_right(shared_ptr<Node> g) {
        swap(g->color, g->left->color);
        return rotate_right(g);
    }

    static shared_ptr<Node> rotate_left(shared_ptr<Node> g) {
        shared_ptr<Node> u = g->right;
        g->right = u->left;
        u->left = g;
        return u;
    }

    static shared_ptr<Node> flip_left(shared_ptr<Node> g) {
        swap(g->color, g->right->color);
        return rotate_left(g);
    }

    //------------------------------------------------------------------------------------------------------------
    // Insert
    //------------------------------------------------------------------------------------------------------------

    static shared_ptr<Node> fix_red_edge(shared_ptr<Node> g) {
        if (is_red(g->left) && is_red(g->right)) {
            push_black(g);
            return g;
        }

        //red-edge to the left of g
        if (is_red(g->left) && is_red(g->left->right)) {
            g->left = rotate_left(g->left);
        }
        if (is_red(g->left) && is_red(g->left->left)) {
            return flip_right(g);
        }

        //red-edge to the right of g
        if (is_red(g->right) && is_red(g->right->left)) {
            g->right = rotate_right(g->right);
        }
        if (is_red(g->right) && is_red(g->right->right)) {
            return flip_left(g);
        }

        //no fixes needed
        return g;
    }

    static shared_ptr<Node> insert_helper(shared_ptr<Node> g, Tkey newKey, Tval newVal) {

        if (g == nullptr) {
            g = make_shared<Node>();
            g->key = newKey;
            g->val = newVal;
            g->color = RED;
            return g;

        } else if (g->key == newKey) {
            g->val = newVal;
            return g;

        } else {

            if (newKey < g->key) {
                g->left = insert_helper(g->left, newKey, newVal);
            } else {
                g->right = insert_helper(g->right, newKey, newVal);
            }

            return fix_red_edge(g);
        }
    }

    //------------------------------------------------------------------------------------------------------------
    // Remove
    //------------------------------------------------------------------------------------------------------------

    shared_ptr<Node> splice(shared_ptr<Node> w) {
        if (w->color == RED) {
            assert(w->left == nullptr && w->right == nullptr);
            return nullptr;
        } else if (w->left != nullptr) {
            assert(w->left->color == RED);
            w->left->color = BLACK;
            return w->left;
        } else if (w->right != nullptr) {
            assert(w->right->color == RED);
            w->right->color = BLACK;
            return w->right;
        } else {
            return doubleBlackNilNode;
        }
    }

    shared_ptr<Node> fix_left_doubleblack_right_black(shared_ptr<Node> z) {
        assert(is_double_black(z->left) && is_black(z->right));
        pull_black(z);
        if (is_red(z->right->left) && is_black(z->right->right)) {
            z->right = rotate_right(z->right);
        }
        if (is_red(z->right->right)) {
            z = flip_left(z);
            push_black(z);
        }
        return z;
    }

    shared_ptr<Node> fix_left_doubleblack(shared_ptr<Node> z) {
        assert(is_double_black(z->left));
        if (is_red(z->right)) {
            z = flip_left(z);
            z->left = fix_left_doubleblack_right_black(z->left);
            return z;
        } else {
            return fix_left_doubleblack_right_black(z);
        }
    }

    shared_ptr<Node> fix_right_doubleblack_left_black(shared_ptr<Node> z) {
        assert(is_double_black(z->right) && is_black(z->left));
        pull_black(z);
        if (is_red(z->left->right) && is_black(z->left->left)) {
            z->left = rotate_left(z->left);
        }
        if (is_red(z->left->left)) {
            z = flip_right(z);
            push_black(z);
        }
        return z;
    }

    shared_ptr<Node> fix_right_doubleblack(shared_ptr<Node> z) {
        assert(is_double_black(z->right));
        if (is_red(z->left)) {
            z = flip_right(z);
            z->right = fix_right_doubleblack_left_black(z->right);
            return z;
        } else {
            return fix_right_doubleblack_left_black(z);
        }
    }

    shared_ptr<Node> fix_doubleblack(shared_ptr<Node> z) {
        if (is_double_black(z->left)) {
            return fix_left_doubleblack(z);
        } else if (is_double_black(z->right)) {
            return fix_right_doubleblack(z);
        } else {
            return z;
        }
    }

    shared_ptr<Node> extract_min(shared_ptr<Node> u, shared_ptr<Node> v) {
        //* Find and remove the smallest element from the subtree rooted at u.
        //* Once found, the key and value replace the data stored on the node v.
        //* The return value is the root of the new tree with the smallest value removed.

        if (u->left) {
            //go left, just passing along v unchanged.
            u->left = extract_min(u->left, v);

            //fixup any violations
            return fix_doubleblack(u);

        } else {
            //Before removing u, copy the key and value to v.
            v->key = u->key;
            v->val = u->val;

            //remove u
            return splice(u);
        }
    }

    shared_ptr<Node> remove_helper(shared_ptr<Node> v, Tkey key) {
        //Remove an element equal to key from the subtree rooted at v.
        //Returns the new root of the subtree with key removed.

        if (!v) {
            //no element matching key was found so nothing needs to be removed
            return v;
        }

        if (key == v->key) {
            //we have found the element to remove

            if (!v->right || !v->left) {
                //v has zero or one children, remove it directly
                return splice(v);
            } else {
                //v has both children so remove the smallest element from the right subtree of v.
                //pass in v to allow extract_min to directly replace the key and value from v.
                v->right = extract_min(v->right, v);
            }

        } else if (key < v->key) {
            v->left = remove_helper(v->left, key);
        } else {
            v->right = remove_helper(v->right, key);
        }

        return fix_doubleblack(v);
    }

    //------------------------------------------------------------------------------------------------------------
    // Debug print
    //------------------------------------------------------------------------------------------------------------

    static int debug_print_helper(shared_ptr<Node> v, int indent) {
        //print the tree via a pre-order traversal.  We print the node, then print
        //its children at one larger indent level.
        //returns the black height of the node

        if (indent) {
            cout << setw(indent) << " ";
        }
        if (v) {
            string color = v->color == RED ? "RED" : "BLACK";
            cout << v->key << ":" << color << ":" << v->val << endl;

            int leftBlack = 0, rightBlack = 0;
            if (v->left || v->right) {
                leftBlack = debug_print_helper(v->left, indent+4);
                rightBlack = debug_print_helper(v->right, indent+4);
            }

            //check black-height
            if (leftBlack != rightBlack) {
                cout << "ERROR! left and right children have different black heights" << endl;
            }
            //check red-edge
            if (is_red(v) && (is_red(v->left) || is_red(v->right))) {
                cout << "ERROR! v and a child of v are both red!" << endl;
            }

            //return new black height
            if (v->color == BLACK) {
                return leftBlack + 1;
            } else if (v->color == RED) {
                return leftBlack;
            } else {
                cout << "ERROR! invalid color" << endl;
            }
        } else {
            cout << "nil" << endl;
            return 0;
        }
    }


public:

    RedBlackTree() {
        doubleBlackNilNode = make_shared<Node>();
    }


    void insert(Tkey key, Tval val) {
        root = insert_helper(root, key, val);
        if (root->color == RED) {
            root->color = BLACK;
        }
    }

    void remove(Tkey key) {
        root = remove_helper(root, key);
        if (is_double_black(root)) {
            if (root == doubleBlackNilNode) {
                root = nullptr;
            } else {
                root->color = BLACK;
            }
        }
    }

    void debug_print() {
        debug_print_helper(root, 0);
    }
};

int main() {
    srand(time(0));

    RedBlackTree<int, int> t;

    //insert keys in order, which is the worst for an unbalanced tree
    for (int i = 0; i < 8; i++) {
        t.insert(i, i);
    }
    //print the tree and check the red-black properties
    t.debug_print();

//    //insert random elements
//    RedBlackTree<int, int> t2;
//    for (int i = 0; i < 50; i++) {
//        t2.insert(rand(), -rand());
//    }
//    //check the red-black properties
//    t2.debug_print();
//
//    //remove 27
//    cout << "Removing 27" << endl;
//    t.remove(27);
//    t.debug_print();
//
//
//    //add some more stuff
//    int middleKey = rand();
//    t2.insert(middleKey, -rand());
//
//    //add some more stuff
//    for (int i = 0; i < 50; i++) {
//        t2.insert(rand(), -rand());
//    }
//
//    cout << "New stuff " << endl;
//    t2.debug_print();
//
//    cout << "Removing " << middleKey << endl;
//    t2.remove(middleKey);
//    t2.debug_print();

}