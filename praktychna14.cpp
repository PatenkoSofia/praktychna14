#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct Node {
    int value;
    Node* left;
    Node* right;
};

std::size_t allocatedNodes = 0;
std::size_t freedNodes = 0;

std::size_t activeNodes() {
    return allocatedNodes - freedNodes;
}

Node* createNode(int value) {
    Node* node = new Node{value, nullptr, nullptr};
    ++allocatedNodes;
    return node;
}

void destroyTree(Node* node) {
    if (node == nullptr) {
        return;
    }
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
    ++freedNodes;
}

void clearTree(Node*& root) {
    destroyTree(root);
    root = nullptr;
}

bool isEmpty(const Node* root) {
    return root == nullptr;
}

Node* readPrefix(std::istream& in) {
    std::string token;
    if (!(in >> token)) {
        throw std::runtime_error("неочікуваний кінець опису дерева");
    }
    if (token == "#") {
        return nullptr;
    }
    std::size_t used = 0;
    const int value = std::stoi(token, &used);
    if (used != token.size()) {
        throw std::runtime_error("некоректне значення: " + token);
    }
    Node* node = createNode(value);
    try {
        node->left = readPrefix(in);
        node->right = readPrefix(in);
    } catch (...) {
        clearTree(node);
        throw;
    }
    return node;
}

void writePrefix(const Node* node, std::vector<std::string>& tokens) {
    if (node == nullptr) {
        tokens.push_back("#");
        return;
    }
    tokens.push_back(std::to_string(node->value));
    writePrefix(node->left, tokens);
    writePrefix(node->right, tokens);
}

Node* insertBst(Node* root, int value) {
    if (root == nullptr) {
        return createNode(value);
    }
    if (value < root->value) {
        root->left = insertBst(root->left, value);
    } else {
        root->right = insertBst(root->right, value);
    }
    return root;
}

void preorderRec(const Node* node, std::vector<int>& out) {
    if (node == nullptr) {
        return;
    }
    out.push_back(node->value);
    preorderRec(node->left, out);
    preorderRec(node->right, out);
}

void inorderRec(const Node* node, std::vector<int>& out) {
    if (node == nullptr) {
        return;
    }
    inorderRec(node->left, out);
    out.push_back(node->value);
    inorderRec(node->right, out);
}

void postorderRec(const Node* node, std::vector<int>& out) {
    if (node == nullptr) {
        return;
    }
    postorderRec(node->left, out);
    postorderRec(node->right, out);
    out.push_back(node->value);
}

std::vector<int> preorder(const Node* root) {
    std::vector<int> out;
    preorderRec(root, out);
    return out;
}

std::vector<int> inorder(const Node* root) {
    std::vector<int> out;
    inorderRec(root, out);
    return out;
}

std::vector<int> postorder(const Node* root) {
    std::vector<int> out;
    postorderRec(root, out);
    return out;
}

std::size_t countNodes(const Node* node) {
    if (node == nullptr) {
        return 0;
    }
    return 1 + countNodes(node->left) + countNodes(node->right);
}

std::size_t countLeaves(const Node* node) {
    if (node == nullptr) {
        return 0;
    }
    if (node->left == nullptr && node->right == nullptr) {
        return 1;
    }
    return countLeaves(node->left) + countLeaves(node->right);
}

std::size_t countInternal(const Node* root) {
    return countNodes(root) - countLeaves(root);
}

std::size_t height(const Node* node) {
    if (node == nullptr) {
        return 0;
    }
    return 1 + std::max(height(node->left), height(node->right));
}

class ArrayStack {
public:
    explicit ArrayStack(std::size_t capacity)
        : data_(new const Node*[capacity]), capacity_(capacity), size_(0) {}
    ~ArrayStack() {
        delete[] data_;
    }
    ArrayStack(const ArrayStack&) = delete;
    ArrayStack& operator=(const ArrayStack&) = delete;

