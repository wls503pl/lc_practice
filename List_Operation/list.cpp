#include <cstddef>
#include <initializer_list>
#include <iostream>

class SinglyLinkedList {
private:
    struct Node {
        int value;
        Node* next;

        explicit Node(int v) : value(v), next(nullptr) {}
    };

    Node* head_ = nullptr;
    std::size_t size_ = 0;

public:
    SinglyLinkedList() = default;

    // 由链表对象负责释放节点；禁止浅拷贝，避免重复释放。
    SinglyLinkedList(const SinglyLinkedList&) = delete;
    SinglyLinkedList& operator=(const SinglyLinkedList&) = delete;

    ~SinglyLinkedList() {
        destroy();
    }

    // 创建/重建链表：原有节点先销毁，再按 values 的顺序建立新链表。
    void create(std::initializer_list<int> values) {
        destroy();
        Node* tail = nullptr;

        for (int value : values) {
            Node* node = new Node(value);
            if (head_ == nullptr) {
                head_ = node;
            } else {
                tail->next = node;
            }
            tail = node;
            ++size_;
        }
    }

    // 在索引 index 处插入；合法范围为 [0, size_]。
    bool insertAt(std::size_t index, int value) {
        if (index > size_) {
            return false;
        }

        Node* node = new Node(value);
        if (index == 0) {
            node->next = head_;
            head_ = node;
        } else {
            Node* previous = head_;
            for (std::size_t i = 0; i < index - 1; ++i) {
                previous = previous->next;
            }
            node->next = previous->next;
            previous->next = node;
        }

        ++size_;
        return true;
    }

    // 删除索引 index 处的节点；合法范围为 [0, size_ - 1]。
    bool eraseAt(std::size_t index) {
        if (index >= size_) {
            return false;
        }

        Node* target = nullptr;
        if (index == 0) {
            target = head_;
            head_ = head_->next;
        } else {
            Node* previous = head_;
            for (std::size_t i = 0; i < index - 1; ++i) {
                previous = previous->next;
            }
            target = previous->next;
            previous->next = target->next;
        }

        delete target;
        --size_;
        return true;
    }

    // 原地反转链表，时间复杂度 O(n)，额外空间复杂度 O(1)。
    void reverse() {
        Node* previous = nullptr;
        Node* current = head_;

        while (current != nullptr) {
            Node* next = current->next;
            current->next = previous;
            previous = current;
            current = next;
        }

        head_ = previous;
    }

    // 显式销毁所有节点；析构函数也会调用，因此可重复调用。
    void destroy() {
        while (head_ != nullptr) {
            Node* next = head_->next;
            delete head_;
            head_ = next;
        }
        size_ = 0;
    }

    void print() const {
        const Node* current = head_;
        while (current != nullptr) {
            std::cout << current->value << " -> ";
            current = current->next;
        }
        std::cout << "nullptr\n";
    }

    std::size_t size() const {
        return size_;
    }
};

int main() {
    SinglyLinkedList list;

    list.create({10, 20, 30});
    std::cout << "创建后: ";
    list.print();

    list.insertAt(1, 15);  // 在索引 1 插入 15
    std::cout << "插入 15 后: ";
    list.print();

    list.eraseAt(2);       // 删除索引 2 的节点（此处是 20）
    std::cout << "删除索引 2 后: ";
    list.print();

    list.reverse();
    std::cout << "反转后: ";
    list.print();

    list.destroy();
    std::cout << "销毁后: ";
    list.print();

    return 0;
}
