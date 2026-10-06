template<typename T>
class shared_ptr
{
public:
    typedef T element_type;                     // 内部类型定义
    shared_ptr();                               // constructor
    template<typename Y> explicit shared_ptr(Y *p);
    template<typename Y, typename D> shared_ptr(Y *p, D d);
    ~shared_ptr();                              // destructor
    shared_ptr(const shared_ptr& r);            // copy constructor
    shared_ptr &operator=(const shared_ptr& r); // assignment operator
    template<typename Y> shared_ptr& operator=(const shared_ptr<Y>& r);
    void reset();                               // reset smart pointer
    template<typename Y> void reset(Y *p);
    template<typename Y, typename D> void reset(Y *p, D d);

    T& operator*() const;                       // * operator overloading
    T* operator->() const;                      // -> operator overloading
    T* get() const;                             // Obtain the raw pointer

    bool unique() const;                        // 强引用计数是否等于1, 等价于 p.use_count() == 1，在 C++17 被标记为 deprecated,C++20 被移除
    // 因为它在多线程下不可靠。判断完 unique() 之后,别的线程可能立刻拷贝出新的 shared_ptr,所以不能用它来做"我独占了,可以放心修改"的线程安全判断
    long use_count() const;                     // reference count

    explicit operator bool() const;             // explicit bool value transfer,让智能指针可以直接当"是否非空"来判断,等价于 get() != nullptr。
    // 如果去掉 explicit,shared_ptr 会先隐式转成 bool,再提升成 int,带来一堆离谱却能通过编译的写法
    // if (p) 只判断是否非空，if (p.get()) 取出裸指针再判断,效果相同但多暴露了裸指针
    void swap(shared_ptr &b);                   // 交换指针，交换两个 shared_ptr 各自管理的对象(以及各自的控制块指针),不改变引用计数,
    // 也不会拷贝或销毁对象。只是交换内部的两个指针,所以很快,且不抛异常。
};