    void push(const Node* node) {
        if (size_ == capacity_) {
            throw std::runtime_error("переповнення допоміжного стека");
        }
        data_[size_++] = node;
    }
    const Node* pop() {
        return data_[--size_];
    }
    const Node* top() const {
        return data_[size_ - 1];
    }
    bool empty() const {
        return size_ == 0;
    }

private:
    const Node** data_;
    std::size_t capacity_;
    std::size_t size_;
};

std::vector<int> preorderIterative(const Node* root) {
    std::vector<int> out;
    ArrayStack stack(countNodes(root) + 1);
    if (root != nullptr) {
        stack.push(root);
    }
    while (!stack.empty()) {
        const Node* node = stack.pop();
        out.push_back(node->value);
        if (node->right != nullptr) {
            stack.push(node->right);
        }
        if (node->left != nullptr) {
            stack.push(node->left);
        }
    }
    return out;
}

std::vector<int> inorderIterative(const Node* root) {
    std::vector<int> out;
    ArrayStack stack(countNodes(root) + 1);
    const Node* current = root;
    while (current != nullptr || !stack.empty()) {
        while (current != nullptr) {
            stack.push(current);
            current = current->left;
        }
        current = stack.pop();
        out.push_back(current->value);
        current = current->right;
    }
    return out;
}

std::vector<int> postorderIterative(const Node* root) {
    std::vector<int> out;
    ArrayStack stack(countNodes(root) + 1);
    const Node* current = root;
    const Node* last = nullptr;
    while (current != nullptr || !stack.empty()) {
        if (current != nullptr) {
            stack.push(current);
            current = current->left;
        } else {
            const Node* peek = stack.top();
            if (peek->right != nullptr && peek->right != last) {
                current = peek->right;
            } else {
                out.push_back(peek->value);
                last = stack.pop();
            }
        }
    }
    return out;
}

bool collectUnique(const Node* node, std::set<const Node*>& seen) {
    if (node == nullptr) {
        return true;
    }
    if (!seen.insert(node).second) {
        return false;
    }
    return collectUnique(node->left, seen) && collectUnique(node->right, seen);
}

std::string format(const std::vector<int>& values) {
    if (values.empty()) {
        return "порожньо";
    }
    std::ostringstream out;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i != 0) {
            out << ", ";
        }
        out << values[i];
    }
    return out.str();
}

std::string join(const std::vector<std::string>& tokens) {
    std::string result;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        if (i != 0) {
            result += ' ';
        }
        result += tokens[i];
    }
    return result;
}

void printTree(const Node* node, std::size_t depth) {
    if (node == nullptr) {
        return;
    }
    printTree(node->right, depth + 1);
    std::cout << std::string(depth * 4, ' ') << node->value << '\n';
    printTree(node->left, depth + 1);
}

struct TestCase {
    int number;
    const char* description;
    const char* prefix;
    std::vector<int> preorderExpected;
    std::vector<int> inorderExpected;
    std::vector<int> postorderExpected;
    std::size_t nodes;
    std::size_t leaves;
    std::size_t height;
};

