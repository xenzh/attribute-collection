#include <iostream>
#include <sstream>
#include <string_view>
#include <cassert>

#include "attribute.h"
#include "holder.h"
#include "collection.h"
#include"tags.h"


namespace porter {


void test_attribute()
{
    // value and attr assignment
    attr::Single<tag::service, true> service;
    service = Service("pisvc");
    assert(*service);
    assert(**service == "pisvc");

    // Collection attr assignment and access
    using Coll = attr::Collection<
        attr::Single<tag::service, true>,
        attr::Single<tag::label, false>,
        attr::Multiple<tag::context>
    >;

    Coll coll;
    coll << Service("pisvc");
    coll << Context("LID", "FIINDEX:LUATTRUU")
         << Context("DFPATH", "anton-test.1");

    assert( coll(tag::service {}) == "pisvc" );
    assert( coll(tag::context {}, "LID")->get() == "FIINDEX:LUATTRUU" );
    assert( coll(tag::context {}, "NONE") == std::nullopt );

    // Construction from other and extension.
    using PartialMatch = attr::Collection<
        attr::Single<tag::service, true>,
        attr::Single<tag::subsystem, true>
    >;

    PartialMatch partial;
    partial << Service("integsvc");
    partial << Subsystem("fcalchippo");

    Coll base {partial};

    assert( base(tag::service {}) == "integsvc" );

    auto extended = std::move(base).extend(Label(42), Pwho(1234));
    assert( extended(tag::service {}) == "integsvc" );
    assert( extended(tag::label {}) == 42 );
    assert( extended(tag::pwho {}) == 1234 );

    auto sum = Service("pisvc") + Subsystem("adc");
    assert( sum(tag::service {}) == "pisvc" );
    assert( sum(tag::subsystem {}) == "adc" );

    auto sum2 = std::move(sum) + Label(4242);
    assert( sum2(tag::service {}) == "pisvc" );
    assert( sum2(tag::subsystem {}) == "adc" );
    assert( sum2(tag::label {}) == 4242 );
}


} // namespace porter


int main()
{
    porter::test_attribute();
    return 0;
}
