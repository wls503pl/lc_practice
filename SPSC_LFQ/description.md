核心设计思路先说清楚：环形缓冲区（ring buffer），两个原子索引 head_（生产者写入位置）和 tail_（消费者读取位置）。关键规则是：head_ 只有生产者线程会写，tail_ 只有消费者线程会写——这正是 SPSC 能不用 CAS、只靠 load/store 就搞定的原因。

为什么需要 acquire/release，而不是随便用 relaxed？

问题的本质是：这两个索引不仅仅是"数字"，它们还在给对方发信号——"这块内存我已经写完了，你可以安全读了"。如果只用 relaxed，编译器和 CPU 有权对指令重排序，会出现这种灾难：

push 里，buffer_[head] = item 这行实际发生在 head_.store(...) 之后（被重排了）
消费者看到 head_ 已经更新，以为数据写好了，跑去读 buffer_[tail]，结果读到的是旧数据或者半写状态的数据

release（写 head_ 时用）的含义是："在我之前的所有写操作（这里是写 buffer_[head]），必须在这次 store 之前对其他线程可见"——相当于给之前的写操作加了个"发布"的屏障，不许重排到 store 后面去。

acquire（读对方的索引时用）的含义是："如果我看到了你 release 的这个值，那么你在 release 之前做的所有写操作，我都能看到"——保证不会有更早的读操作被重排到这次 load 前面，导致读到过期数据。

所以配对起来看：

push：先写 buffer_[head]，再用 release 更新 head_ → 相当于给"数据已就绪"这件事打了个时间戳并广播出去
pop：先用 acquire 读 head_（判断队列是否为空），一旦看到新值，就能保证紧接着读 buffer_[tail] 拿到的是完整数据 → 再用 release 更新 tail_，告诉生产者"这个槽位我读完腾空了"

为什么判断满/空时读自己的索引用 relaxed 就够：比如 push 里读 head_.load(relaxed)，因为 head_ 只有自己（生产者）会写，同一线程内本来就是顺序执行的，不存在跨线程可见性问题，不需要同步语义，用 relaxed 省一点开销即可。而读对方写的索引（tail_.load(acquire) 在 push 里，head_.load(acquire) 在 pop 里）就必须 acquire，因为那是跨线程的信号。

一句话总结记忆点：谁写就acquire读对方，自己读自己relaxed；数据写完了才release发布索引。

常追问的点:
为什么要 capacity + 1 而不是正好 capacity？—— 留一个空位来区分"满"和"空"，否则 head == tail 这个条件既可能表示空也可能表示满，无法区分。
为什么两个原子变量要 alignas(64) 分开放？—— 避免伪共享（false sharing）：如果 head_ 和 tail_ 挤在同一个 cache line，生产者写 head_ 会让消费者的 cache line 失效（反之亦然），即使两者逻辑上互不冲突，也会互相拖慢。分开对齐到独立 cache line 后二者互不干扰。