#include "Defines/BaseDefs/BaseObject_copy.hpp"
#include "Defines/ConstantDefs/Marcos.h"

#include <vector>
#include <iostream>
USING_NS;
class test_arr : public BaseObject
{
private:
    std::vector<int> m_vec;
private:
    test_arr() {std::cout << "test_arr ctor, 地址: " << this << std::endl;}
public:
    static test_arr* create()
    {
        test_arr* pRet = new test_arr();
        return pRet;
    }
    void print_all()
    {
        std::cout << "print_all begin" << std::endl;
        for (auto it : m_vec)
        {
            std::cout << it << std::endl;
        }
        std::cout << "print_all end" << std::endl;
    }
    void emplace_back(int val)
    {
        m_vec.emplace_back(val);
    }

};



int main()
{
    test_arr* parr = test_arr::create();
    std::cout << "ref count = " << parr->getRefCount() << std::endl;
    parr->emplace_back(1);
    parr->emplace_back(2);
    parr->emplace_back(3);
    parr->print_all();
    std::cout << "ref count = " << parr->getRefCount() << std::endl;
    parr->release();
    std::cout << "ref count = " << parr->getRefCount() << std::endl;
    return 0;

}