const TestCase kCases[] = {
    {1, "порожнє дерево", "#", {}, {}, {}, 0, 0, 0},
    {2, "один вузол", "5 # #", {5}, {5}, {5}, 1, 1, 1},
    {3, "три вузли, корінь посередині", "2 1 # # 3 # #", {2, 1, 3}, {1, 2, 3}, {1, 3, 2}, 3, 2, 2},
    {4, "ліве вироджене дерево", "3 2 1 # # # #", {3, 2, 1}, {1, 2, 3}, {1, 2, 3}, 3, 1, 3},
    {5, "праве вироджене дерево", "1 # 2 # 3 # #", {1, 2, 3}, {1, 2, 3}, {3, 2, 1}, 3, 1, 3},
    {6, "корінь із двома листками", "4 2 # # 6 # #", {4, 2, 6}, {2, 4, 6}, {2, 6, 4}, 3, 2, 2},
    {7, "сім вузлів, неповне піддерево", "5 3 2 # # 4 # # 7 6 # # 8 # #", {5, 3, 2, 4, 7, 6, 8}, {2, 3, 4, 5, 6, 7, 8}, {2, 4, 3, 6, 8, 7, 5}, 7, 4, 3},
    {8, "ліве піддерево з глибиною", "1 2 # # 3 4 # # #", {1, 2, 3, 4}, {2, 1, 4, 3}, {2, 4, 3, 1}, 4, 2, 3},
    {9, "від'ємне значення", "0 -2 # # 5 # #", {0, -2, 5}, {-2, 0, 5}, {-2, 5, 0}, 3, 2, 2},
    {10, "однакові значення", "4 4 # # 4 # #", {4, 4, 4}, {4, 4, 4}, {4, 4, 4}, 3, 2, 2},
    {11, "власне повне дерево", "4 2 1 # # 3 # # 6 5 # # 7 # #", {4, 2, 1, 3, 6, 5, 7}, {1, 2, 3, 4, 5, 6, 7}, {1, 3, 2, 5, 7, 6, 4}, 7, 4, 3},
    {12, "власне ліво-вироджене дерево", "5 4 3 2 1 # # # # # #", {5, 4, 3, 2, 1}, {1, 2, 3, 4, 5}, {1, 2, 3, 4, 5}, 5, 1, 5},
    {13, "власне право-вироджене дерево", "1 # 2 # 3 # 4 # 5 # #", {1, 2, 3, 4, 5}, {1, 2, 3, 4, 5}, {5, 4, 3, 2, 1}, 5, 1, 5},
    {14, "власне неповне дерево", "10 5 # 7 # # 15 12 # # #", {10, 5, 7, 15, 12}, {5, 7, 10, 12, 15}, {7, 5, 12, 15, 10}, 5, 2, 3},
    {15, "власне дерево з повтореннями", "6 6 2 # # # 6 # 9 # #", {6, 6, 2, 6, 9}, {2, 6, 6, 6, 9}, {2, 6, 9, 6, 6}, 5, 2, 3},
};

bool runCase(const TestCase& tc) {
    std::vector<std::string> errors;
    std::istringstream in(tc.prefix);
    Node* root = readPrefix(in);

    const std::vector<int> pre = preorder(root);
    const std::vector<int> ino = inorder(root);
    const std::vector<int> post = postorder(root);
    const std::size_t nodes = countNodes(root);
    const std::size_t leaves = countLeaves(root);
    const std::size_t internal = countInternal(root);
    const std::size_t h = height(root);

    if (pre != tc.preorderExpected) {
        errors.push_back("прямий обхід");
    }
    if (ino != tc.inorderExpected) {
        errors.push_back("симетричний обхід");
    }
    if (post != tc.postorderExpected) {
        errors.push_back("зворотний обхід");
    }
    if (preorderIterative(root) != pre || inorderIterative(root) != ino || postorderIterative(root) != post) {
        errors.push_back("ітеративні обходи відрізняються від рекурсивних");
    }
    if (pre.size() != nodes || ino.size() != nodes || post.size() != nodes) {
        errors.push_back("кількість елементів в обходах");
    }
    if (!pre.empty() && (pre.front() != root->value || post.back() != root->value)) {
        errors.push_back("корінь не на початку прямого або не в кінці зворотного обходу");
    }
    std::set<const Node*> seen;
    if (!collectUnique(root, seen) || seen.size() != nodes) {
        errors.push_back("цикл або повторне посилання");
    }
    if (nodes != tc.nodes) {
        errors.push_back("кількість вузлів");
    }
    if (leaves != tc.leaves) {
        errors.push_back("кількість листків");
    }
    if (internal + leaves != nodes) {
        errors.push_back("інваріант листки плюс внутрішні");
    }
    if (h != tc.height) {
        errors.push_back("висота");
    }

    clearTree(root);
    if (!isEmpty(root)) {
        errors.push_back("корінь не порожній після очищення");
    }

    std::cout << "№" << tc.number << " " << tc.description << ": прямий " << format(pre)
              << "; симетричний " << format(ino) << "; зворотний " << format(post)
              << "; вузлів " << nodes << ", листків " << leaves << ", висота " << h
              << (errors.empty() ? "; OK" : "; ПОМИЛКА") << '\n';
    for (const std::string& error : errors) {
        std::cout << "    помилка: " << error << '\n';
    }
    return errors.empty();
}

