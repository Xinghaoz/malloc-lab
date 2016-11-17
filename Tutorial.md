## 由implicit list变成explicit list:
由implicit list变成explicit list，就是利用一个*双向*链表来存储free block。
难点在于，这个链表不能用现成的数据结构，甚至要直接上heap上写next与previous指针

变成链表之后要考虑的事情：
1. 什么时候要insert进链表里面，在哪个方法insert
    要insert进链表，那只有三种情况：
    第一种是free一个block
    第二种是extend heap
    第三种比较容易被忽略，在malloc的时候我们也可能要insert，为什么呢？比如说malloc一个16byte的大小，
    find_fit找到了一个64bit的大小，那么还剩(64-16-16(header)) = 32 byte的free block，这个free
    block也是要insert进链表里的
    对于前两种情况，其实我们可以把insert统一写进coalesce里面，因为无论是free和extend_heap，最后都要
    调用coalesce这个函数。在最共同的地方修改有利于我们后续的debug以及修改

2. 什么时候要delete，在哪个方法delete
    我选择不在malloc里面delete，而是


1. heap_listp 改成 free_listp

要改的方法，以及内容：
+ malloc部分
与malloc有关的函数有malloc, find_fit, place

1. malloc这个方法基本上没有要改的地方

2. find_fit要改的
