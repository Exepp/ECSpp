#ifndef NOTIFIER_H
#define NOTIFIER_H

#include <array>
#include <functional>
#include <vector>

namespace epp
{

template<class Event>
class Notifier
{
    using EvType_t = typename Event::Type;

    static constexpr size_t const NumOfEv = (size_t)EvType_t::Count;

    using Callback_t = std::function<void(Event const&)>;

    using EvCbVec_t = std::vector<Callback_t>;

    using EvCbVecArr_t = std::array<EvCbVec_t, NumOfEv>;


public:
    template<class T, class F, class... Args>
    void addSubscriber(T& sub, F callback, EvType_t type)
    {
        Callback_t callb = [&, sub](Event const& e) { (sub.*callback)(e); };
        callbacks[(size_t)type].push_back(callb);
    }

protected:
    void notify(Event const& event) const
    {
        for (auto const& cb : callbacks[(size_t)event.type])
            cb(event);
    }

private:
    EvCbVecArr_t callbacks;
};

} // namespace epp
#endif // NOTIFIER_H