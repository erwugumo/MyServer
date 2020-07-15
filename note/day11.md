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

销毁很难，那么就使用对象池，不销毁

静态成员：
在类定义中，它的成员（包括数据成员和成员函数）可以用关键字static申明为静态成员。静态成员的特性是不管这个类创建了多少个对象，它其中的静态成员只有一个拷贝，这个拷贝被所有属于这个类的对象共享。

从原理来分析，我们能清楚的了解，类的数据成员和成员函数都是跟着类的执行，在编译器上为他在堆栈上分配内存空间存储的。静态数据成员和静态成员函数和他们不同，它们是在程序开始运行时候存储在静态存储空间的。

静态数据成员属于类，而不像普通的数据成员那样属于某个对象，因此我们可以用“类名::”这样的形式访问静态数据成员。如：Student::count。
静态数据成员不能在类中进行初始化，因为类中不给他分配内存空间（前面说的类中存储地址是堆栈，静态数据成员存储地址是静态存储空间），所以必须在其他地方为他提供定义和初始化。默认时，静态成员被初始化为0。
静态数据成员和静态变量一样，是在编译时创建并进行初始化的。它在该类的任何对象创建之前就已经存在。因此，公有的静态数据成员可以在对象定义之前就被访问。对象定以后，公有的静态数据成员也可以通过对象进行访问，格式如下：
对象名.静态数据成员名；

对象名->静态数据成员名；
静态数据成员也遵循public，protect，private的原则。所以，私有的静态数据成员不能被类外部函数访问，也不能用对象进行访问。
静态数据成员的类型可以是该成员所属的类类型。非静态数据成员被限定为申明其自身类的对象的指针或引用。

```cpp
 class A{
 private:
     static A a1;    // OK
     A *a1;           // OK
     A a3;             // Error
 };
```
静态数据成员可以用作默认实参，而普通数据成员不可以。　　
```cpp
class Example{
public:
    static int a;
    int b;

    void fun1(int i = a); // 正确，a为静态数据成员
    void fun2(int i = b); // 错误，b为普通数据成员
};
```
静态成员函数
在类定义中，在成员函数之前加上static申明成为静态成员函数。和静态数据成员一样，静态成员函数属于整个类，是该类的所有对象共享的成员函数，而不是属于某个对象特有的函数。定义静态成员函数的格式如下：

Static 返回类型 函数名(实参表)

与静态数据成员一样，它也遵循public，protect和private的原则。调用公有的静态成员函数格式如下：

类名::静态成员函数(实参表)

对象名.静态成员函数(实参表)

对象名->静态成员函数(实参表)

静态成员函数可以定义成内嵌的，也可以在内外定义，在类外定义是，前面不需要加static。
一般情况下，静态成员函数主要是用来访问全局变量或者同一个类中的静态数据成员。
私有静态成员函数不能被类的外部函数和对象访问。
使用静态成员函数的一个原因就是可以用它在建立任何对象之前处理静态数据成员这是普通成员函数不能实现的。
编译系统将静态成员函数限定为内部连接，也就是说，与现行的文件相连接的文件中的同名函数不会与该函数发生冲突，维护了该函数的安全性，这是使用静态成员函数的另外一个原因。
在一般的成员函数中都隐藏一个this指针，用来指向对象自身，而在静态成员函数中没有这个this指针，因为它不与特定的对象相关联。调用静态成员函数使用如下格式：类名::静态成员函数名()；
静态成员函数不能被申明为const，因为static成员不是任何对象的组成部分，毕竟，将成员函数申明为const就是承诺不会修改该函数所属的对象。
静态成员函数不能被申明为虚函数。
一般而言，静态成员函数不可访问类中的非静态成员。如果确实需要，静态成员函数只能通过对象名（或指向对象名的指针）访问该对象的非静态成员。如：
```cpp
static void display(small_cat &w)
2 {
3     printf("The small cat weight %d pounds\n", w.weight);
4 }
```
Observer模式：
```cpp
class Observer
{
    public:
    virtual ~Observer();
    virtual void update()=0;
    /*纯虚函数是在声明虚函数时被“初始化”为0的函数。声明纯虚函数的一般形式是 virtual 函数类型 函数名 (参数表列) =0;

    注意: ①纯虚函数没有函数体；

      ②最后面的“=0”并不表示函数返回值为0，它只起形式上的作用，告诉编译系统“这是纯虚函数”; 

      ③这是一个声明语句，最后应有分号。
    */
    //...
}
class Observable
{
    public:
    void register_(Observer* x);
    void unregister(Observer* x);

    void notifyObservers()
    {
        for(Observer* x: observers_)
        {
            x->update();
        }
    }

    private:
    std::vector<Observer*> observers_;
    //vector中push_back对象时，会调用对象的拷贝构造函数。而且在vector空间不足时，继续push_back，vector会将之前的所有对象都拷贝构造到一块更大的空间里。也就是说对象如果较大，那么最好用vector保存指针以减少调用拷贝构造 造成的消耗，如果vector存指针，那么也就拷贝指针而已，消耗非常小。如果实在需要使用vector保存对象，那么尽量利用c++11提供的emplace_back代替push_back。
}
```
当一个observable对象去通知每个observer对象时，要调用notifyObservers函数，需要访问保存在observers_中的所有observer对象，但observable对象不能得知observer_中的所有对象都没被销毁。那么我在new每个observer的时候都注册一次可以么？然后析构的时候调用unregister解注册？
```cpp
class Observer
{
    public:
    void observe(Observable* s)
    {
        s->register_(this);
        subject_=s;
    }
    virtual ~Observer()
    {
        subject_->unregister(this);
    }
    Observable* subject_;
    //...
}
```
不行，Observer在析构的时候无法保证subject_还活着。
即使subject_永生，那也不行。
比如线程A执行对Observer的析构，没走到unregister这一步，那么subject_里面仍然后这个Observer，此时线程B开始对Observer执行update，那么出现错误。
我们迫切需要一种方法，让我们知道指针指向的对象是否被销毁。
想安全的销毁对象，需要保证所有人都看不到用不到这个对象了，才能销毁。否则比如p1、p2指向对象O，通过p1销毁O，p2变成空悬指针，报错。
使用引用计数。
万能的解决方案：引入另外一层间接性，用对象管理共享资源。计数型引用指针。
shared_ptr是引用计数型智能指针，在Boost和std::tr1里提供。当引用计数降为0时，对象（资源）即被销毁。weak_ptr也是引用计数型智能指针，但它不增加对象的引用次数，是弱引用。
shared_ptr控制对象的生命期，当指向对象x的最后一个shared_ptr析构或reset的时候，x保证被销毁
weak_ptr不控制对象的生命期，但它知道对象是否还活着。如果对象还活着，那么它可以提升为shared_ptr，死了会失败。

