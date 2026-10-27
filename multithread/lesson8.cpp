#include <iostream>


class Data{
public: 
    Data() {
        std::cout << "Data::Data()" << std::endl;
    }
    void UseData() const {
        std::cout << "Data::UseData()" << std::endl;
    }
};
class Widget {
public:
    Widget() = default;
    Widget(const Widget&) = delete;
    Widget& operator = (const Widget&) = delete;
    ~Widget() {
        delete m_data;
    }
    void Foo() const {
        std::cout << "Widget::Foo()" << std::endl;
        if (!m_data)
            m_data = new Data();
        m_data->UseData();
    }
private:
    mutable const Data* m_data = nullptr;
};

int main() {
    Widget widget;
    widget.Foo();
    widget.Foo();
}