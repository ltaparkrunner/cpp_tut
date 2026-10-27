#include <memory>

template <typename T>
class Stack {
public:
    Stack() = default;
    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;

    ~Stack() {
        while (m_head)
            delete std::exchange(m_head, m_head->next);
    }

private:
    struct Node
    {
        std::unique_ptr<T> data;
        Node* next;
    };

    void Push(const T& data) {
        m_head = new Node{ std::make_unique<T>(data), m_head };
    }

    void Push(T&& data) {
        m_head = new Node{ std::make_unique<T>(std::move(data)), m_head};
    }

    std::unique_ptr<T> Pop() noexcept {
        std::unique_ptr<T> data;
        if (auto oldHead = m_head) {
            data = std::move(m_head->data);
            m_head = m_head->next;
            delete oldHead;
        }
        return data;
    }
    Node* m_head = nullptr;
};

int main() {
    return 0;
}

template <typename T>
class LockFreeStack {
private:
    struct Node {
        std::unique_ptr<T> data;
        Node* next = nullptr;

        Node(T&& d)
            : data(std::make_unique<T>(std::move(d))) {}
        Node(const T& d)
            : data(std::make_unique<T>(d)) {}
    };
    std::atomic<Node*> m_head = nullptr;
public:
    LockFreeStack() = default;
    LockFreeStack(const LockFreeStack&) = delete;
    LockFreeStack& operator=(const LockFreeStack&) = delete;
    // ~LockFreeStack() {
    //     for (Node* h = m_head.load(); h;)
    //         delete std::exchange(h, h->next);
    // }
    void Push(const T& data) {
        Push(new Node(data));
    }
    void Push(T&& data) {
        Push(new Node(std::move(data)));
    }
    std::unique_ptr<T> Pop() noexcept {
        ++m_threadsInPop;
        Node* oldHead = m_head.load();
        while (oldHead != nullptr
        && !m_head.compare_exchange_weak(oldHead, oldHead->next));
        auto data = oldHead ? std::move(oldHead->data) : std::unique_ptr<T>();
        TryReclaim(oldHead);
        return data;
    }
private:
    void Push(Node* newNode) noexcept {
        newNode->next = m_head.load();
        while (!m_head.compare_exchange_weak(newNode->next,
            newNode));
    }

    static void DeleteNodes(Node* nodes) noexcept
    {
        while(nodes)
            delete std::exchange(nodes, nodes->next);
    }

//    std::atomic<Node*> m_head = nullptr;
    std::atomic<unsigned> m_threadsInPop = 0;
    std::atomic<Node*> m_nodesToDelete = nullptr;
    void TryReclaim(Node* oldHead) noexcept {
        Node* nodesToDelete = m_nodesToDelete.exchange(nullptr);
        if(m_threadsInPop == 1) {// we are the only thread inside Pop func
            // Try to take the list of nodes away        
            if(--m_threadsInPop == 0) {
                DeleteNodes(nodesToDelete);
            }
            else if (nodesToDelete != nullptr) {
                ChainPendingNodes(nodesToDelete);
            }
            delete oldHead;
        }
        else{
            if(oldHead)
                ChainPendingNodes(nodesToDelete);
            --m_threadsInPop;
        }
    }
public:
    ~LockFreeStack() {
        DeleteNodes(m_nodesToDelete.load());
        DeleteNodes(m_head);
    }
private:
    void ChainPendingNodes(Node* const first) noexcept{
        assert(first);
        Node* last = first;
        while(Node* const next = last->next)
            last = next;
        ChainPendingNodes(first, last);
    }

    void ChainPendingNodes(Node* first, Node* last) noexcept
    {
        assert(first && last);
        last->next = m_nodesToDelete;
        while(!m_nodesToDelete.compare_exchange_weak(last->next, first))
        ;
    }

    void ChainPendingNode(Node* node) noexcept
    {
        assert(node);
        ChainPendingNodes(node, node);
    }
};
