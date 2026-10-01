package com.solis.event;

import java.util.ArrayList;
import java.util.List;
import java.util.function.Consumer;

public final class EventBus {
    private final List<Consumer<Object>> listeners = new ArrayList<>();

    public void subscribe(Consumer<Object> listener) { listeners.add(listener); }

    public void post(Object event) {
        for (Consumer<Object> listener : listeners) listener.accept(event);
    }
}
