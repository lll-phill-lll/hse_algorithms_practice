#include <iostream>
#include <vector>
#include <string>


// Disclaimer:
// This is quick version from seminar
// for understanding basic construction
// of class.
// will be changed to normal realization

class PStack {
private:
    struct Node {
        Node* prev;
        int val;

        Node(): prev(nullptr) {}
        Node(Node* prev, int val): prev(prev), val(val) {}
    };
    std::vector<Node*> versions;
    int last_index = 0;

public:
    PStack() {
        versions.push_back(new Node());
    }

    // don't delete twice one node
    ~PStack() {
        // TODO
    }

    int pop(int version) {
        Node* node = versions[version];

        if (node->prev) {
            versions.push_back(node->prev);
        }

        return node->val;
    }

    void push(int version, int val) {
        Node* node = versions[version];
        versions.push_back(new Node(node, val));
    }


};

int main() {
    PStack stack;
    int n;
    std::cin >> n;

    std::string operation;
    int version;
    int value;

    for (int i = 0; i != n; ++i) {
        std::cin >> operation;
        if (operation == "push") {
            std::cin >> value >> version;
            stack.push(value, version);
        } else if (operation == "pop") {
            std::cin >> version;
            int res = stack.pop(version);
            std::cout << res << std::endl;
        } else {
            std::cout << "no such operation" << std::endl;
        }
    }

    return 0;
}