引用计数增加的情况

拷贝一个shared_ptr，其所指对象的引用计数会递增，如：

用一个shared_ptr初始化另一个shared_ptr
用一个shared_ptr给另一个shared_ptr赋值
将shared_ptr作为参数传递给一个函数
shared_ptr作为函数的返回值
 

引用计数减少的情况

给shared_ptr赋予一个新值
shared_ptr被销毁（如离开作用域）

局部的shared_ptr离开其作用域，它所指对象的引用计数会递减(-1)

假设：没有全局的shared_ptr，那么正确的结果应该是该shared_ptr所指的对象被销毁

我之前错误的想法：多个局部shared_ptr共同指向同一个对象，那么该对象的引用计数就>1，该函数结束时对象的引用计数减1（但仍>0），那么该对象不应该被销毁。

纠正想法：既然是多个局部shared_ptr指向该对象，那么函数结束时对象的引用计数就不应该只减1啊！！

```cpp
shared_ptr<int> init()
{
    shared_ptr<int> sp2 = make_shared<int>(3);
    shared_ptr<int> sp3(sp2);
    cout << sp2.use_count() << endl;        //输出2
    return sp2;                             //返回sp2，故引用计数递增，变为3
}                                           //sp2和sp3离开作用域，引用计数减2，变为1
 
int main()
{
    auto p = init();                        //此处赋值的拷贝与return处的拷贝是一致的
    cout << p.use_count() << endl;          //输出1
    return 0;
}
```

将shared_ptr应用到Observer上：
```cpp
class Observerable
{
    public:
    void register_(weak_ptr<Observer> x);
    void notifyObservers();

    private:
    mutable MutexLock mutex_;
    std::vector<weak_ptr<Observer> > observers;
    typedef std::vector<weak_ptr<Observer> >::iterator Iterator;
}
class Observable::notifyObservers()
{
    MutexLockGuard lock(mutex_);
    Iterator it = observers_.begin();
    while(it!=obserbers_.end())
    {
        shared_ptr<Observer> obj(it->lock())；//尝试提升
        if(obj)
        {
            //访问observer需要通过weak_ptr
            //销毁observer需要通过weak_ptr
            //weak_ptr提升为shared_ptr
            //无法被销毁
            obj->update();
            ++it;
        }
        else
        {
            it=observers_erase(it);
        }
    }
}
```
虽然解决了问题，但要求Observer类的所有对象必须用shared_ptr来管理。
注意，shared_ptr能够对所管理的对象实现防止读取已销毁对象的错误，但对管理的对象的读写也要加锁。
shared_ptr管理的资源是在析构时线程安全的，但它本身不是。
对shared_ptr的本身的读写操作都要加锁。
```cpp
MutexLock mutex;
shared_ptr<Foo> globalPtr;
//形参是const shared_ptr<Foo>类型的引用，是一个常量指针
void doit(const shared_ptr<Foo>& pFoo);
void read()
{
    shared_ptr<Foo> localPtr;
    {
        MutexLockGuard lock(mutex);
        localPtr=globalPtr;
    }
    doit(localPtr);
}

void write()
{
    shared_ptr<Foo> newPtr(new Foo);
    {
        MutexLockGuard lock(mutex);
        globalPtr=newPtr;
    }
    doit(newPtr);
}
```
令类的this为shared_ptr类型：继承enable_shared_from_this类即可：
```cpp
class StockFactory : public boost::enable_shared_from_this<StockFactory>,boost::noncopyable;
```
为了使用enable_shared_from_this，StockFactory必须是堆上对象，且由shared_ptr管理其生命周期。
C++关于堆上与栈上分配对象

