// Copyright (c) 2018-2022 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/mpl/print.hpp>

#define BOOST_TEST_MODULE api
#include <boost/test/included/unit_test.hpp>

// This is required by Boost.Test's error reporting.
namespace std {
std::ostream& operator<<(std::ostream& os, const std::type_info& ti) {
    return os << ti.name();
}
} // namespace std

#define elided

// namespace meta = meta;

//using namespace boost::mp11;

// md<

// # Using YOMM2 without macros

// YOMM2 provides a public interface that does not require using macros. This
// can be useful in certain situations, for example when combining open methods
// and templates - see the [templates tutorial](templates.md).

// The following code is a partial rewrite of the synopsis example that does not
// use any macros.

// >

#include <yorel/yomm2/core.hpp> // make it innocuous to include it in a namespace

namespace synopsis_functions_no_macros {

// code<
#include <yorel/yomm2/core.hpp>

using namespace yorel::yomm2;

class Animal {
  public:
    virtual ~Animal() {}
};

class Dog : public Animal {};
class Bulldog : public Dog {};

use_classes<Animal, Dog, Bulldog> use_animal_classes;

// >

// md<

// The new `use_classes` template takes any number of
// classes, and infers the inheritance relationships they may have between them.
// Instantiating a `use_classes` object registers the
// classes, in the same fashion as - but more conveniently than - a series of
// invocations of `register_class`.

// >

// code<
struct kick_key;
using kick_method = method<kick_key, std::string(virtual_<Animal&>)>;
// >

// md<

// A YOMM2 method is implemented as a singleton of an instance of the `method`
// template. The second argument is obviously the signature of the method -
// including the return type and the `virtual_` markers.

// What about the first argument? Its role is to separate different methods with
// the same signature. Consider a more animal-friendly method:

// >

// code<
struct feed_key;
using feed_method = method<feed_key, std::string(virtual_<Animal&>)>;
// >

// md<

// In the absence of the first parameter, `kick` and `feed` would be the same
// method. Together, the two arguments provide a unique key for the method.
// Since the `kick_key` and `feed_key` types are local to the current namespace,
// this scheme also protects against accidental interference across namespaces.

// The same key can be used for more than one method, provided that the
// signatures are different. The good practice is to use the same key for all
// the methods in a namespace that have the same name.

// Now let's add a definition to the method:

// >

// code<
std::string kick_dog(Dog& dog) {
    return "bark";
}

kick_method::add_function<kick_dog> add_kick_dog;
// >

// md<

// Note that the name of the function serving as a method definition must be
// unique; in presence of overloads, we would hav eno means of picking the
// appropriate function. Function templates and explicit specialization can also
// be used for this purpose.

// What about `next`? The constructor of `add_function` can be passed a pointer
// to a function that will be set to the function's next definition by
// `update_methods`. The pointer type is available in the method as `next_type`.

// >

// code<
kick_method::next_type kick_bulldog_next;

std::string kick_bulldog(Bulldog& dog) {
    return kick_bulldog_next(dog) + " and bite back";
}

kick_method::add_function<kick_bulldog> add_kick_bulldog(&kick_bulldog_next);
// >

// md<

// We can now call the method. The class contains a static function object named
// `fn`, whose `operator()` has the signature specified in the method
// declaration, minus the `virtual_<>` decorators.

// >

// code<
BOOST_AUTO_TEST_CASE(test_synopsis_functions_no_macros) {
    update_methods();

    std::unique_ptr<Animal> snoopy = std::make_unique<Dog>();
    BOOST_TEST(kick_method::fn(*snoopy) == "bark");

    std::unique_ptr<Animal> hector = std::make_unique<Bulldog>();
    BOOST_TEST(kick_method::fn(*hector) == "bark and bite back");
}
// >

// md<

// ## A peek inside the two main YOMM2 macros

// The code in the example above is essentially what
// `YOMM2_DECLARE`/`declare_method` and `YOMM2_DEFINE`/`define_method` generate.

// In addition, `declare_method` generates an ordinary inline function that
// forwards to the `fn` object nested inside the method. Importantly, ordinary
// functions can be overloaded, and their address can be taken, which is not the
// case for function objects.

// `declare_method` also declares a guide function that enables `define_method`
// to find the method being specialized.

// `define_method` wraps the function body inside a class, along with a `next`
// static variable. It fakes a call to a guide function named after the method,
// passing it `declval` arguments for the definition's parameter list. The
// compiler performs overload resolution, and the macro uses `decltype` to
// extract the result type, i.e the method's class, and registers the definition
// and the `next` pointer with `add_function`.

// In the process, both macros need to create identifiers for the various static
// objects, and the name of the function inside the definition wrapper class.
// These symbols are generated by two macros; in both cases, the symbols are
// copiously obfuscated, to minimize the risk of collision with the user's
// symbols.

// - `YOMM2_GENSYM` expands to a new symbol each time it is called. It is used
//   for the static "registrar" objects.

// - `YOMM2_SYMBOL(name)` expands to an obfuscated version of `name`. It is used
//   for the method key and the guide function.

// Both macros are defined by header file `yorel/yomm2/symbols.hpp`.

// >

} // namespace synopsis_functions_no_macros

