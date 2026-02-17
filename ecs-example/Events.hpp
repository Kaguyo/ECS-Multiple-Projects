#pragma once
#include <functional>
#include <vector>

enum class EvtType {
    Attack,
    Defend,
    Special
};

struct Event {
    EvtType type;
    int sourceID;
    int targetID;
};

using EventCallback = std::function<void(const Event&)>;

class EventBus {
public:
    void subscribe(EvtType type, EventCallback cb) {
        listeners[(int)type].push_back(cb);
    }

    void send(const Event& e) {
        for (auto &cb : listeners[(int)e.type]) {
            cb(e);
        }
    }
private:
    std::vector<EventCallback> listeners[3];
};
