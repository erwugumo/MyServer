线程安全：
同步原语与互斥器
同步原语就是以下几个：
1临界区:通过对多线程的串行化来访问公共资源或一段代码，速度快，适合控制数据访问。  
  
2互斥量:为协调一起对一个共享资源的单独访问而设计的。  
  
3信号量:为控制一个具备有限数量用户资源而设计。  
  
4事 件:用来通知线程有一些事件已发生，从而启动后继任务的开始。

可重入：可重入函数主要用于多任务环境中，一个可重入的函数简单来说就是可以被中断的函数，也就是说，
可以在这个函数执行的任何时刻中断它，转入OS调度下去执行另外一段代码，而返回控制时不会出现什么错误；
而不可重入的函数由于使用了一些系统资源，比如全局变量区，中断向量表等，所以它如果被中断的话，可能会出现问题，
这类函数是不能运行在多任务环境下的。

一个线程安全的类：

```cpp
class Counter:boost::noncopyable
{
    public:
    Counter():value_(0){}
    int64_t value() const;
    //该函数为只读函数，不允许修改其中的数据成员的值。
    int64_t getAndIncrease();

    private:
    int64_t value_;
    mutable MutexLock mutex_;//每个counter对象有自己的mutex_，不同对象之间不构成锁争用
};

int64_t Counter::value() const
{
    MutexLockGuard lock(mutex_);//lock的析构会晚于返回对象的构造，保护共享数据value_
    return value_;
}

int64_t Counter::getAndIncrease()
{
    MutexLockGuard lock(mutex_);
    int64_t ret=value++;//两个线程可以一起执行这条，它们对不同的类进行操作
    return ret;
}
```
作者：no.body
链接：https://www.zhihu.com/question/19801131/answer/27459821
来源：知乎
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。

什么是回调函数？我们绕点远路来回答这个问题。编程分为两类：系统编程（system programming）和应用编程（application programming）。所谓系统编程，简单来说，就是编写库；而应用编程就是利用写好的各种库来编写具某种功用的程序，也就是应用。系统程序员会给自己写的库留下一些接口，即API（application programming interface，应用编程接口），以供应用程序员使用。所以在抽象层的图示里，库位于应用的底下。当程序跑起来时，一般情况下，应用程序（application program）会时常通过API调用库里所预先备好的函数。但是有些库函数（library function）却要求应用先传给它一个函数，好在合适的时候调用，以完成目标任务。这个被传入的、后又被调用的函数就称为回调函数（callback function）。打个比方，有一家旅馆提供叫醒服务，但是要求旅客自己决定叫醒的方法。可以是打客房电话，也可以是派服务员去敲门，睡得死怕耽误事的，还可以要求往自己头上浇盆水。这里，“叫醒”这个行为是旅馆提供的，相当于库函数，但是叫醒的方式是由旅客决定并告诉旅馆的，也就是回调函数。而旅客告诉旅馆怎么叫醒自己的动作，也就是把回调函数传入库函数的动作，称为登记回调函数（to register a callback function）。如下图所示（图片来源：维基百科）：<img src="https://pic4.zhimg.com/50/0ef3106510e2e1630eb49744362999f8_hd.jpg?source=1940ef5c" data-rawwidth="625" data-rawheight="233" class="origin_image zh-lightbox-thumb" width="625" data-original="https://pic4.zhimg.com/0ef3106510e2e1630eb49744362999f8_r.jpg?source=1940ef5c"/>可以看到，回调函数通常和应用处于同一抽象层（因为传入什么样的回调函数是在应用级别决定的）。而回调就成了一个高层调用底层，底层再回过头来调用高层的过程。（我认为）这应该是回调最早的应用之处，也是其得名如此的原因。回调机制的优势从上面的例子可以看出，回调机制提供了非常大的灵活性。请注意，从现在开始，我们把图中的库函数改称为中间函数了，这是因为回调并不仅仅用在应用和库之间。任何时候，只要想获得类似于上面情况的灵活性，都可以利用回调。这种灵活性是怎么实现的呢？乍看起来，回调似乎只是函数间的调用，但仔细一琢磨，可以发现两者之间的一个关键的不同：在回调中，我们利用某种方式，把回调函数像参数一样传入中间函数。可以这么理解，在传入一个回调函数之前，中间函数是不完整的。换句话说，程序可以在运行时，通过登记不同的回调函数，来决定、改变中间函数的行为。这就比简单的函数调用要灵活太多了。

延迟式回调，最典型的例子是createThread(threadFuntion),这里threadFuntion是callback函数，createThread是中间函数。如果起始函数想等待线程完毕，就是用join函数。

比如排序，排序算法的人可以写好了算法，但是compare由callback提供。这个是同步回调。但是sort算法和compare解耦了。增加任何不同的compare算法，sort都不用改变。

A "callback" is any function that is called by another function which takes the first function as a parameter. （在一个函数中调用另外一个函数就是callback）

对象构造要做到线程安全，只需要在构造期间不要泄露this指针：
不要在构造函数中注册任何回调
不要在构造函数中把this传递给跨线程的对象
即便是在构造函数的最后一行也不行->因为这个类可能是一个基类，执行完构造函数接着执行派生类的构造函数

