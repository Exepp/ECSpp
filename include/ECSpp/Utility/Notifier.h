#ifndef NOTIFIER_H
#define NOTIFIER_H

#include <functional>
#include <vector>

namespace epp
{

template<class Event>
class Notifier
{
    static_assert(std::is_unsigned_v<std::underlying_type_t<typename Event::Type>>);

public:
    using EvType_t = typename Event::Type;

private:
    using This_t = Notifier<Event>;

    using Callback_t = std::function<void(Event&)>;

    using NotifyCb_t = void (This_t::*)(Event&) const;


    static constexpr EvType_t const EveryType = EvType_t::_Every;

    static constexpr std::size_t const NumOfEv = std::size_t(EveryType);

    static_assert(NumOfEv > 0, "Cannot create an empty event notifier");


    using EvCbVec_t = std::vector<Callback_t>;

    using EvCbVecArr_t = std::vector<EvCbVec_t>;


public:
    Notifier() = default;

    Notifier(This_t const&) = delete;

    Notifier(This_t&&) = delete;

    This_t& operator=(This_t const&) = delete;

    This_t& operator=(This_t&&) = delete;

    virtual ~Notifier() = default;


    void addSubscriber(Callback_t callback, EvType_t type)
    {
        callbacks[std::size_t(type)].push_back(std::move(callback));
    }

    template<class T, class F>
    void addSubscriber(T* sub, F callback, EvType_t type)
    {
        Callback_t callb = [sub, callback](Event& e) { (sub->*callback)(e); };
        callbacks[std::size_t(type)].push_back(callb);
    }

    template<class T>
    void addSubscriber(T* sub, NotifyCb_t callback, EvType_t type) // when Notifier::notify is the callback (to distinguish between overloads)
    {
        addSubscriber<T, NotifyCb_t>(sub, callback, type);
    }

protected:
    void notify(Event& event) const
    {
        assert(event.type != EveryType);

        for (auto const& cb : callbacks[std::size_t(EveryType)])
            cb(event);
        for (auto const& cb : callbacks[std::size_t(event.type)])
            cb(event);
    }

    void notify(Event&& event) const
    {
        notify(event);
    }

private:
    EvCbVecArr_t callbacks = EvCbVecArr_t(NumOfEv + 1);
};

} // namespace epp

#endif // NOTIFIER_H