auto WaitForFlag(bool & condition, std::mutex& m) {
    for(long long checkCount = 1;; ++checkCount) {
        std::lock_guard lk{ m };
        if (condition) return checkCount;
    }
}

int main() {
    bool datsIsReady = false;
    std::mutex dataMutex;
    std::string data;

    std::jthread consumer{ [&dataIsReady, &dataMutex, &data] {
        std::osyncstream(std::cout) << "Consumer is waiting for data\n";
        auto count = WaitForFlag(dataIsReady, dataMutex);
    }};
    std::jthread producer{ [&dataIsReady, &dataMutex, &data] {
        std::cout << "Producer is preparing data\n";

    }};
}