#pragma once
#include <catch/catch.hpp>
#include <probe/probe.hpp>
#include <flg/flags.hpp>

struct semantics_flags: flg::flags<int, semantics_flags>
{
    constexpr static flag None        = none_flag();
    constexpr static flag NoConstruct = from_index(0);
    constexpr static flag NoCopy      = from_index(1);
    constexpr static flag NoMove      = from_index(2);
};

template <typename probe, typename functor>
void verify_semantics(semantics_flags flags, functor f)
{
    probe::reset_counts();

    f();

    auto counts = probe::last();

    if(flags & semantics_flags::NoConstruct)
    {
        REQUIRE(counts.defco == 0);
        REQUIRE(counts.valco == 0);
        REQUIRE(counts.copco == 0);
        REQUIRE(counts.movco == 0);
    }
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
