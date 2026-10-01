package com.solis.module;

import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.Map;

public final class ModuleManager {
    private final Map<Class<? extends Module>, Module> modules = new LinkedHashMap<>();

    public void register(Module module) { modules.put(module.getClass(), module); }
    public void registerDefaults() {}

    public void tick() {
        for (Module module : modules.values()) {
            if (module.isEnabled()) module.onTick();
        }
    }

    public <T extends Module> T get(Class<T> type) { return type.cast(modules.get(type)); }
    public Collection<Module> all() { return modules.values(); }
}