当一个类正在构造时在构造函数中将this泄露给了其它对象，这在单线程串行执行情况下可能没有什么问题，但是在多线程下那么问题就比较大了。比如线程1负责构造这个对象A但是在构造函数中将this指针泄露给了其它线程所调用的对象B，不巧的是其它线程所调用的对象B看见A有些不爽将其析构了。那么最后A自以为一切构造好了返回，线程1然后对这个A操作，最后可怕的错误(比如段错误)无穷无尽的折磨线程1......

构造函数加initialize有时可以解决在构造函数中需要调用其他函数的问题。
靠判断initialize函数的返回值判断是否出问题。

单线程的对象析构不难，最多需要注意避免空悬指针和野指针。
空悬指针：指向已经销毁的对象或已经回收的地址
野指针：未初始化的指针
多线程中有许多竞态条件。用mutex不再能保证线程安全，因为析构函数会销毁mutex。
eg
```cpp
Foo::~Foo()
{
    MutexLockGuard lock(mutex_);
    //(1)
}
void Foo::update()
{
    MutexLockGuard lock(mutex_);//(2)
    //...
}
// thread A
delete x;
//delete 释放new分配的单个对象指针指向的内存 
//delete[] 释放new分配的对象数组指针指向的内存
x=NULL;

// thread B
if(x){
    x->update();
}
```
当线程A执行到(1)处，线程B刚执行(2)时，A持有mutex，B等待A释放mutex；然而A接下来销毁mutex，之后就不知道了。
因此靠NULL防止二次释放没有用。作为数据成员的mutex不能保护析构。

一个函数同时读写一个class的两个对象，也可能死锁：
swap(a,b)
先得到a的锁，请求b的锁
swap(b,a)
先得到b的锁，请求a的锁
俩swap同时执行，死锁
需要按相同顺序加锁，比较mutex对象的地址，始终先获取较小地址的mutex。

没有高效的方法来判断C++中一个动态的对象是否还活着：光看指针看不出来，指针指向一块内存，若对象已经销毁，那么根本不能访问，访问不了就判断不了。这是C++指针问题的根源。Java没有。

1、三种关系
1.1 Composition 复合 （has a）
1.1.1 定义
表示“has a ”，即“拥有”关系。一个class里面，有另一种class的东西。即我这种类包含有另外一种类的对象。

1.1.2 好处
所有的功能不用自己写，直接调用他拥有类的功能就好。如下例queue中使用deque的对象，即为复合关系，就可直接调用deque的成员函数。
```cpp
template <class T, class Sequence = deque<T> >
class queue
{
protected:
	Sequence c;		//底层容器
public：
	//以下完全利用c的操作函数完成
	bool empty() const { return c.empty(); }
	size_type size() const { return c.size(); }
	reference front() { return c.front(); }
	reference back() { return c.back(); }
	//deque是两端可进出，queue是末端进前端出
	void push(const value_type& x) { c.push_back(x); }
	void pop() { c.pop_front(); }
};
```
1.1.3 引入一种设计模式：Adapter
一种的功能可以完全满足另一种需要的功能，如deque能完全满足queue。则只需改一些deque的接口即可。
构造由内而外：要构造外面的东西，需要先把内部的东西构造完成。即调用内部的默认（default）构造函数，然后才执行自己。
析构由外而内：要进行析构，先把外面的东西剥离掉，即先执行自己，然后才调用内部的析构函数。
1.2 Delegation 委托 （Composition by reference）
（虽然是用指针在传，但术语都叫做by reference）

1.2.1 定义
不同于上面复合的“拥有”，是实实在在的包含，委托是一种用指针来指向另一个类的方式。在任何我想用你来做事情的时候，就可以用指针调用你，把任务委托给你。所以委托可以看做是一种比较虚的拥有，因此在图中用虚的菱形。
1.2.2 与Composition的不同之处
Composition是有了外面就要有里面。（同步）
Delegation是有了外面不一定有里面，当外面调用里面时，才会创建里面的内容。（不同步）

1.2.3 不可以牵一发而动全身
当a要对hello进行改变时，b和c不能改变。因此应当在a改变之前进行复制，给a一个副本让a进行改动，b和c不变。这就是copy on write（在写的时候进行复制）。

1.3 Inheritance 继承（is a）
1.3.1 定义
写法：要定义一个类时，用冒号，后面加上继承方式，最后加上要继承的类。
语法：从子类往父类画，一个空心的三角形。（子类的东西比父类多）
1.3.2 继承方式
public：使用public继承，传达出一种逻辑：是一种，即“is a”。这种继承方式是最重要的，后面两种其他语言不一定有，都默认public继承。
还要其余两种继承方式：private和protected，这里请先查阅相关书籍。

1.3.3 内存
子类的对象中要有父类的成分（part），所以这也是一种外部包含内部的状态，如Composition 。
因此构造由内而外：要构造外面的东西，需要先把内部的东西构造完成。即调用Based的默认（default）构造函数，然后才执行Derived的。析构由外而内：要进行析构，先把外面的东西剥离掉，即先执行Derived的，然后才调用Based的析构函数。
注：父类Based的dtor必须是virtual的！

1.3.4 好处——virtual
子类除了拥有自己的数据和方法以外，还可以拥有父类的。但最有价值的是和虚函数搭配的情况。
继承的过程：数据可以被继承下来，函数也可以被继承下来（继承的是调用权），根据子类是否重新定义决定是否virtual。



