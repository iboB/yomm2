// md<
// <sub>/ ->home / ->reference </sub>
// ## yorel::yomm2::use_classes
// <sub>defined in <yorel/yomm2/core.hpp></sub>
// ## register_classes
// <sub>defined in <yorel/yomm2/cute.hpp>, also provided by
// <yorel/yomm2/keywords.hpp></sub>
// ## YOMM2_CLASSES
// <sub>defined in <yorel/yomm2/macros.hpp></sub>, also provided by
// <yorel/yomm2/yomm2.hpp>
// <!-- -->
// ---
// ```
// template<typename... Cs> struct use_classes;
// ```
// <!-- -->
// ---

// `use_classes`, instantiated as a [static object](static-object.md), registers
// a list of classes, and their inheritance relationships. All classes that
// potentially take part in a method call must be registered with `use_classes`.

// In order for `use_classes` to correctly deduce the inheritance graphs, if a
// class is a direct base of another class, they must appear together in a same
// instance of `use_classes`. If a class has several direct base classes, they
// need not all appear in the same `use_classes`; inheritance relationships can
// be added incrementally. See examples below.

// Note that the registration requirement does not only apply to classes used as
// virtual parameters, and the classes used as parameters in method definitions
// that correspond to virtual parameters. The runtime class of all the objects
// potentially partaking in method *calls* must be registered. For example,
// given the hierarchy Animal -> Dog -> Bulldog; if a method declaration takes a
// `virtual_<Animal>`; if the method has two definitions, one for Animal, and
// one for Bulldog; and the program calls the method for a Dog (that is not a
// Bulldog); then Dog must be registered as well, and it needs to appear with
// Animal in a `use_classes`, and with Bulldog in a `use_classes`.

// In debug builds, YOMM2 checks at the call site that the runtime classes of
// all the virtual arguments has been registered. If not, an error message is
// written to `cerr`, and `abort()` is called. The check works even if the YOMM2
// runtime was compiled in release mode. If the program itself is compiled in
// release mode, and not all the classes have been registered, the program will
// segfault, or worse, the wrong method definition may be called.

// The time complexity of `use_classes` is O(n^2) a compile time and at runtime
// (during `update_methods`). If necessary, large hierarchies can be registered
// incrementally.

// ## macros

// `register_classes(...)`  and `YOMM2_CLASSES(...)` are simple wrappers around
// `use_classes`, both equivalent to `use_classes<...> YOMM2_GENSYM`.

// ## example

// Given the following hierarchy:

// >

#define BOOST_TEST_MODULE runtime
#include <boost/test/included/unit_test.hpp>
#include <yorel/yomm2/keywords.hpp>

BOOST_AUTO_TEST_CASE(dummy) {}

// code<
struct Animal {
    virtual ~Animal() {
    }
};
struct Herbivore : virtual Animal {};
struct Carnivore : virtual Animal {};
struct Omnivore : Herbivore, Carnivore {};
struct Human : Omnivore {};
struct Wolf : Omnivore {};
struct Sheep : Herbivore {};
// >

// md<
// All the classes can be registered with a single static object:
// >

// code<
// at file scope
using yorel::yomm2::use_classes;
use_classes<
    Animal, Herbivore, Carnivore, Omnivore, Human, Wolf, Sheep
> YOMM2_GENSYM;
// >

// md<
// Or, using either macro:
// >

// code<
register_classes(Animal, Herbivore, Carnivore, Omnivore, Human, Wolf, Sheep);
YOMM2_CLASSES(Animal, Herbivore, Carnivore, Omnivore, Human, Wolf, Sheep);
// >

// md<
// Classes can also be registered incrementally:
// >

// code<
use_classes<Animal, Herbivore, Carnivore> YOMM2_GENSYM;
use_classes<Omnivore, Human, Wolf> YOMM2_GENSYM;
use_classes<Sheep, Herbivore> YOMM2_GENSYM;
// >

// md<

// The following is **wrong**, because `use_classes` cannot infer that `Human`
// and `Wolf` derive from `Omnivore`.

// >

// code<
use_classes<Animal, Herbivore, Carnivore, Omnivore> YOMM2_GENSYM;
use_classes<Human, Wolf> YOMM2_GENSYM;      // wrong!
use_classes<Sheep, Herbivore> YOMM2_GENSYM; // ok
// >

// md<

// ## see also

// |  | |
// |---|---|
// | ->YOMM2_GENSYM | generate a symbol

//

// >
