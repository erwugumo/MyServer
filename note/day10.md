类模版std::function是一种通用、多态的函数封装。std::function的实例可以对任何可以调用的目标实体进行存储、复制、和调用操作，
这些目标实体包括普通函数、Lambda表达式、函数指针、以及其它函数对象等。
std::function对象是对C++中现有的可调用实体的一种类型安全的包裹（我们知道像函数指针这类可调用实体，是类型不安全的）。

通常std::function是一个函数对象类，它包装其它任意的函数对象，被包装的函数对象具有类型为T1, …,TN的N个参数，
并且返回一个可转换到R类型的值。std::function使用 模板转换构造函数接收被包装的函数对象；
特别是，闭包类型可以隐式地转换为std::function。

临时变量通常在函数参数传递发生类型转换以及函数返回值时被创建。比如下面这个例子：
void uppercasify(const string& str)
{}

int main(int argc, char* argv[])
{
 char subtleBookPlug[] = "Effective C++";
 uppercasify(subtleBookPlug);  // 此处有类型转换
 return 1;
}
函数uppercasify需要const string&类型的参数，而实参类型为char *，故编译器会尝试着进行类型转换。
此时一个string类型的临时变量将被创建，并用subtleBookPlug来初始化对象，最后将临时变量传给函数uppercasify。
理解了这个例子，也就能较好的理解为什么C/C++不允许为非const的引用类型创建临时变量了。比如下面这个例子：
void uppercasify(string& str)  // 参数类型改为string &
{}

int main(int argc, char* argv[])
{
 char subtleBookPlug[] = "Effective C++";
 uppercasify(subtleBookPlug);
 return 1;
}
此时，如果创建了一个临时变量，那函数所修改的对象为临时变量，而不是用户所期待的subtleBookPlug了，从而容易引起误操作。
产生临时变量的三种情况：
一：以By Value的方式传值；
二：参数为const的类型。
三：类型转换

一：以By Value的方式传值。
        我们都知道，引用类型和指针类型传递的都是地址，可以直接对地址中存放的数据进行操作，而以传值的方式传递参数，
        就会在heap中重新分配一个临时区域，将实参中的数据拷贝到临时区域中，而你对这份数据进行的任何的操作都不会影响实参的内容，
        因为实参跟形参只是内容相同，分别在两块不同的内存中。而引用和指针操作的是同一块内存，所以形参修改后，实参也修改了。
二：参数为const的类型。
        因为常量是不能修改，在只需要实参中的数据，而不需对实参进行修改时，或是禁止对实参进行修改时，把形参定义为const类型，
        系统会产生一个临时变量，就能起到保护数据的作用，如在函数strlen中，修改参数的值行吗？本来只是想得到实参的长度，
        结果在函数中被修改了，那得到得实参长度还是真实的吗。
        如果你程序中的数据到处都可以被修改，那是多么的可怕（所以我们讨厌全局变量），所以const还是有它存在的价值。
三：类型转换的时候会产生临时变量。
        真是糟糕啊，在用类型转换带来便利的同时，产生临时变量就是我们承担的损失。
        如将一个short类型转换成int类型，他们占用的内存不一样，如果不产生临时变量，那不就short类型和int类型占用的字节数不就一样了吗，
        sizeof不就坑爹了吗。
 
C++语言禁止为非常量引用产生临时对象。同时证明引用类型传参不会产生临时变量，如char[]转换成string会报错，他们都是引用类型。
临时变量不能作为非const引用参数，不是因为他是常量，而是因为c++编译器的一个关于语义的限制。如果一个参数是以非const引用传入，
c++编译器就有理由认为程序员会在函数中修改这个值，并且这个被修改的引用在函数返回后要发挥作用。
但如果你把一个临时变量当作非const引用参数传进来，由于临时变量的特殊性，程序员并不能操作临时变量，而且临时变量随时可能被释放掉，
所以，一般说来，修改一个临时变量是毫无意义的，据此，c++编译器加入了临时变量不能作为非const引用的这个语义限制，
意在限制这个非常规用法的潜在错误。


什么是类型安全？
类型安全很大程度上可以等价于内存安全，类型安全的代码不会试图访问自己没被授权的内存区域。
“类型安全”常被用来形容编程语言，其根据在于该门编程语言是否提供保障类型安全的机制；
有的时候也用“类型安全”形容某个程序，判别的标准在于该程序是否隐含类型错误。
类型安全的编程语言与类型安全的程序之间，没有必然联系。好的程序员可以使用类型不那么安全的语言写出类型相当安全的程序，
相反的，差一点儿的程序员可能使用类型相当安全的语言写出类型不太安全的程序。绝对类型安全的编程语言暂时还没有。