bool runVariant() {
    const int sequence[] = {49, 24, 67, 11, 35, 57, 80};
    Node* root = nullptr;
    for (int value : sequence) {
        root = insertBst(root, value);
    }

    std::vector<std::string> tokens;
    writePrefix(root, tokens);
    const std::string description = join(tokens);

    std::istringstream in(description);
    Node* rebuilt = readPrefix(in);
    const bool sameShape = preorder(rebuilt) == preorder(root) && inorder(rebuilt) == inorder(root)
        && postorder(rebuilt) == postorder(root);
    clearTree(rebuilt);

    const std::vector<int> pre = preorder(root);
    const std::vector<int> ino = inorder(root);
    const std::vector<int> post = postorder(root);
    const std::size_t nodes = countNodes(root);
    const std::size_t leaves = countLeaves(root);
    const std::size_t internal = countInternal(root);
    const std::size_t h = height(root);

    std::cout << "Варіант 22. Послідовність створення: 49, 24, 67, 11, 35, 57, 80\n";
    std::cout << "Префіксний опис з порожніми гілками: " << description << '\n';
    std::cout << "Прямий обхід: " << format(pre) << '\n';
    std::cout << "Симетричний обхід: " << format(ino) << '\n';
    std::cout << "Зворотний обхід: " << format(post) << '\n';
    std::cout << "Вузлів: " << nodes << ", листків: " << leaves << ", внутрішніх: " << internal
              << ", висота: " << h << '\n';
    std::cout << "Прямий обхід ітеративний: " << format(preorderIterative(root)) << '\n';
    std::cout << "Опис відтворює те саме дерево: " << (sameShape ? "так" : "ні") << '\n';
    std::cout << "Схема дерева (праві гілки вгорі):\n";
    printTree(root, 0);

    const bool ok = sameShape && pre == std::vector<int>({49, 24, 11, 35, 67, 57, 80})
        && ino == std::vector<int>({11, 24, 35, 49, 57, 67, 80})
        && post == std::vector<int>({11, 35, 24, 57, 80, 67, 49})
        && nodes == 7 && leaves == 4 && internal == 3 && h == 3
        && preorderIterative(root) == pre && inorderIterative(root) == ino && postorderIterative(root) == post;

    clearTree(root);
    std::cout << "Після очищення активних вузлів: " << activeNodes() << '\n';
    return ok && activeNodes() == 0;
}

}

int main() {
    try {
        std::cout << "Практична робота 14. Створення та обхід бінарного дерева\n\n";
        const bool variantOk = runVariant();

        std::cout << "\nАвтоматичні тести: 15 дерев, 3 рекурсивні та 3 ітеративні обходи, характеристики\n";
        std::size_t passed = 0;
        const std::size_t total = std::size(kCases);
        for (const TestCase& tc : kCases) {
            if (runCase(tc)) {
                ++passed;
            }
        }

        std::cout << "\nПройдено тестів: " << passed << " з " << total << '\n';
        std::cout << "Створено вузлів: " << allocatedNodes << ", звільнено: " << freedNodes
                  << ", активних: " << activeNodes() << '\n';

        const bool allOk = variantOk && passed == total && activeNodes() == 0;
        std::cout << (allOk ? "Підсумок: успішно" : "Підсумок: є помилки") << '\n';
        return allOk ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << "Помилка: " << e.what() << '\n';
        return 1;
    }
}
