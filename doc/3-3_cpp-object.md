There are two ways of representing C++ objects in the runtime system.

**Objects are any *type* that *directly* exhibits polymorphism**. Hence the zero case is that if your object has no virtual methods (nor virtual inheritance) it is a structure (and hence not an object) and the following discussion does not apply. Finally if your type only exhibits polymorphism through external runtime system features (like multimethods, or concepts) then this discussion also does not apply.

C++ objects support only single dispatch (object) methods, but we must represent their dispatch to the runtime system in some way. The main difficulty we are working around here is that we cannot (without importing the compiler's internals) generate virtual function tables at runtime. In general either of the following ways support creating dispatch hierarchies for both object-methods and multi-methods. They primarily have tradeoffs in how the objects can be interacted with by programmers.

The first way is to program the objects as if they are normal C++ objects, and then wrap their defines in a way that they can be placed into the runtime system. The primary benefit of this is that the types in question behave exactly like a C++ programmer would expect. And they can be defined without modifying the header files (or code files) the classes appear in. The downside of this is that the C++ code has to suffer a notable performance penalty or write a significant amount of additional code if it wishes to obey the runtime system (e.g. if at runtime someone subclasses from a C++ class).

The second way (WIP, see [dyno](https://github.com/ldionne/dyno) for an example) is to program the objects as if they were runtime native objects (e.g. that they are laid out similar to what a compiler obeying the runtime system might decide to generate). This is done using a library of helper templates, the most notable of which is defining the virtual function table by hand (along with the helpers for additional features). Compared to the first method, this is more core (though it is all new code). However this system has an easier expressive power that matches the runtime system, while being as performant as C++ compiled virtual methods by using template magic to use similar optimizations (e.g. inlining functions when types are guaranteed).

## C++ Objects

This is the first way, where we try to work with existing C++ objects, using the classic C++ syntax. This is the easiest way to start using the system, but has some of the more complex restrictions and behaviours (as edge cases for each must be accounted for). Notably any types created with these methods will have limitations on how they can be inherited from in someway (due to not having access to the virtual function table).

We layout a number of approaches, they generally assume we have a C++ hierarchy like the below:

```c++
class AbstractBase
{
  /* data members */
  protected:
    int _foo;
  /* methods */
  public:
    void virtual theMethod(SomeOtherObject* other, int num) = 0;
};

class Base : public AbstractBase
{
  /* data members */
  protected:
    int _bar;
  public:
    int aPublicMember;
  /* methods */
  public:
    void virtual theMethod(SomeOtherObject* other, int num) override;
};

class Derived : public Base
{
  /* data members */
  protected:
    int _baz;
  /* methods */
  public:
    void virtual theMethod(SomeOtherObject* other, int num) override;
};

```

And then lets assume we defined something for `SomeOtherObject`:

```C++
// .cpp
decltype(syn::type_define<SomeOtherObject>::Definition) syn::type_define<SomeOtherObject>::Definition(
	[](auto _)
    {
        /*...*/
    });
```

### External - Standalone

Let's say we don't care about the hierarchy. In this case we can basically treat it as if it is an external non-object:

```c++
// .h
// this boilerplate ommitted where used (e.g. syn::type_define<>::Definition) in future examples
namespace syn
{
    template<>
    struct type_define<Base>
    {
        static syn::Define<Base> Definition;
    };
}

// .cpp
decltype(syn::type_define<Base>::Definition) syn::type_define<Base>::Definition(
	[](auto _)
    {
        _.name("Base");
        _.detectLifecycle();
        
    	_.member("aPublicMember", &Base::aPublicMember);
    	_.method("theMethod", &Base::theMethod);
    });
```

Now as far as the type system is concerned the hierarchy is effectively `C++ object -> Base`.

If somehow a C++ derivative of it makes it into the runtime system (e.g. `Derived`) without being added to the runtime:
* The virtual function `theMethod` will dispatch correctly when called from the runtime system
  * and importantly virtual destructors will clean it up correctly.
* The runtime system will incorrectly believe the type to be `Base` rather than `Derived`.
* Virtual inheritence (and correctly cast multiple inheritence in general) should function.

If this type is inherited on the runtime side:
* The C++ side will not be able to call into it correctly (the virtual functions will not take the runtime overrides into account).
  * By default (if it detects any virtual methods) this will cause a warning on the runtime side.

This is an acceptable way to expose a C++ object hiearchy if you don't expect users to extend it. 

### Stub - Standalone

A similar alternative that allows access to protected members is to create a stub:

```c++
// .h
class StubBase final : public Base
{
public:
    static syn::Define<StubBase> Definition;
};

// .cpp
decltype(StubBase::Definition) StubBase::Definition(
	[](auto _)
    {
        _.name("Base");
        _.detectLifecycle();
        _.finalized(); // optional
        
    	_.member("_bar", &StubBase::_bar).protected_();
    	_.member("_baz", &StubBase::_baz).protected_();
        
    	_.member("aPublicMember", &StubBase::aPublicMember);
    	_.method("theMethod", &StubBase::theMethod);
    });
```

Here we gain access to the protected members.

By defining it final (and calling `finalized()`) we are telling the runtime system that the C++ system won't be able to pass in a derivative of this class. As long as the C++ system obeys type safety (e.g. doesn't present a `Base` as a `StubBase`) the runtime system now has better visibility into this object (e.g. it can optimize around it, and extend it natively). This is however an optional feature.

However we still have the problem of inheritance on the runtime side not being translated back to the C++ side.

This is a safer alternative to the standalone strategy with some additional features (like access to protected members, or use of final).

### External - Hierarchy

We can bring the whole hierarchy in if we wish:

```c++
// .cpp
decltype(syn::type_define<AbstractBase>::Definition) syn::type_define<AbstractBase>::Definition(
	[](auto _)
    {
        _.name("AbstractBase");
        _.detectLifecycle(); // will call _.abstract()
        
    	_.method("theMethod", &Derived::theMethod);
    });

decltype(syn::type_define<Base>::Definition) syn::type_define<Base>::Definition(
	[](auto _)
    {
        _.name("Base");
        _.inherits<AbstractBase>();
        _.detectLifecycle();
        
    	_.member("aPublicMember", &Derived::aPublicMember);
    });

decltype(syn::type_define<Derived>::Definition) syn::type_define<Derived>::Definition(
	[](auto _)
    {
        _.name("Derived");
        _.inherits<Base>();
        _.detectLifecycle();
    });
```

This allows the runtime system to create any of the given objects, as well as good visibility into them. Note we only have to register `theMethod` once, because the C++ support system will know it can simply call the base wrapper around it (e.g., it understands member function pointers). Importantly, compile time overloading must still be registered multiple times.

### External - Hierarchy w/ Method

By using an external method object to register the hierarchy's methods with we get pretty close to fully integrated objects:


```c++
// .h
extern syn::Method<> MyMethod;


// .cpp
decltype(syn::type_define<AbstractBase>::Definition) syn::type_define<AbstractBase>::Definition(
	[](auto _)
    {
        _.name("AbstractBase");
        _.detectLifecycle(); // will call _.abstract()
        
    	_.method("theMethod", &Derived::theMethod);
    });

decltype(syn::type_define<Base>::Definition) syn::type_define<Base>::Definition(
	[](auto _)
    {
        _.name("Base");
        _.inherits<AbstractBase>();
        _.detectLifecycle();
        
    	_.member("aPublicMember", &Derived::aPublicMember);
    });

decltype(syn::type_define<Derived>::Definition) syn::type_define<Derived>::Definition(
	[](auto _)
    {
        _.name("Derived");
        _.inherits<Base>();
        _.detectLifecycle();
    });
```



## Runtime Objects