namespace detail {
    template <typename It, typename T>
    struct AccumulateBlock
    {
        void operator()(It begin, It end, T& result)
        {
            result = std::accumulate(begin, end, result);
        }
        /* data */
    };
    
}