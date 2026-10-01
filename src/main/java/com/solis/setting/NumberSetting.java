package com.solis.setting;

public final class NumberSetting extends Setting<Double> {
    private final double min;
    private final double max;

    public NumberSetting(String name, double value, double min, double max) {
        super(name, value);
        this.min = min;
        this.max = max;
    }

    public double getMin() { return min; }
    public double getMax() { return max; }
}
