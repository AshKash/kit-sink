#include <stddef.h>  // for the definition of size_t

size_t* alloc_counter()
{
    return ::new size_t;
}

void dealloc_counter(size_t* ptr)
{
    ::delete ptr;
}

class StandardObjectPolicy {
  public:
    template<typename T> void dispose (T* object) {
        delete object;
    }
};

class SimpleReferenceCount {
  private:
    size_t* counter;    // the allocated counter
  public:
    SimpleReferenceCount () {
        counter = NULL;
    }

    // default copy constructor and copy-assignment operator
    // are fine in that they just copy the shared counter

  public:
    // allocate the counter and initialize its value to one:
    template<typename T> void init (T*) {
        counter = alloc_counter();
        *counter = 1;
    }

    // dispose of the counter:
    template<typename T> void dispose (T*) {
        dealloc_counter(counter);
    }

    // increment by one:
    template<typename T> void increment (T*) {
        ++*counter;
    }
    
    // decrement by one:
    template<typename T> void decrement (T*) {
        --*counter;
    }

    // test for zero:
    template<typename T> bool is_zero (T*) {
        return *counter == 0;
    }
};

template<typename T,
         typename CounterPolicy = SimpleReferenceCount,
         typename ObjectPolicy = StandardObjectPolicy>
class CountingPtr : private CounterPolicy, private ObjectPolicy {
  private:
    // shortcuts:
    typedef CounterPolicy CP;
    typedef ObjectPolicy  OP;

    T* object_pointed_to;      // the object referred to (or NULL if none)

  public:
    // default constructor (no explicit initialization):
    CountingPtr() {
        this->object_pointed_to = NULL;
    }

    // a converting constructor (from a built-in pointer):
    explicit CountingPtr (T* p) {
        this->init(p);         // init with ordinary pointer
    }

    // copy constructor:
    CountingPtr (CountingPtr<T,CP,OP> const& cp)
     : CP((CP const&)cp),      // copy policies
       OP((OP const&)cp) {
        this->attach(cp);      // copy pointer and increment counter
    }

    // destructor:
    ~CountingPtr() {
        this->detach();        // decrement counter
                               //  (and dispose counter if last owner)
    }

    // assignment of a built-in pointer
    CountingPtr<T,CP,OP>& operator= (T* p) {
        // no counting pointer should point to *p yet:
        assert(p != this->object_pointed_to);
        this->detach();        // decrement counter
                               //  (and dispose counter if last owner)
        this->init(p);         // init with ordinary pointer
        return *this;
    }

    // copy assignment (beware of self-assignment):
    CountingPtr<T,CP,OP>&
    operator= (CountingPtr<T,CP,OP> const& cp) {
        if (this->object_pointed_to != cp.object_pointed_to) {
            this->detach();    // decrement counter
                               //  (and dispose counter if last owner)
            CP::operator=((CP const&)cp);  // assign policies
            OP::operator=((OP const&)cp);
            this->attach(cp);  // copy pointer and increment counter
        }
        return *this;
    }

    // the operators that make this a smart pointer:
    T* operator-> () const {
        return this->object_pointed_to;
    }

    T& operator* () const {
        return *this->object_pointed_to;
    }

    // additional interfaces will be added later
    //...

  private:
    // helpers:
    // - init with ordinary pointer (if any)
    void init (T* p) {
        if (p != NULL) {
            CounterPolicy::init(p);
        }
        this->object_pointed_to = p;
    }

    // - copy pointer and increment counter (if any)
    void attach (CountingPtr<T,CP,OP> const& cp) {
        this->object_pointed_to = cp.object_pointed_to;
        if (cp.object_pointed_to != NULL) {
            CounterPolicy::increment(cp.object_pointed_to);
        }
    }

    // - decrement counter (and dispose counter if last owner)
    void detach() {
        if (this->object_pointed_to != NULL) {
            CounterPolicy::decrement(this->object_pointed_to);
            if (CounterPolicy::is_zero(this->object_pointed_to)) {
                // dispose counter, if necessary:
                CounterPolicy::dispose(this->object_pointed_to);
                // use object policy to dispose the object pointed to:
                ObjectPolicy::dispose(this->object_pointed_to);
            }
        }
    }
};
