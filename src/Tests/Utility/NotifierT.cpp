#include <ECSpp/EntityManager/EntitySpawner.h>
#include <ECSpp/Utility/Notifier.h>
#include <gtest/gtest.h>

using namespace epp;

struct TestStr : public Notifier<EntityEvent>
{
    void f(TestStr& other)
    {
        other.addSubscriber(std::bind(&TestStr::notify, this, std::placeholders::_1), EntityEvent::Type::_Every);
    }

    void notify2()
    {
        notify({ EntityEvent::Type::Creation, Entity() });
    }
};

TEST(Notifier, t1)
{
    TestStr t;
    TestStr t2;
    t.f(t2);
    t.addSubscriber([](EntityEvent const& ev) { printf("test\n"); }, EntityEvent::Type::_Every);
    t2.notify2();
}