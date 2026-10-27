#include <thread>
#include <atomic>
#include <stdexcept>
#include <functional>

struct HazardPointer
{
    std::atomic<std::thread::id> id;
    std::atomic<void*> pointer;
    /* data */
};

constexpr unsigned MaxHazardPointers = 100;
HazardPointer HazardPointers[MaxHazardPointers];

class HpOwner {
public:
    HpOwner(const HpOwner&) = delete;
    HpOwner& operator=(const HpOwner&) = delete;
    HpOwner();

    std::atomic<void*>& GetPointer() noexcept { return m_hp->pointer; }
    ~HpOwner() {
        // make room in hazard pointers' array
        m_hp->pointer.store(nullptr);
        m_hp->id.store(std::thread::id());
    }
private:
    HazardPointer* m_hp;
};
HpOwner::HpOwner()
    : m_hp(nullptr) {
    const auto thisThreadId = std::this_thread::get_id();

    for (auto& hp : HazardPointers) {
        std::thread::id nullId; // Id doesn't match with any thread
        if (hp.id.compare_exchange_strong(nullId, thisThreadId)) {
            // An empmy slot is found, save addr to hazard pointer
            m_hp = &hp;
            break;
        }
    }

    if(!m_hp)
        // Too many threads use hasard pointers
        throw std::runtime_error("No hazard pointers available.");
}

std::atomic<void*>& GetHazardPointerForCurrentThread() {
    // In the first invocation this func for a thread it's HazardPointer will be created
    thread_local static HpOwner hazard;
    // In the next invokation a hashed pointer meaning will be return
    return hazard.GetPointer();     
}

bool OutstandingHazardPointersFor(void* p) noexcept {
    for(auto& hp : HazardPointers) {
        if(hp.pointer.load() == p) {
            return true;
        }
    }
    return false;
}

struct DataToReclaim {
    void* data;
    std::function<void(void*)> deleter;
    DataToReclaim* next = nullptr;

    // It saves data, that should be deleted
    template <typename T>
    DataToReclaim(T* p)
        : data(p)
        , deleter([](void* p) { delete static_cast<T*>(p);})
        {}
    ~DataToReclaim() {
        // The destructor is expected to not throw exceptions.
        deleter(data);
    }
};

// Pointer to the list of notes to be deferred deletion
std::atomic<DataToReclaim*> NodesToReclaim = nullptr;

// We add a node for future relese.
void AddToReclaimList(DataToReclaim* node) noexcept {
    node->next = NodesToReclaim.load();
    while (!NodesToReclaim.compare_exchange_weak(node->next, node))
    ;
}

template <typename T>
void ReclaimLaiter(T* data) {
    AddToReclaimList(new DataToReclaim(data));
}

void DeleteNodesWithNoHazards() noexcept {
    // Try to take a list of nodes away
    auto current = NodesToReclaim.exchange(nullptr);

    while(current) {
        DataToReclaim* const next = current->next;

        if(!OutstandingHazardPointersFor(current->data)) {
            // we delete note along with its data
            delete current;
        }
        else {
            // We add current node to the list for future deletion.
            AddToReclaimList(current);
        }
        current = next;
    }

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
        Node* oldHead = TryExtractHead();
        return TryPopDataFromHead(oldHead);
    }
private:
    Node* TryExtractHead();
    void TryExtractHead();
    static std::unique_ptr<T> TryPopDataFromHead(Node* oldHead);
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
        // DeleteNodes(m_nodesToDelete.load());
        // DeleteNodes(m_head);
        DeleteNodesWithNoHazards();
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

template <typename T>
LockFreeStack<T>::Node *LockFreeStack<T>::TryExtractHead(){
    std::atomic<void*>& hp = GetHazardPointerForCurrentThread();
    Node* oldHead = m_head.load();
    do{
        Node* temp;
        do{ // Set Hazard pointer to head.
            temp = oldHead;
            hp.store(oldHead);
            // can be UB ??
            oldHead = m_head.load();
        } while (oldHead != temp);
    } while(oldHead && !m_head.compare_exchange_strong(oldHead, oldHead->next));
    hp.store(nullptr);
}

template <typename T>
static std::unique_ptr<T> LockFreeStack<T>::TryPopDataFromHead(LockFreeStack<T>::Node* oldHead){
    std::unique_ptr<T> data;
    if(oldHead) {
        data = std::move(oldHead->data);
        if(OutstandingHazardPointersFor(oldHead)) {
            ReclaimLater(oldHead);
        }
        else {
            delete oldHead;
        }
        DeleteNodesWithNoHazards();
    }
    return data;
}

int main() {
    return 0;
}