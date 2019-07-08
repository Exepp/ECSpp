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
public:
    using EvType_t = typename Event::Type;

private:
    using This_t = Notifier<Event>;

    using Callback_t = std::function<void(Event&)>;

    using NotifyCb_t = void (This_t::*)(Event&) const;

    using NotifyFn_t = std::function<void(This_t const&, Event&)>;


    static constexpr size_t const NumOfEv = (size_t)EvType_t::_Count;

    static constexpr EvType_t const EveryType = EvType_t::_Every;

    static_assert(NumOfEv < (size_t)EveryType, "_Count should always come before _Every");


    using EvCbVec_t = std::vector<Callback_t>;

    using EvCbVecArr_t = std::array<EvCbVec_t, NumOfEv>;


public:
    Notifier() = default;

    Notifier(This_t const&) = delete;

    Notifier(This_t&&) = delete;

    This_t& operator=(This_t const&) = delete;

    This_t& operator=(This_t&&) = delete;

    virtual ~Notifier() = default;


    template<class T, class F>
    void addSubscriber(T* sub, F callback, EvType_t type)
    {
        Callback_t callb = [sub, callback](Event& e) { (sub->*callback)(e); };
        if (type == EveryType)
            everyTypeCallbacks.push_back(callb);
        else
            callbacks[(size_t)type].push_back(callb);
    }

    template<class T>
    void addSubscriber(T* sub, NotifyCb_t callback, EvType_t type)
    {
        addSubscriber<T, NotifyCb_t>(sub, callback, type);
    }

    // protected:
    void notify(Event& event) const
    {
        for (auto const& cb : everyTypeCallbacks)
            cb(event);
        for (auto const& cb : callbacks[(size_t)event.type])
            cb(event);
    }

    void notify(Event&& event) const
    {
        notify(event);
    }

private:
    EvCbVecArr_t callbacks;

    EvCbVec_t everyTypeCallbacks;
};

} // namespace epp
#endif // NOTIFIER_H