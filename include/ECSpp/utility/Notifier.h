#ifndef EPP_NOTIFIER_H
#define EPP_NOTIFIER_H

#include <functional>
#include <vector>

namespace epp {

template <typename Event>
class Notifier {
    static_assert(std::is_unsigned_v<std::underlying_type_t<typename Event::Type>>);

public:
    using EvType_t = typename Event::Type;

private:
    using Callback_t = std::function<void(Event const&)>;

    static constexpr EvType_t const EveryType = EvType_t::_Every;
    static constexpr std::size_t const NumOfEv = std::size_t(EveryType);

    static_assert(NumOfEv > 0, "Cannot create an empty event notifier");

    using EvCbVec_t = std::vector<Callback_t>;
    using EvCbVecArr_t = std::vector<EvCbVec_t>;

public:
    Notifier() = default;
    Notifier(Notifier const&) = delete;
    Notifier(Notifier&&) = delete;
    Notifier& operator=(Notifier const&) = delete;
    Notifier& operator=(Notifier&&) = delete;

    void addSubscriber(Callback_t callback, EvType_t type)
    {
        callbacks[std::size_t(type)].push_back(std::move(callback));
    }

protected:
    void notify(Event const& event) const
    {
        EPP_ASSERT(event.type != EveryType);

        for (auto const& cb : callbacks[std::size_t(EveryType)])
            cb(event);
        for (auto const& cb : callbacks[std::size_t(event.type)])
            cb(event);
    }

private:
    EvCbVecArr_t callbacks = EvCbVecArr_t(NumOfEv + 1);
};

} // namespace epp

#endif // EPP_NOTIFIER_H