#pragma once
#include <catch/catch.hpp>
#include <probe/probe.hpp>

namespace semantics_flags
{
    using type = int;
    constexpr type None = 0;
    constexpr type NoConstruct = 1 << 0;
    constexpr type NoCopy = 1 << 1;
    constexpr type NoMove = 1 << 2;
}

template <typename probe, typename functor>
void verify_semantics(semantics_flags::type flags, functor f)
{
    probe::reset_counts();

    f();

    auto counts = probe::last();

    if(flags & semantics_flags::NoConstruct)
        REQUIRE((counts.defco == 0 && counts.valco == 0 && counts.movco == 0 && counts.copco == 0));
    if(flags & semantics_flags::NoCopy)
        REQUIRE(counts.copies == 0);
    if(flags & semantics_flags::NoMove)
        REQUIRE(counts.moves == 0);
}

template <typename probe>
struct no_leak_verifier
{
    no_leak_verifier()
    {
        probe::reset_state();
    }
    no_leak_verifier(const no_leak_verifier&) = delete;
    no_leak_verifier(no_leak_verifier&&) = delete;
    no_leak_verifier& operator=(const no_leak_verifier&) = delete;
    no_leak_verifier& operator=(no_leak_verifier&&) = delete;
    ~no_leak_verifier()
    {
        REQUIRE(probe::clean());
    }
};