// md<

// ## Trimming verbosity

// The "synopsis" example is quite verbose. Many of the names used in it are
// pure noise. They are use to define static objects, for the sole purpose of
// executing their constructor. They are never referenced explicitly. In a
// language like Python, we would simply call functions at file scope; C++ does
// not allow that, or only by ruse and abuse.

// Let's rewrite the example, this time using the symbol-generation macros, and
// a helper.

// >

// md<

// (`Animal` classes same as before)

// >

namespace synopsis_better {
class Animal {
  public:
    virtual ~Animal() {}
};

class Dog : public Animal {};
class Bulldog : public Dog {};

// code<
#include <yorel/yomm2/core.hpp>
#include <yorel/yomm2/symbols.hpp>

using namespace yorel::yomm2;

use_classes<Animal, Dog, Bulldog> YOMM2_GENSYM;

struct YOMM2_SYMBOL(kick);

using kick_method = method<YOMM2_SYMBOL(kick), std::string(virtual_<Animal&>)>;

// >

// md<

// `add_function` is a workhorse that is intended to be used directly only by
// `define_method`. YOMM2 has another mechanism that is a bit more high level:
// *definition containers*.

// A definition container is a struct that, at the minimum, contains a static
// function named `fn`. Containers are added to methods via the
// `add_definition` nested type:

// >

// code<
struct kick_dog {
    static std::string fn(Dog& dog) { return "bark"; }
};

kick_method::add_definition<kick_dog> YOMM2_GENSYM;
// >

// md<

// This may not seem like a huge improvement, until we need a `next` function.
// If the container has a static member variable called `next`, and it is of the
// appropriate type, `add_definition` will pick it up for `update_methods` to
// fill. Static member variables are a bit clumsy, because, unlike functions,
// they must be declared inside the class, and defined outside. Methods have a
// nested CRTP helper to inject a `next` into a container.

// >

// code<
struct kick_bulldog : kick_method::next<kick_bulldog> {
    static std::string fn(Bulldog& dog) {
        return next(dog) + " and bite back";
    }
};

kick_method::add_definition<kick_bulldog> YOMM2_GENSYM;
// >

// md<

// Do you have doubts about the value of definition containers? Here are two
// more reasons why you should use them.

// 1. Containers are the core of the best pattern I could come up with to
//    implement templatized methods and definitions.

// 2. In the future, additional functionality may be added to containers.

// The rest of the example is as before.

// >

BOOST_AUTO_TEST_CASE(test_synopsis_definition_containers) {
    update_methods();

    std::unique_ptr<Animal> snoopy = std::make_unique<Dog>();
    BOOST_TEST(kick_method::fn(*snoopy) == "bark");

    std::unique_ptr<Animal> hector = std::make_unique<Bulldog>();
    BOOST_TEST(kick_method::fn(*hector) == "bark and bite back");
}

} // namespace synopsis_better
