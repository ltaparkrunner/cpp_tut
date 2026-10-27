#include <iostream>
#include <utility>
#include <vector>

void process(int& i) { std::cout << "Lvalue\n"; }
void process(int&& i) { std::cout << "Rvalue\n"; }

template <typename T>
void wrapper(T&& arg) {
    // Without forward, 'arg' is a named variable, so it's always an lvalue.
    // With forward, we preserve the original value category.
    process(std::forward<T>(arg));
}
int main2();

int main() {
    int a = 0;
    wrapper(a);            // Prints "Lvalue" (T is int&)
    wrapper(std::move(a)); // Prints "Rvalue" (T is int)
    main2();
}

#include <iostream>
#include <memory>
#include <utility>
 
struct A
{
    A(int&& n) { std::cout << "rvalue overload, n=" << n << '\n'; }
    A(int& n)  { std::cout << "lvalue overload, n=" << n << '\n'; }
};
 
class B
{
public:
    template<class T1, class T2, class T3>
    B(T1&& t1, T2&& t2, T3&& t3) :
        a1_{std::forward<T1>(t1)},
        a2_{std::forward<T2>(t2)},
        a3_{std::forward<T3>(t3)}
    {}
 
private:
    A a1_, a2_, a3_;
};
 
template<class T, class U>
std::unique_ptr<T> make_unique1(U&& u)
{
    return std::unique_ptr<T>(new T(std::forward<U>(u)));
}
 
template<class T, class... U>
std::unique_ptr<T> make_unique2(U&&... u)
{
    return std::unique_ptr<T>(new T(std::forward<U>(u)...));
}
 
auto make_B(auto&&... args) // since C++20
{
    return B(std::forward<decltype(args)>(args)...);
}
 
int main2()
{
    auto p1 = make_unique1<A>(2); // rvalue
    int i = 1;
    auto p2 = make_unique1<A>(i); // lvalue
 
    std::cout << "B\n";
    auto t = make_unique2<B>(2, i, 3);
 
    std::cout << "make_B\n";
    [[maybe_unused]] B b = make_B(4, i, 5);

    return 0;
}