```cpp
class ConfigFile
{
  public:
    ConfigFile();
    const std::string& value(const std::string& section, const std::string& entry);
  protected:
    std::map _content;
};
```
在上述类中
ConfigFile cf();为在栈上分配对象。
ConfigFile cf = new ConfigFile();为在堆上分配对象。
  在C++中，对象是在堆栈中创建的。这样可达到更快的速度，因为当我们进入一个特定的作用域时，堆栈指针会向下移动一个单位，为那个作用域内创建的、以堆栈为基础的所有对象分配存储空间。而当我们离开作用域的时候（调用完毕所有局部构建器后），堆栈指针会向上移动一个单位。然而，在C++里创建“内存堆”（Heap）对象通常会慢得多，因为它建立在C的内存堆基础上。这种内存堆实际是一个大的内存池，要求必须进行再循环（再生）。在C++里调用delete以后，释放的内存会在堆里留下一个洞，所以再调用new的时候，存储分配机制必须进行某种形式的搜索，使对象的存储与堆内任何现成的洞相配，否则就会很快用光堆的存储空间。之所以内存堆的分配会在C++里对性能造成如此重大的性能影响，对可用内存的搜索正是一个重要的原因。所以创建基于堆栈的对象要快得多。
  在这个函数return之后，在堆上面的对象还能够被安全的使用，而在栈上创建的对象变得不可靠。
也就是说，在堆上创建的对象，只要你不用来delete来释放，任何时候你用指向这个对象的指针都可以访问这个对象，也就是说指针指向的内存在手动释放(使用delete)前不会再被分配掉。
  而且栈是有大小限制的，大概为2.3M，当你分配的对象大小大于2.3M时就只能在堆上分配对象了。

```cpp
class StockFactory:public boost::enable_shared_from_this<StockFactory>,boost::noncopyable
{
    public:
    shared_ptr<Stock> get(const string& key)
    {
        shared_ptr<Stock> pStock;
        MutexLockGuard lock(mutex_);
        weak_ptr<Stock>& wkStock=stock_[key];
        pStock=wkStock.lock();
        //升级失败说明key对应的stock不存在
        if(!pStock)
        {
            //智能指针包含了 reset() 方法，如果不传递参数（或者传递 NULL），则智能指针会释放当前管理的内存。如果传递一个对象，则智能指针会释放当前对象，来管理新传入的对象。
            pStock.reset(
                new Stock(key),boost::bind(&StockFactory::weakDeleteCallback,
                boost::weak_ptr<StockFactory>(shared_from_this()),
                _1)
            );
            //bind表示将weak_ptr绑到weakDeleteCallback上
            //即将pStock指向的内存中的数据析构，之后调用weakDeleteCallback，该函数的参数是weak_ptr，将shared_ptr强制转换为weak_ptr。shared_ptr是shared_from_this，也就是StockFactor类的shared_ptr版this函数。
            wkStock=pStock;
        }
        return pStock;
    }

    private:
    //这里使用weak而不是shared，是为了防止StockFactory类的生命意外延长。记住这里的StockFactory保存的是Stock的指针，Factory自己销毁了但是对象不一定销毁。
    static void weakDeleteCallback(
        const boost::weak_ptr<StockFactory>& wkFactory,
        Stock* stock
    )
    {
        shared_ptr<StockFactory> factory(wkFactory.lock());
        if(factory)//如果还活着
        {
            factory->removeStock(stock);
        }
        delete stock;
    }

    void removeStock(Stock* stock)
    {
        if(stock)
        {
            MutexLockGuard lock(mutex_);
            stocks_erase(stock->key);
        }
    }

    private:
    mutable MutexLock mutex_;
    std::map<string,weak_ptr<Stock> >stocks_;

}
```