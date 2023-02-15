// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include "test/helper.hpp"

#include "lib/common/template_remover.hpp"

using namespace fcpp;
using namespace common;

int run_status;

struct net {
    template <typename S, typename T>
    net(tagged_tuple<S,T> const&) {}
    net(net const&) = delete;
    net& operator=(net const&) = delete;
    void run(int x) {
        run_status = x;
    }
    double next() {
        return 42;
    }
};

struct ten {
    template <typename S, typename T>
    ten(tagged_tuple<S,T> const&) {}
    ten(ten const&) = delete;
    ten& operator=(ten const&) = delete;
    void run(int x) {
        run_status = -x;
    }
    double next() {
        return 24;
    }
};

namespace mess {
    struct tag {};
    struct gat {};
}
struct oth {};
struct hto {};

using tuple_type = tagged_tuple_t<mess::tag, int, mess::gat, std::string, oth, net, hto, short>;
tagged_tuple_t<> none;

template <typename T>
using int_conv = std::is_convertible<T, int>;

template <typename T>
struct net_like {
    static constexpr bool value = std::is_same<T, net>::value or std::is_same<T, ten>::value;
};

TEST(TemplateRemoverTest, TypeFilter) {
    EXPECT_SAME(type_filter<int_conv, int, double, std::string, net, char>, type_sequence<int, double, char>);
    EXPECT_SAME(type_filter<net_like, int, double, std::string, net, char>, type_sequence<net>);
}

TEST(TemplateRemoverTest, TaggedTupleFilter) {
    EXPECT_SAME(tagged_tuple_filter<int_conv, tuple_type>, tagged_tuple_t<mess::tag, int, hto, short>);
    EXPECT_SAME(tagged_tuple_filter<net_like, tuple_type>, tagged_tuple_t<oth, net>);
    EXPECT_SAME(tagged_tuple_filter<convertible_to<int>::template predicate,tuple_type>, tagged_tuple_t<mess::tag, int, hto, short>);
    EXPECT_SAME(tagged_tuple_filter<can_assign<int>::template predicate,tuple_type>, tagged_tuple_t<mess::tag, int, mess::gat, std::string, hto, short>);
}

auto inc = [](int x){ return x + 1; };
auto run = [](auto& n){ n.run(42); };
auto nxt = [](auto& n){ return n.next(); };

TEST(TemplateRemoverTest, ApplierReturnType) {
    EXPECT_SAME(applier_return_type<tuple_type, decltype(inc), int_conv>, int);
    EXPECT_SAME(applier_return_type<tuple_type, decltype(run), net_like>, void);
    EXPECT_SAME(applier_return_type<tuple_type, decltype(nxt), net_like>, double);
    EXPECT_SAME(applier_return_type<tuple_type&, decltype(inc), int_conv>, int);
    EXPECT_SAME(applier_return_type<tuple_type&, decltype(run), net_like>, void);
    EXPECT_SAME(applier_return_type<tuple_type&, decltype(nxt), net_like>, double);
    EXPECT_SAME(applier_return_type<tuple_type&&, decltype(inc), int_conv>, int);
    EXPECT_SAME(applier_return_type<tuple_type&&, decltype(run), net_like>, void);
    EXPECT_SAME(applier_return_type<tuple_type&&, decltype(nxt), net_like>, double);
    EXPECT_SAME(applier_return_type<tuple_type const, decltype(inc), int_conv>, int);
    EXPECT_SAME(applier_return_type<tuple_type const, decltype(run), net_like>, void);
    EXPECT_SAME(applier_return_type<tuple_type const, decltype(nxt), net_like>, double);
    EXPECT_SAME(applier_return_type<tuple_type const&, decltype(inc), int_conv>, int);
    EXPECT_SAME(applier_return_type<tuple_type const&, decltype(run), net_like>, void);
    EXPECT_SAME(applier_return_type<tuple_type const&, decltype(nxt), net_like>, double);
}

TEST(TemplateRemoverTest, Applier) {
    common::tagged_tuple_t<oth, net, hto, ten> t(none, none);
    run_status = 0;
    applier("oth", t, run);
    EXPECT_EQ(run_status, 42);
    applier("hto", t, run);
    EXPECT_EQ(run_status, -42);
    EXPECT_EQ(applier("oth", t, nxt), 42);
    EXPECT_EQ(applier("hto", t, nxt), 24);
}

TEST(TemplateRemoverTest, FilteredApplier) {
    tuple_type t(24, "hi", none, 42);
    EXPECT_EQ(applier<int_conv>("tag", t, inc), 25);
    EXPECT_EQ(applier<int_conv>("hto", t, inc), 43);
}

TEST(TemplateRemoverTest, Getter) {
    tuple_type t(24, "hi", none, 42);
    EXPECT_EQ(getter<int>("tag", t), 24);
    EXPECT_EQ(getter<std::string>("gat", t), "hi");
    EXPECT_EQ(getter<int>("hto", t), 42);
}

TEST(TemplateRemoverTest, Setter) {
    tuple_type t(24, "hi", none, 42);
    setter("tag", t, 10);
    setter("gat", t, "nah");
    setter("hto", t, 17);
    EXPECT_EQ(get<mess::tag>(t), 10);
    EXPECT_EQ(get<mess::gat>(t), "nah");
    EXPECT_EQ(get<hto>(t), 17);
}
