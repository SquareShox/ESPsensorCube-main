#include <Arduino.h>
#include <sensors.h>
#include <modbus_handler.h>
#include <config.h>
#include <calib.h>

// Temporal moving average periods
#define FAST_PERIOD_MS (10 * 1000)   // 10 seconds
#define SLOW_PERIOD_MS (5 * 60 * 1000) // 5 minutes

// Sample buffer sizes (adjust based on available memory)
#define FAST_BUFFER_SIZE 30   // ~30 samples for 10s average
#define SLOW_BUFFER_SIZE 100  // ~100 samples for 5min average

// Generic circular buffer template for sensor data
template<typename T, size_t SIZE>
class CircularBuffer {
private:
    T buffer[SIZE];
    size_t head = 0;
    size_t count = 0;
    unsigned long timestamps[SIZE];
    
public:
    void push(const T& value, unsigned long timestamp) {
        buffer[head] = value;
        timestamps[head] = timestamp;
        head = (head + 1) % SIZE;
        if (count < SIZE) count++;
    }
    
    size_t size() const { return count; }
    bool empty() const { return count == 0; }
    
    T getWeightedAverage(unsigned long currentTime, unsigned long maxAge) {
        if (empty()) return T{};
        
        T sum = T{};
        float totalWeight = 0.0;
        
        for (size_t i = 0; i < count; i++) {
            size_t index = (head + SIZE - count + i) % SIZE;
            unsigned long age = currentTime - timestamps[index];
            
            if (age <= maxAge) {
                // Exponential decay weight - newer samples have higher weight
                float weight = exp(-((float)age / maxAge) * 2.0);
                sum = addWeighted(sum, buffer[index], weight);
                totalWeight += weight;
            }
        }
        
        if (totalWeight > 0) {
            return divideByWeight(sum, totalWeight);
        }
        return T{};
    }
    
    T getSimpleAverage(unsigned long currentTime, unsigned long maxAge) {
        if (empty()) return T{};
        
        T sum = T{};
        size_t validCount = 0;
        
        for (size_t i = 0; i < count; i++) {
            size_t index = (head + SIZE - count + i) % SIZE;
            unsigned long age = currentTime - timestamps[index];
            
            if (age <= maxAge) {
                sum = addSimple(sum, buffer[index]);
                validCount++;
            }
        }
        
        if (validCount > 0) {
            return divideByCount(sum, validCount);
        }
        return T{};
    }
    
private:
    // Template specialization helpers - will be implemented for each type
    T addWeighted(const T& a, const T& b, float weight);
    T addSimple(const T& a, const T& b);
    T divideByWeight(const T& sum, float weight);
    T divideByCount(const T& sum, size_t count);
};

// Solar sensor averaging
template<>
SolarData CircularBuffer<SolarData, FAST_BUFFER_SIZE>::addWeighted(const SolarData& a, const SolarData& b, float weight) {
    SolarData result = a;
    if (b.valid) {
        result.valid = true;
        // For solar data, we mainly average numeric values
        float aV = a.V.toFloat();
        float bV = b.V.toFloat();
        result.V = String(aV + bV * weight);
        
        float aI = a.I.toFloat();
        float bI = b.I.toFloat();
        result.I = String(aI + bI * weight);
        
        float aVPV = a.VPV.toFloat();
        float bVPV = b.VPV.toFloat();
        result.VPV = String(aVPV + bVPV * weight);
        
        float aPPV = a.PPV.toFloat();
        float bPPV = b.PPV.toFloat();
        result.PPV = String(aPPV + bPPV * weight);
        
        // Keep most recent non-numeric data
        if (b.PID.length() > 0) result.PID = b.PID;
        if (b.FW.length() > 0) result.FW = b.FW;
        if (b.LOAD.length() > 0) result.LOAD = b.LOAD;
    }
    return result;
}

template<>
SolarData CircularBuffer<SolarData, FAST_BUFFER_SIZE>::addSimple(const SolarData& a, const SolarData& b) {
    SolarData result = a;
    if (b.valid) {
        result.valid = true;
        result.V = String(a.V.toFloat() + b.V.toFloat());
        result.I = String(a.I.toFloat() + b.I.toFloat());
        result.VPV = String(a.VPV.toFloat() + b.VPV.toFloat());
        result.PPV = String(a.PPV.toFloat() + b.PPV.toFloat());
    }
    return result;
}

template<>
SolarData CircularBuffer<SolarData, FAST_BUFFER_SIZE>::divideByWeight(const SolarData& sum, float weight) {
    SolarData result = sum;
    if (weight > 0) {
        result.V = String(sum.V.toFloat() / weight);
        result.I = String(sum.I.toFloat() / weight);
        result.VPV = String(sum.VPV.toFloat() / weight);
        result.PPV = String(sum.PPV.toFloat() / weight);
    }
    return result;
}

template<>
SolarData CircularBuffer<SolarData, FAST_BUFFER_SIZE>::divideByCount(const SolarData& sum, size_t count) {
    SolarData result = sum;
    if (count > 0) {
        result.V = String(sum.V.toFloat() / count);
        result.I = String(sum.I.toFloat() / count);
        result.VPV = String(sum.VPV.toFloat() / count);
        result.PPV = String(sum.PPV.toFloat() / count);
    }
    return result;
}

// Solar sensor averaging - SLOW_BUFFER_SIZE specializations
template<>
SolarData CircularBuffer<SolarData, SLOW_BUFFER_SIZE>::addWeighted(const SolarData& a, const SolarData& b, float weight) {
    SolarData result = a;
    if (b.valid) {
        result.valid = true;
        // For solar data, we mainly average numeric values
        float aV = a.V.toFloat();
        float bV = b.V.toFloat();
        result.V = String(aV + bV * weight);
        
        float aI = a.I.toFloat();
        float bI = b.I.toFloat();
        result.I = String(aI + bI * weight);
        
        float aVPV = a.VPV.toFloat();
        float bVPV = b.VPV.toFloat();
        result.VPV = String(aVPV + bVPV * weight);
        
        float aPPV = a.PPV.toFloat();
        float bPPV = b.PPV.toFloat();
        result.PPV = String(aPPV + bPPV * weight);
        
        // Keep most recent non-numeric data
        if (b.PID.length() > 0) result.PID = b.PID;
        if (b.FW.length() > 0) result.FW = b.FW;
        if (b.LOAD.length() > 0) result.LOAD = b.LOAD;
    }
    return result;
}

template<>
SolarData CircularBuffer<SolarData, SLOW_BUFFER_SIZE>::addSimple(const SolarData& a, const SolarData& b) {
    SolarData result = a;
    if (b.valid) {
        result.valid = true;
        result.V = String(a.V.toFloat() + b.V.toFloat());
        result.I = String(a.I.toFloat() + b.I.toFloat());
        result.VPV = String(a.VPV.toFloat() + b.VPV.toFloat());
        result.PPV = String(a.PPV.toFloat() + b.PPV.toFloat());
    }
    return result;
}

template<>
SolarData CircularBuffer<SolarData, SLOW_BUFFER_SIZE>::divideByWeight(const SolarData& sum, float weight) {
    SolarData result = sum;
    if (weight > 0) {
        result.V = String(sum.V.toFloat() / weight);
        result.I = String(sum.I.toFloat() / weight);
        result.VPV = String(sum.VPV.toFloat() / weight);
        result.PPV = String(sum.PPV.toFloat() / weight);
    }
    return result;
}

template<>
SolarData CircularBuffer<SolarData, SLOW_BUFFER_SIZE>::divideByCount(const SolarData& sum, size_t count) {
    SolarData result = sum;
    if (count > 0) {
        result.V = String(sum.V.toFloat() / count);
        result.I = String(sum.I.toFloat() / count);
        result.VPV = String(sum.VPV.toFloat() / count);
        result.PPV = String(sum.PPV.toFloat() / count);
    }
    return result;
}

// I2C sensor averaging (temperature, humidity, pressure, CO2)
template<>
I2CSensorData CircularBuffer<I2CSensorData, FAST_BUFFER_SIZE>::addWeighted(const I2CSensorData& a, const I2CSensorData& b, float weight) {
    I2CSensorData result = a;
    if (b.valid) {
        result.valid = true;
        result.temperature += b.temperature * weight;
        result.humidity += b.humidity * weight;
        result.pressure += b.pressure * weight;
        result.co2 += b.co2 * weight;
        result.type = b.type; // Keep latest type
    }
    return result;
}

template<>
I2CSensorData CircularBuffer<I2CSensorData, FAST_BUFFER_SIZE>::addSimple(const I2CSensorData& a, const I2CSensorData& b) {
    I2CSensorData result = a;
    if (b.valid) {
        result.valid = true;
        result.temperature += b.temperature;
        result.humidity += b.humidity;
        result.pressure += b.pressure;
        result.co2 += b.co2;
    }
    return result;
}

template<>
I2CSensorData CircularBuffer<I2CSensorData, FAST_BUFFER_SIZE>::divideByWeight(const I2CSensorData& sum, float weight) {
    I2CSensorData result = sum;
    if (weight > 0) {
        result.temperature /= weight;
        result.humidity /= weight;
        result.pressure /= weight;
        result.co2 /= weight;
    }
    return result;
}

template<>
I2CSensorData CircularBuffer<I2CSensorData, FAST_BUFFER_SIZE>::divideByCount(const I2CSensorData& sum, size_t count) {
    I2CSensorData result = sum;
    if (count > 0) {
        result.temperature /= count;
        result.humidity /= count;
        result.pressure /= count;
        result.co2 /= count;
    }
    return result;
}

// I2C sensor averaging - SLOW_BUFFER_SIZE specializations
template<>
I2CSensorData CircularBuffer<I2CSensorData, SLOW_BUFFER_SIZE>::addWeighted(const I2CSensorData& a, const I2CSensorData& b, float weight) {
    I2CSensorData result = a;
    if (b.valid) {
        result.valid = true;
        result.temperature += b.temperature * weight;
        result.humidity += b.humidity * weight;
        result.pressure += b.pressure * weight;
        result.co2 += b.co2 * weight;
        result.type = b.type; // Keep latest type
    }
    return result;
}

template<>
I2CSensorData CircularBuffer<I2CSensorData, SLOW_BUFFER_SIZE>::addSimple(const I2CSensorData& a, const I2CSensorData& b) {
    I2CSensorData result = a;
    if (b.valid) {
        result.valid = true;
        result.temperature += b.temperature;
        result.humidity += b.humidity;
        result.pressure += b.pressure;
        result.co2 += b.co2;
    }
    return result;
}

template<>
I2CSensorData CircularBuffer<I2CSensorData, SLOW_BUFFER_SIZE>::divideByWeight(const I2CSensorData& sum, float weight) {
    I2CSensorData result = sum;
    if (weight > 0) {
        result.temperature /= weight;
        result.humidity /= weight;
        result.pressure /= weight;
        result.co2 /= weight;
    }
    return result;
}

template<>
I2CSensorData CircularBuffer<I2CSensorData, SLOW_BUFFER_SIZE>::divideByCount(const I2CSensorData& sum, size_t count) {
    I2CSensorData result = sum;
    if (count > 0) {
        result.temperature /= count;
        result.humidity /= count;
        result.pressure /= count;
        result.co2 /= count;
    }
    return result;
}

// SPS30 sensor averaging
template<>
SPS30Data CircularBuffer<SPS30Data, FAST_BUFFER_SIZE>::addWeighted(const SPS30Data& a, const SPS30Data& b, float weight) {
    SPS30Data result = a;
    if (b.valid) {
        result.valid = true;
        result.pm1_0 += b.pm1_0 * weight;
        result.pm2_5 += b.pm2_5 * weight;
        result.pm4_0 += b.pm4_0 * weight;
        result.pm10 += b.pm10 * weight;
        result.nc0_5 += b.nc0_5 * weight;
        result.nc1_0 += b.nc1_0 * weight;
        result.nc2_5 += b.nc2_5 * weight;
        result.nc4_0 += b.nc4_0 * weight;
        result.nc10 += b.nc10 * weight;
        result.typical_particle_size += b.typical_particle_size * weight;
    }
    return result;
}

template<>
SPS30Data CircularBuffer<SPS30Data, FAST_BUFFER_SIZE>::addSimple(const SPS30Data& a, const SPS30Data& b) {
    SPS30Data result = a;
    if (b.valid) {
        result.valid = true;
        result.pm1_0 += b.pm1_0;
        result.pm2_5 += b.pm2_5;
        result.pm4_0 += b.pm4_0;
        result.pm10 += b.pm10;
        result.nc0_5 += b.nc0_5;
        result.nc1_0 += b.nc1_0;
        result.nc2_5 += b.nc2_5;
        result.nc4_0 += b.nc4_0;
        result.nc10 += b.nc10;
        result.typical_particle_size += b.typical_particle_size;
    }
    return result;
}

template<>
SPS30Data CircularBuffer<SPS30Data, FAST_BUFFER_SIZE>::divideByWeight(const SPS30Data& sum, float weight) {
    SPS30Data result = sum;
    if (weight > 0) {
        result.pm1_0 /= weight;
        result.pm2_5 /= weight;
        result.pm4_0 /= weight;
        result.pm10 /= weight;
        result.nc0_5 /= weight;
        result.nc1_0 /= weight;
        result.nc2_5 /= weight;
        result.nc4_0 /= weight;
        result.nc10 /= weight;
        result.typical_particle_size /= weight;
    }
    return result;
}

template<>
SPS30Data CircularBuffer<SPS30Data, FAST_BUFFER_SIZE>::divideByCount(const SPS30Data& sum, size_t count) {
    SPS30Data result = sum;
    if (count > 0) {
        result.pm1_0 /= count;
        result.pm2_5 /= count;
        result.pm4_0 /= count;
        result.pm10 /= count;
        result.nc0_5 /= count;
        result.nc1_0 /= count;
        result.nc2_5 /= count;
        result.nc4_0 /= count;
        result.nc10 /= count;
        result.typical_particle_size /= count;
    }
    return result;
}

// SPS30 sensor averaging - SLOW_BUFFER_SIZE specializations
template<>
SPS30Data CircularBuffer<SPS30Data, SLOW_BUFFER_SIZE>::addWeighted(const SPS30Data& a, const SPS30Data& b, float weight) {
    SPS30Data result = a;
    if (b.valid) {
        result.valid = true;
        result.pm1_0 += b.pm1_0 * weight;
        result.pm2_5 += b.pm2_5 * weight;
        result.pm4_0 += b.pm4_0 * weight;
        result.pm10 += b.pm10 * weight;
        result.nc0_5 += b.nc0_5 * weight;
        result.nc1_0 += b.nc1_0 * weight;
        result.nc2_5 += b.nc2_5 * weight;
        result.nc4_0 += b.nc4_0 * weight;
        result.nc10 += b.nc10 * weight;
        result.typical_particle_size += b.typical_particle_size * weight;
    }
    return result;
}

template<>
SPS30Data CircularBuffer<SPS30Data, SLOW_BUFFER_SIZE>::addSimple(const SPS30Data& a, const SPS30Data& b) {
    SPS30Data result = a;
    if (b.valid) {
        result.valid = true;
        result.pm1_0 += b.pm1_0;
        result.pm2_5 += b.pm2_5;
        result.pm4_0 += b.pm4_0;
        result.pm10 += b.pm10;
        result.nc0_5 += b.nc0_5;
        result.nc1_0 += b.nc1_0;
        result.nc2_5 += b.nc2_5;
        result.nc4_0 += b.nc4_0;
        result.nc10 += b.nc10;
        result.typical_particle_size += b.typical_particle_size;
    }
    return result;
}

template<>
SPS30Data CircularBuffer<SPS30Data, SLOW_BUFFER_SIZE>::divideByWeight(const SPS30Data& sum, float weight) {
    SPS30Data result = sum;
    if (weight > 0) {
        result.pm1_0 /= weight;
        result.pm2_5 /= weight;
        result.pm4_0 /= weight;
        result.pm10 /= weight;
        result.nc0_5 /= weight;
        result.nc1_0 /= weight;
        result.nc2_5 /= weight;
        result.nc4_0 /= weight;
        result.nc10 /= weight;
        result.typical_particle_size /= weight;
    }
    return result;
}

template<>
SPS30Data CircularBuffer<SPS30Data, SLOW_BUFFER_SIZE>::divideByCount(const SPS30Data& sum, size_t count) {
    SPS30Data result = sum;
    if (count > 0) {
        result.pm1_0 /= count;
        result.pm2_5 /= count;
        result.pm4_0 /= count;
        result.pm10 /= count;
        result.nc0_5 /= count;
        result.nc1_0 /= count;
        result.nc2_5 /= count;
        result.nc4_0 /= count;
        result.nc10 /= count;
        result.typical_particle_size /= count;
    }
    return result;
}

// IPS sensor averaging
template<>
IPSSensorData CircularBuffer<IPSSensorData, FAST_BUFFER_SIZE>::addWeighted(const IPSSensorData& a, const IPSSensorData& b, float weight) {
    IPSSensorData result = a;
    if (b.valid) {
        result.valid = true;
        for (int i = 0; i < 7; i++) {
            result.pc_values[i] += (unsigned long)(b.pc_values[i] * weight);
            result.pm_values[i] += b.pm_values[i] * weight;
            if (b.debugMode) {
                result.np_values[i] += (unsigned long)(b.np_values[i] * weight);
                result.pw_values[i] += (unsigned long)(b.pw_values[i] * weight);
            }
        }
        result.debugMode = b.debugMode;
        result.won = b.won;
    }
    return result;
}

template<>
IPSSensorData CircularBuffer<IPSSensorData, FAST_BUFFER_SIZE>::addSimple(const IPSSensorData& a, const IPSSensorData& b) {
    IPSSensorData result = a;
    if (b.valid) {
        result.valid = true;
        for (int i = 0; i < 7; i++) {
            result.pc_values[i] += b.pc_values[i];
            result.pm_values[i] += b.pm_values[i];
            if (b.debugMode) {
                result.np_values[i] += b.np_values[i];
                result.pw_values[i] += b.pw_values[i];
            }
        }
    }
    return result;
}

template<>
IPSSensorData CircularBuffer<IPSSensorData, FAST_BUFFER_SIZE>::divideByWeight(const IPSSensorData& sum, float weight) {
    IPSSensorData result = sum;
    if (weight > 0) {
        for (int i = 0; i < 7; i++) {
            result.pc_values[i] = (unsigned long)(sum.pc_values[i] / weight);
            result.pm_values[i] /= weight;
            if (sum.debugMode) {
                result.np_values[i] = (unsigned long)(sum.np_values[i] / weight);
                result.pw_values[i] = (unsigned long)(sum.pw_values[i] / weight);
            }
        }
    }
    return result;
}

template<>
IPSSensorData CircularBuffer<IPSSensorData, FAST_BUFFER_SIZE>::divideByCount(const IPSSensorData& sum, size_t count) {
    IPSSensorData result = sum;
    if (count > 0) {
        for (int i = 0; i < 7; i++) {
            result.pc_values[i] /= count;
            result.pm_values[i] /= count;
            if (sum.debugMode) {
                result.np_values[i] /= count;
                result.pw_values[i] /= count;
            }
        }
    }
    return result;
}

// IPS sensor averaging - SLOW_BUFFER_SIZE specializations
template<>
IPSSensorData CircularBuffer<IPSSensorData, SLOW_BUFFER_SIZE>::addWeighted(const IPSSensorData& a, const IPSSensorData& b, float weight) {
    IPSSensorData result = a;
    if (b.valid) {
        result.valid = true;
        for (int i = 0; i < 7; i++) {
            result.pc_values[i] += (unsigned long)(b.pc_values[i] * weight);
            result.pm_values[i] += b.pm_values[i] * weight;
            if (b.debugMode) {
                result.np_values[i] += (unsigned long)(b.np_values[i] * weight);
                result.pw_values[i] += (unsigned long)(b.pw_values[i] * weight);
            }
        }
        result.debugMode = b.debugMode;
        result.won = b.won;
    }
    return result;
}

template<>
IPSSensorData CircularBuffer<IPSSensorData, SLOW_BUFFER_SIZE>::addSimple(const IPSSensorData& a, const IPSSensorData& b) {
    IPSSensorData result = a;
    if (b.valid) {
        result.valid = true;
        for (int i = 0; i < 7; i++) {
            result.pc_values[i] += b.pc_values[i];
            result.pm_values[i] += b.pm_values[i];
            if (b.debugMode) {
                result.np_values[i] += b.np_values[i];
                result.pw_values[i] += b.pw_values[i];
            }
        }
    }
    return result;
}

template<>
IPSSensorData CircularBuffer<IPSSensorData, SLOW_BUFFER_SIZE>::divideByWeight(const IPSSensorData& sum, float weight) {
    IPSSensorData result = sum;
    if (weight > 0) {
        for (int i = 0; i < 7; i++) {
            result.pc_values[i] = (unsigned long)(sum.pc_values[i] / weight);
            result.pm_values[i] /= weight;
            if (sum.debugMode) {
                result.np_values[i] = (unsigned long)(sum.np_values[i] / weight);
                result.pw_values[i] = (unsigned long)(sum.pw_values[i] / weight);
            }
        }
    }
    return result;
}

template<>
IPSSensorData CircularBuffer<IPSSensorData, SLOW_BUFFER_SIZE>::divideByCount(const IPSSensorData& sum, size_t count) {
    IPSSensorData result = sum;
    if (count > 0) {
        for (int i = 0; i < 7; i++) {
            result.pc_values[i] /= count;
            result.pm_values[i] /= count;
            if (sum.debugMode) {
                result.np_values[i] /= count;
                result.pw_values[i] /= count;
            }
        }
    }
    return result;
}

// MCP3424 ADC averaging
template<>
MCP3424Data CircularBuffer<MCP3424Data, FAST_BUFFER_SIZE>::addWeighted(const MCP3424Data& a, const MCP3424Data& b, float weight) {
    MCP3424Data result = a;
    if (b.deviceCount > 0) {
        result.deviceCount = b.deviceCount;
        result.resolution = b.resolution;
        result.gain = b.gain;
        for (uint8_t dev = 0; dev < b.deviceCount && dev < MAX_MCP3424_DEVICES; dev++) {
            result.addresses[dev] = b.addresses[dev];
            if (b.valid[dev]) {
                result.valid[dev] = true;
                for (int ch = 0; ch < 4; ch++) {
                    result.channels[dev][ch] += b.channels[dev][ch] * weight;
                }
            }
        }
    }
    return result;
}

template<>
MCP3424Data CircularBuffer<MCP3424Data, FAST_BUFFER_SIZE>::addSimple(const MCP3424Data& a, const MCP3424Data& b) {
    MCP3424Data result = a;
    if (b.deviceCount > 0) {
        for (uint8_t dev = 0; dev < b.deviceCount && dev < MAX_MCP3424_DEVICES; dev++) {
            if (b.valid[dev]) {
                result.valid[dev] = true;
                for (int ch = 0; ch < 4; ch++) {
                    result.channels[dev][ch] += b.channels[dev][ch];
                }
            }
        }
    }
    return result;
}

template<>
MCP3424Data CircularBuffer<MCP3424Data, FAST_BUFFER_SIZE>::divideByWeight(const MCP3424Data& sum, float weight) {
    MCP3424Data result = sum;
    if (weight > 0) {
        for (uint8_t dev = 0; dev < result.deviceCount && dev < MAX_MCP3424_DEVICES; dev++) {
            if (result.valid[dev]) {
                for (int ch = 0; ch < 4; ch++) {
                    result.channels[dev][ch] /= weight;
                }
            }
        }
    }
    return result;
}

template<>
MCP3424Data CircularBuffer<MCP3424Data, FAST_BUFFER_SIZE>::divideByCount(const MCP3424Data& sum, size_t count) {
    MCP3424Data result = sum;
    if (count > 0) {
        for (uint8_t dev = 0; dev < result.deviceCount && dev < MAX_MCP3424_DEVICES; dev++) {
            if (result.valid[dev]) {
                for (int ch = 0; ch < 4; ch++) {
                    result.channels[dev][ch] /= count;
                }
            }
        }
    }
    return result;
}

// MCP3424 ADC averaging - SLOW_BUFFER_SIZE specializations
template<>
MCP3424Data CircularBuffer<MCP3424Data, SLOW_BUFFER_SIZE>::addWeighted(const MCP3424Data& a, const MCP3424Data& b, float weight) {
    MCP3424Data result = a;
    if (b.deviceCount > 0) {
        result.deviceCount = b.deviceCount;
        result.resolution = b.resolution;
        result.gain = b.gain;
        for (uint8_t dev = 0; dev < b.deviceCount && dev < MAX_MCP3424_DEVICES; dev++) {
            result.addresses[dev] = b.addresses[dev];
            if (b.valid[dev]) {
                result.valid[dev] = true;
                for (int ch = 0; ch < 4; ch++) {
                    result.channels[dev][ch] += b.channels[dev][ch] * weight;
                }
            }
        }
    }
    return result;
}

template<>
MCP3424Data CircularBuffer<MCP3424Data, SLOW_BUFFER_SIZE>::addSimple(const MCP3424Data& a, const MCP3424Data& b) {
    MCP3424Data result = a;
    if (b.deviceCount > 0) {
        for (uint8_t dev = 0; dev < b.deviceCount && dev < MAX_MCP3424_DEVICES; dev++) {
            if (b.valid[dev]) {
                result.valid[dev] = true;
                for (int ch = 0; ch < 4; ch++) {
                    result.channels[dev][ch] += b.channels[dev][ch];
                }
            }
        }
    }
    return result;
}

template<>
MCP3424Data CircularBuffer<MCP3424Data, SLOW_BUFFER_SIZE>::divideByWeight(const MCP3424Data& sum, float weight) {
    MCP3424Data result = sum;
    if (weight > 0) {
        for (uint8_t dev = 0; dev < result.deviceCount && dev < MAX_MCP3424_DEVICES; dev++) {
            if (result.valid[dev]) {
                for (int ch = 0; ch < 4; ch++) {
                    result.channels[dev][ch] /= weight;
                }
            }
        }
    }
    return result;
}

template<>
MCP3424Data CircularBuffer<MCP3424Data, SLOW_BUFFER_SIZE>::divideByCount(const MCP3424Data& sum, size_t count) {
    MCP3424Data result = sum;
    if (count > 0) {
        for (uint8_t dev = 0; dev < result.deviceCount && dev < MAX_MCP3424_DEVICES; dev++) {
            if (result.valid[dev]) {
                for (int ch = 0; ch < 4; ch++) {
                    result.channels[dev][ch] /= count;
                }
            }
        }
    }
    return result;
}

// ADS1110 averaging
template<>
ADS1110Data CircularBuffer<ADS1110Data, FAST_BUFFER_SIZE>::addWeighted(const ADS1110Data& a, const ADS1110Data& b, float weight) {
    ADS1110Data result = a;
    if (b.valid) {
        result.valid = true;
        result.voltage += b.voltage * weight;
        result.dataRate = b.dataRate;
        result.gain = b.gain;
    }
    return result;
}

template<>
ADS1110Data CircularBuffer<ADS1110Data, FAST_BUFFER_SIZE>::addSimple(const ADS1110Data& a, const ADS1110Data& b) {
    ADS1110Data result = a;
    if (b.valid) {
        result.valid = true;
        result.voltage += b.voltage;
    }
    return result;
}

template<>
ADS1110Data CircularBuffer<ADS1110Data, FAST_BUFFER_SIZE>::divideByWeight(const ADS1110Data& sum, float weight) {
    ADS1110Data result = sum;
    if (weight > 0) {
        result.voltage /= weight;
    }
    return result;
}

template<>
ADS1110Data CircularBuffer<ADS1110Data, FAST_BUFFER_SIZE>::divideByCount(const ADS1110Data& sum, size_t count) {
    ADS1110Data result = sum;
    if (count > 0) {
        result.voltage /= count;
    }
    return result;
}

// ADS1110 averaging - SLOW_BUFFER_SIZE specializations
template<>
ADS1110Data CircularBuffer<ADS1110Data, SLOW_BUFFER_SIZE>::addWeighted(const ADS1110Data& a, const ADS1110Data& b, float weight) {
    ADS1110Data result = a;
    if (b.valid) {
        result.valid = true;
        result.voltage += b.voltage * weight;
        result.dataRate = b.dataRate;
        result.gain = b.gain;
    }
    return result;
}

template<>
ADS1110Data CircularBuffer<ADS1110Data, SLOW_BUFFER_SIZE>::addSimple(const ADS1110Data& a, const ADS1110Data& b) {
    ADS1110Data result = a;
    if (b.valid) {
        result.valid = true;
        result.voltage += b.voltage;
    }
    return result;
}

template<>
ADS1110Data CircularBuffer<ADS1110Data, SLOW_BUFFER_SIZE>::divideByWeight(const ADS1110Data& sum, float weight) {
    ADS1110Data result = sum;
    if (weight > 0) {
        result.voltage /= weight;
    }
    return result;
}

template<>
ADS1110Data CircularBuffer<ADS1110Data, SLOW_BUFFER_SIZE>::divideByCount(const ADS1110Data& sum, size_t count) {
    ADS1110Data result = sum;
    if (count > 0) {
        result.voltage /= count;
    }
    return result;
}

// INA219 averaging
template<>
INA219Data CircularBuffer<INA219Data, FAST_BUFFER_SIZE>::addWeighted(const INA219Data& a, const INA219Data& b, float weight) {
    INA219Data result = a;
    if (b.valid) {
        result.valid = true;
        result.busVoltage += b.busVoltage * weight;
        result.current += b.current * weight;
        result.power += b.power * weight;
        result.shuntVoltage += b.shuntVoltage * weight;
    }
    return result;
}

template<>
INA219Data CircularBuffer<INA219Data, FAST_BUFFER_SIZE>::addSimple(const INA219Data& a, const INA219Data& b) {
    INA219Data result = a;
    if (b.valid) {
        result.valid = true;
        result.busVoltage += b.busVoltage;
        result.current += b.current;
        result.power += b.power;
        result.shuntVoltage += b.shuntVoltage;
    }
    return result;
}

template<>
INA219Data CircularBuffer<INA219Data, FAST_BUFFER_SIZE>::divideByWeight(const INA219Data& sum, float weight) {
    INA219Data result = sum;
    if (weight > 0) {
        result.busVoltage /= weight;
        result.current /= weight;
        result.power /= weight;
        result.shuntVoltage /= weight;
    }
    return result;
}

template<>
INA219Data CircularBuffer<INA219Data, FAST_BUFFER_SIZE>::divideByCount(const INA219Data& sum, size_t count) {
    INA219Data result = sum;
    if (count > 0) {
        result.busVoltage /= count;
        result.current /= count;
        result.power /= count;
        result.shuntVoltage /= count;
    }
    return result;
}

// INA219 averaging - SLOW_BUFFER_SIZE specializations
template<>
INA219Data CircularBuffer<INA219Data, SLOW_BUFFER_SIZE>::addWeighted(const INA219Data& a, const INA219Data& b, float weight) {
    INA219Data result = a;
    if (b.valid) {
        result.valid = true;
        result.busVoltage += b.busVoltage * weight;
        result.current += b.current * weight;
        result.power += b.power * weight;
        result.shuntVoltage += b.shuntVoltage * weight;
    }
    return result;
}

template<>
INA219Data CircularBuffer<INA219Data, SLOW_BUFFER_SIZE>::addSimple(const INA219Data& a, const INA219Data& b) {
    INA219Data result = a;
    if (b.valid) {
        result.valid = true;
        result.busVoltage += b.busVoltage;
        result.current += b.current;
        result.power += b.power;
        result.shuntVoltage += b.shuntVoltage;
    }
    return result;
}

template<>
INA219Data CircularBuffer<INA219Data, SLOW_BUFFER_SIZE>::divideByWeight(const INA219Data& sum, float weight) {
    INA219Data result = sum;
    if (weight > 0) {
        result.busVoltage /= weight;
        result.current /= weight;
        result.power /= weight;
        result.shuntVoltage /= weight;
    }
    return result;
}

template<>
INA219Data CircularBuffer<INA219Data, SLOW_BUFFER_SIZE>::divideByCount(const INA219Data& sum, size_t count) {
    INA219Data result = sum;
    if (count > 0) {
        result.busVoltage /= count;
        result.current /= count;
        result.power /= count;
        result.shuntVoltage /= count;
    }
    return result;
}

// SHT40 averaging
template<>
SHT40Data CircularBuffer<SHT40Data, FAST_BUFFER_SIZE>::addWeighted(const SHT40Data& a, const SHT40Data& b, float weight) {
    SHT40Data result = a;
    if (b.valid) {
        result.valid = true;
        result.temperature += b.temperature * weight;
        result.humidity += b.humidity * weight;
        result.pressure += b.pressure * weight;
    }
    return result;
}

template<>
SHT40Data CircularBuffer<SHT40Data, FAST_BUFFER_SIZE>::addSimple(const SHT40Data& a, const SHT40Data& b) {
    SHT40Data result = a;
    if (b.valid) {
        result.valid = true;
        result.temperature += b.temperature;
        result.humidity += b.humidity;
        result.pressure += b.pressure;
    }
    return result;
}

template<>
SHT40Data CircularBuffer<SHT40Data, FAST_BUFFER_SIZE>::divideByWeight(const SHT40Data& sum, float weight) {
    SHT40Data result = sum;
    if (weight > 0) {
        result.temperature /= weight;
        result.humidity /= weight;
        result.pressure /= weight;
    }
    return result;
}

template<>
SHT40Data CircularBuffer<SHT40Data, FAST_BUFFER_SIZE>::divideByCount(const SHT40Data& sum, size_t count) {
    SHT40Data result = sum;
    if (count > 0) {
        result.temperature /= count;
        result.humidity /= count;
        result.pressure /= count;
    }
    return result;
}

// SHT40 averaging - SLOW_BUFFER_SIZE specializations
template<>
SHT40Data CircularBuffer<SHT40Data, SLOW_BUFFER_SIZE>::addWeighted(const SHT40Data& a, const SHT40Data& b, float weight) {
    SHT40Data result = a;
    if (b.valid) {
        result.valid = true;
        result.temperature += b.temperature * weight;
        result.humidity += b.humidity * weight;
        result.pressure += b.pressure * weight;
    }
    return result;
}

template<>
SHT40Data CircularBuffer<SHT40Data, SLOW_BUFFER_SIZE>::addSimple(const SHT40Data& a, const SHT40Data& b) {
    SHT40Data result = a;
    if (b.valid) {
        result.valid = true;
        result.temperature += b.temperature;
        result.humidity += b.humidity;
        result.pressure += b.pressure;
    }
    return result;
}

template<>
SHT40Data CircularBuffer<SHT40Data, SLOW_BUFFER_SIZE>::divideByWeight(const SHT40Data& sum, float weight) {
    SHT40Data result = sum;
    if (weight > 0) {
        result.temperature /= weight;
        result.humidity /= weight;
        result.pressure /= weight;
    }
    return result;
}

template<>
SHT40Data CircularBuffer<SHT40Data, SLOW_BUFFER_SIZE>::divideByCount(const SHT40Data& sum, size_t count) {
    SHT40Data result = sum;
    if (count > 0) {
        result.temperature /= count;
        result.humidity /= count;
        result.pressure /= count;
    }
    return result;
}

// CalibratedSensorData averaging - FAST_BUFFER_SIZE specializations
template<>
CalibratedSensorData CircularBuffer<CalibratedSensorData, FAST_BUFFER_SIZE>::addWeighted(const CalibratedSensorData& a, const CalibratedSensorData& b, float weight) {
    CalibratedSensorData result = a;
    if (b.valid) {
        result.valid = true;
        
        // Temperatury B4
        result.K1_temp += b.K1_temp * weight;
        result.K2_temp += b.K2_temp * weight;
        result.K3_temp += b.K3_temp * weight;
        result.K4_temp += b.K4_temp * weight;
        result.K5_temp += b.K5_temp * weight;
        
        // Napiecia B4  
        result.K1_voltage += b.K1_voltage * weight;
        result.K2_voltage += b.K2_voltage * weight;
        result.K3_voltage += b.K3_voltage * weight;
        result.K4_voltage += b.K4_voltage * weight;
        result.K5_voltage += b.K5_voltage * weight;
        
        // Temperatury TGS
        result.K6_temp += b.K6_temp * weight;
        result.K7_temp += b.K7_temp * weight;
        result.K8_temp += b.K8_temp * weight;
        result.K9_temp += b.K9_temp * weight;
        result.K12_temp += b.K12_temp * weight;
        
        // Napiecia TGS
        result.K6_voltage += b.K6_voltage * weight;
        result.K7_voltage += b.K7_voltage * weight;
        result.K8_voltage += b.K8_voltage * weight;
        result.K9_voltage += b.K9_voltage * weight;
        result.K12_voltage += b.K12_voltage * weight;
        
        // Gazy ug/m3
        result.CO += b.CO * weight;
        result.NO += b.NO * weight;
        result.NO2 += b.NO2 * weight;
        result.O3 += b.O3 * weight;
        result.SO2 += b.SO2 * weight;
        result.H2S += b.H2S * weight;
        result.NH3 += b.NH3 * weight;
        
        // Gazy ppb
        result.CO_ppb += b.CO_ppb * weight;
        result.NO_ppb += b.NO_ppb * weight;
        result.NO2_ppb += b.NO2_ppb * weight;
        result.O3_ppb += b.O3_ppb * weight;
        result.SO2_ppb += b.SO2_ppb * weight;
        result.H2S_ppb += b.H2S_ppb * weight;
        result.NH3_ppb += b.NH3_ppb * weight;
        
        // TGS sensors
        result.TGS02 += b.TGS02 * weight;
        result.TGS03 += b.TGS03 * weight;
        result.TGS12 += b.TGS12 * weight;
        result.TGS02_ohm += b.TGS02_ohm * weight;
        result.TGS03_ohm += b.TGS03_ohm * weight;
        result.TGS12_ohm += b.TGS12_ohm * weight;
        
        // HCHO i PID
        result.HCHO += b.HCHO * weight;
        result.PID += b.PID * weight;
        result.PID_mV += b.PID_mV * weight;
        
        // VOC
        result.VOC += b.VOC * weight;
        result.VOC_ppb += b.VOC_ppb * weight;
    }
    return result;
}

template<>
CalibratedSensorData CircularBuffer<CalibratedSensorData, FAST_BUFFER_SIZE>::addSimple(const CalibratedSensorData& a, const CalibratedSensorData& b) {
    CalibratedSensorData result = a;
    if (b.valid) {
        result.valid = true;
        
        // Temperatury B4
        result.K1_temp += b.K1_temp;
        result.K2_temp += b.K2_temp;
        result.K3_temp += b.K3_temp;
        result.K4_temp += b.K4_temp;
        result.K5_temp += b.K5_temp;
        
        // Napiecia B4
        result.K1_voltage += b.K1_voltage;
        result.K2_voltage += b.K2_voltage;
        result.K3_voltage += b.K3_voltage;
        result.K4_voltage += b.K4_voltage;
        result.K5_voltage += b.K5_voltage;
        
        // Temperatury TGS
        result.K6_temp += b.K6_temp;
        result.K7_temp += b.K7_temp;
        result.K8_temp += b.K8_temp;
        result.K9_temp += b.K9_temp;
        result.K12_temp += b.K12_temp;
        
        // Napiecia TGS
        result.K6_voltage += b.K6_voltage;
        result.K7_voltage += b.K7_voltage;
        result.K8_voltage += b.K8_voltage;
        result.K9_voltage += b.K9_voltage;
        result.K12_voltage += b.K12_voltage;
        
        // Gazy ug/m3
        result.CO += b.CO;
        result.NO += b.NO;
        result.NO2 += b.NO2;
        result.O3 += b.O3;
        result.SO2 += b.SO2;
        result.H2S += b.H2S;
        result.NH3 += b.NH3;
        
        // Gazy ppb
        result.CO_ppb += b.CO_ppb;
        result.NO_ppb += b.NO_ppb;
        result.NO2_ppb += b.NO2_ppb;
        result.O3_ppb += b.O3_ppb;
        result.SO2_ppb += b.SO2_ppb;
        result.H2S_ppb += b.H2S_ppb;
        result.NH3_ppb += b.NH3_ppb;
        
        // TGS sensors
        result.TGS02 += b.TGS02;
        result.TGS03 += b.TGS03;
        result.TGS12 += b.TGS12;
        result.TGS02_ohm += b.TGS02_ohm;
        result.TGS03_ohm += b.TGS03_ohm;
        result.TGS12_ohm += b.TGS12_ohm;
        
        // HCHO i PID
        result.HCHO += b.HCHO;
        result.PID += b.PID;
        result.PID_mV += b.PID_mV;
        
        // VOC
        result.VOC += b.VOC;
        result.VOC_ppb += b.VOC_ppb;
    }
    return result;
}

template<>
CalibratedSensorData CircularBuffer<CalibratedSensorData, FAST_BUFFER_SIZE>::divideByWeight(const CalibratedSensorData& sum, float weight) {
    CalibratedSensorData result = sum;
    if (weight > 0) {
        // Temperatury B4
        result.K1_temp /= weight;
        result.K2_temp /= weight;
        result.K3_temp /= weight;
        result.K4_temp /= weight;
        result.K5_temp /= weight;
        
        // Napiecia B4
        result.K1_voltage /= weight;
        result.K2_voltage /= weight;
        result.K3_voltage /= weight;
        result.K4_voltage /= weight;
        result.K5_voltage /= weight;
        
        // Temperatury TGS
        result.K6_temp /= weight;
        result.K7_temp /= weight;
        result.K8_temp /= weight;
        result.K9_temp /= weight;
        result.K12_temp /= weight;
        
        // Napiecia TGS
        result.K6_voltage /= weight;
        result.K7_voltage /= weight;
        result.K8_voltage /= weight;
        result.K9_voltage /= weight;
        result.K12_voltage /= weight;
        
        // Gazy ug/m3
        result.CO /= weight;
        result.NO /= weight;
        result.NO2 /= weight;
        result.O3 /= weight;
        result.SO2 /= weight;
        result.H2S /= weight;
        result.NH3 /= weight;
        
        // Gazy ppb
        result.CO_ppb /= weight;
        result.NO_ppb /= weight;
        result.NO2_ppb /= weight;
        result.O3_ppb /= weight;
        result.SO2_ppb /= weight;
        result.H2S_ppb /= weight;
        result.NH3_ppb /= weight;
        
        // TGS sensors
        result.TGS02 /= weight;
        result.TGS03 /= weight;
        result.TGS12 /= weight;
        result.TGS02_ohm /= weight;
        result.TGS03_ohm /= weight;
        result.TGS12_ohm /= weight;
        
        // HCHO i PID
        result.HCHO /= weight;
        result.PID /= weight;
        result.PID_mV /= weight;
        
        // VOC
        result.VOC /= weight;
        result.VOC_ppb /= weight;
    }
    return result;
}

template<>
CalibratedSensorData CircularBuffer<CalibratedSensorData, FAST_BUFFER_SIZE>::divideByCount(const CalibratedSensorData& sum, size_t count) {
    CalibratedSensorData result = sum;
    if (count > 0) {
        // Temperatury B4
        result.K1_temp /= count;
        result.K2_temp /= count;
        result.K3_temp /= count;
        result.K4_temp /= count;
        result.K5_temp /= count;
        
        // Napiecia B4
        result.K1_voltage /= count;
        result.K2_voltage /= count;
        result.K3_voltage /= count;
        result.K4_voltage /= count;
        result.K5_voltage /= count;
        
        // Temperatury TGS
        result.K6_temp /= count;
        result.K7_temp /= count;
        result.K8_temp /= count;
        result.K9_temp /= count;
        result.K12_temp /= count;
        
        // Napiecia TGS
        result.K6_voltage /= count;
        result.K7_voltage /= count;
        result.K8_voltage /= count;
        result.K9_voltage /= count;
        result.K12_voltage /= count;
        
        // Gazy ug/m3
        result.CO /= count;
        result.NO /= count;
        result.NO2 /= count;
        result.O3 /= count;
        result.SO2 /= count;
        result.H2S /= count;
        result.NH3 /= count;
        
        // Gazy ppb
        result.CO_ppb /= count;
        result.NO_ppb /= count;
        result.NO2_ppb /= count;
        result.O3_ppb /= count;
        result.SO2_ppb /= count;
        result.H2S_ppb /= count;
        result.NH3_ppb /= count;
        
        // TGS sensors
        result.TGS02 /= count;
        result.TGS03 /= count;
        result.TGS12 /= count;
        result.TGS02_ohm /= count;
        result.TGS03_ohm /= count;
        result.TGS12_ohm /= count;
        
        // HCHO i PID
        result.HCHO /= count;
        result.PID /= count;
        result.PID_mV /= count;
        
        // VOC
        result.VOC /= count;
        result.VOC_ppb /= count;
    }
    return result;
}

// CalibratedSensorData specializations for SLOW_BUFFER_SIZE
template<>
CalibratedSensorData CircularBuffer<CalibratedSensorData, SLOW_BUFFER_SIZE>::addWeighted(const CalibratedSensorData& a, const CalibratedSensorData& b, float weight) {
    CalibratedSensorData result = a;
    if (b.valid) {
        result.valid = true;
        
        // Elektrochemiczne czujniki gazow
        result.CO += b.CO * weight;
        result.NO += b.NO * weight;
        result.NO2 += b.NO2 * weight;
        result.O3 += b.O3 * weight;
        result.SO2 += b.SO2 * weight;
        result.H2S += b.H2S * weight;
        result.NH3 += b.NH3 * weight;
        
        // Wartosci ppb
        result.CO_ppb += b.CO_ppb * weight;
        result.NO_ppb += b.NO_ppb * weight;
        result.NO2_ppb += b.NO2_ppb * weight;
        result.O3_ppb += b.O3_ppb * weight;
        result.SO2_ppb += b.SO2_ppb * weight;
        result.H2S_ppb += b.H2S_ppb * weight;
        result.NH3_ppb += b.NH3_ppb * weight;
        
        // TGS czujniki - wartosci ppm i ohmy
        result.TGS02 += b.TGS02 * weight;
        result.TGS03 += b.TGS03 * weight;
        result.TGS12 += b.TGS12 * weight;
        result.TGS02_ohm += b.TGS02_ohm * weight;
        result.TGS03_ohm += b.TGS03_ohm * weight;
        result.TGS12_ohm += b.TGS12_ohm * weight;
        
        // HCHO i PID
        result.HCHO += b.HCHO * weight;
        result.PID += b.PID * weight;
        result.PID_mV += b.PID_mV * weight;
        
        // VOC
        result.VOC += b.VOC * weight;
        result.VOC_ppb += b.VOC_ppb * weight;
        
        // Temperatury i napiecia czujnikow
        result.K1_temp += b.K1_temp * weight;
        result.K2_temp += b.K2_temp * weight;
        result.K3_temp += b.K3_temp * weight;
        result.K4_temp += b.K4_temp * weight;
        result.K5_temp += b.K5_temp * weight;
        result.K6_temp += b.K6_temp * weight;
        result.K7_temp += b.K7_temp * weight;
        result.K8_temp += b.K8_temp * weight;
        result.K9_temp += b.K9_temp * weight;
        result.K12_temp += b.K12_temp * weight;
        
        result.K1_voltage += b.K1_voltage * weight;
        result.K2_voltage += b.K2_voltage * weight;
        result.K3_voltage += b.K3_voltage * weight;
        result.K4_voltage += b.K4_voltage * weight;
        result.K5_voltage += b.K5_voltage * weight;
        result.K6_voltage += b.K6_voltage * weight;
        result.K7_voltage += b.K7_voltage * weight;
        result.K8_voltage += b.K8_voltage * weight;
        result.K9_voltage += b.K9_voltage * weight;
        result.K12_voltage += b.K12_voltage * weight;
    }
    return result;
}

template<>
CalibratedSensorData CircularBuffer<CalibratedSensorData, SLOW_BUFFER_SIZE>::addSimple(const CalibratedSensorData& a, const CalibratedSensorData& b) {
    CalibratedSensorData result = a;
    if (b.valid) {
        result.valid = true;
        result.CO += b.CO;
        result.NO += b.NO;
        result.NO2 += b.NO2;
        result.O3 += b.O3;
        result.SO2 += b.SO2;
        result.H2S += b.H2S;
        result.NH3 += b.NH3;
        result.TGS02 += b.TGS02;
        result.TGS03 += b.TGS03;
        result.TGS12 += b.TGS12;
        result.HCHO += b.HCHO;
        result.PID += b.PID;
        result.VOC += b.VOC;
        result.VOC_ppb += b.VOC_ppb;
    }
    return result;
}

template<>
CalibratedSensorData CircularBuffer<CalibratedSensorData, SLOW_BUFFER_SIZE>::divideByWeight(const CalibratedSensorData& sum, float weight) {
    CalibratedSensorData result = sum;
    if (weight > 0) {
        // Elektrochemiczne czujniki gazow
        result.CO /= weight;
        result.NO /= weight;
        result.NO2 /= weight;
        result.O3 /= weight;
        result.SO2 /= weight;
        result.H2S /= weight;
        result.NH3 /= weight;
        
        // Wartosci ppb
        result.CO_ppb /= weight;
        result.NO_ppb /= weight;
        result.NO2_ppb /= weight;
        result.O3_ppb /= weight;
        result.SO2_ppb /= weight;
        result.H2S_ppb /= weight;
        result.NH3_ppb /= weight;
        
        // TGS czujniki - wartosci ppm i ohmy
        result.TGS02 /= weight;
        result.TGS03 /= weight;
        result.TGS12 /= weight;
        result.TGS02_ohm /= weight;
        result.TGS03_ohm /= weight;
        result.TGS12_ohm /= weight;
        
        // HCHO i PID
        result.HCHO /= weight;
        result.PID /= weight;
        result.PID_mV /= weight;
        
        // VOC
        result.VOC /= weight;
        result.VOC_ppb /= weight;
        
        // Temperatury i napiecia czujnikow
        result.K1_temp /= weight;
        result.K2_temp /= weight;
        result.K3_temp /= weight;
        result.K4_temp /= weight;
        result.K5_temp /= weight;
        result.K6_temp /= weight;
        result.K7_temp /= weight;
        result.K8_temp /= weight;
        result.K9_temp /= weight;
        result.K12_temp /= weight;
        
        result.K1_voltage /= weight;
        result.K2_voltage /= weight;
        result.K3_voltage /= weight;
        result.K4_voltage /= weight;
        result.K5_voltage /= weight;
        result.K6_voltage /= weight;
        result.K7_voltage /= weight;
        result.K8_voltage /= weight;
        result.K9_voltage /= weight;
        result.K12_voltage /= weight;
    }
    return result;
}

template<>
CalibratedSensorData CircularBuffer<CalibratedSensorData, SLOW_BUFFER_SIZE>::divideByCount(const CalibratedSensorData& sum, size_t count) {
    CalibratedSensorData result = sum;
    if (count > 0) {
        // Elektrochemiczne czujniki gazow
        result.CO /= count;
        result.NO /= count;
        result.NO2 /= count;
        result.O3 /= count;
        result.SO2 /= count;
        result.H2S /= count;
        result.NH3 /= count;
        
        // Wartosci ppb
        result.CO_ppb /= count;
        result.NO_ppb /= count;
        result.NO2_ppb /= count;
        result.O3_ppb /= count;
        result.SO2_ppb /= count;
        result.H2S_ppb /= count;
        result.NH3_ppb /= count;
        
        // TGS czujniki - wartosci ppm i ohmy
        result.TGS02 /= count;
        result.TGS03 /= count;
        result.TGS12 /= count;
        result.TGS02_ohm /= count;
        result.TGS03_ohm /= count;
        result.TGS12_ohm /= count;
        
        // HCHO i PID
        result.HCHO /= count;
        result.PID /= count;
        result.PID_mV /= count;
        
        // VOC
        result.VOC /= count;
        result.VOC_ppb /= count;
        
        // Temperatury i napiecia czujnikow
        result.K1_temp /= count;
        result.K2_temp /= count;
        result.K3_temp /= count;
        result.K4_temp /= count;
        result.K5_temp /= count;
        result.K6_temp /= count;
        result.K7_temp /= count;
        result.K8_temp /= count;
        result.K9_temp /= count;
        result.K12_temp /= count;
        
        result.K1_voltage /= count;
        result.K2_voltage /= count;
        result.K3_voltage /= count;
        result.K4_voltage /= count;
        result.K5_voltage /= count;
        result.K6_voltage /= count;
        result.K7_voltage /= count;
        result.K8_voltage /= count;
        result.K9_voltage /= count;
        result.K12_voltage /= count;
    }
    return result;
}

// HCHO sensor averaging - FAST_BUFFER_SIZE specializations
template<>
HCHOData CircularBuffer<HCHOData, FAST_BUFFER_SIZE>::addWeighted(const HCHOData& a, const HCHOData& b, float weight) {
    HCHOData result = a;
    if (b.valid) {
        result.valid = true;
        result.hcho += b.hcho * weight;
        result.hcho_ppb += b.hcho_ppb * weight;
    }
    return result;
}

template<>
HCHOData CircularBuffer<HCHOData, FAST_BUFFER_SIZE>::addSimple(const HCHOData& a, const HCHOData& b) {
    HCHOData result = a;
    if (b.valid) {
        result.valid = true;
        result.hcho += b.hcho;
        result.hcho_ppb += b.hcho_ppb;
    }
    return result;
}

template<>
HCHOData CircularBuffer<HCHOData, FAST_BUFFER_SIZE>::divideByWeight(const HCHOData& sum, float weight) {
    HCHOData result = sum;
    if (weight > 0) {
        result.hcho /= weight;
        result.hcho_ppb /= weight;
    }
    return result;
}

template<>
HCHOData CircularBuffer<HCHOData, FAST_BUFFER_SIZE>::divideByCount(const HCHOData& sum, size_t count) {
    HCHOData result = sum;
    if (count > 0) {
        result.hcho /= count;
        result.hcho_ppb /= count;
    }
    return result;
}

// HCHO sensor averaging - SLOW_BUFFER_SIZE specializations
template<>
HCHOData CircularBuffer<HCHOData, SLOW_BUFFER_SIZE>::addWeighted(const HCHOData& a, const HCHOData& b, float weight) {
    HCHOData result = a;
    if (b.valid) {
        result.valid = true;
        result.hcho += b.hcho * weight;
        result.hcho_ppb += b.hcho_ppb * weight;
    }
    return result;
}

template<>
HCHOData CircularBuffer<HCHOData, SLOW_BUFFER_SIZE>::addSimple(const HCHOData& a, const HCHOData& b) {
    HCHOData result = a;
    if (b.valid) {
        result.valid = true;
        result.hcho += b.hcho;
        result.hcho_ppb += b.hcho_ppb;
    }
    return result;
}

template<>
HCHOData CircularBuffer<HCHOData, SLOW_BUFFER_SIZE>::divideByWeight(const HCHOData& sum, float weight) {
    HCHOData result = sum;
    if (weight > 0) {
        result.hcho /= weight;
        result.hcho_ppb /= weight;
    }
    return result;
}

template<>
HCHOData CircularBuffer<HCHOData, SLOW_BUFFER_SIZE>::divideByCount(const HCHOData& sum, size_t count) {
    HCHOData result = sum;
    if (count > 0) {
        result.hcho /= count;
        result.hcho_ppb /= count;
    }
    return result;
}

// Moving average manager class
class MovingAverageManager {
private:
    // Fast buffers (10 second averages) - conditionally allocated
    CircularBuffer<SolarData, FAST_BUFFER_SIZE>* solarFastBuffer;
    CircularBuffer<I2CSensorData, FAST_BUFFER_SIZE>* i2cFastBuffer;
    CircularBuffer<SPS30Data, FAST_BUFFER_SIZE>* sps30FastBuffer;
    CircularBuffer<IPSSensorData, FAST_BUFFER_SIZE>* ipsFastBuffer;
    CircularBuffer<MCP3424Data, FAST_BUFFER_SIZE>* mcp3424FastBuffer;
    CircularBuffer<ADS1110Data, FAST_BUFFER_SIZE>* ads1110FastBuffer;
    CircularBuffer<INA219Data, FAST_BUFFER_SIZE>* ina219FastBuffer;
    CircularBuffer<SHT40Data, FAST_BUFFER_SIZE>* sht40FastBuffer;
    CircularBuffer<CalibratedSensorData, FAST_BUFFER_SIZE>* calibFastBuffer;
    CircularBuffer<HCHOData, FAST_BUFFER_SIZE>* hchoFastBuffer;
    
    // Slow buffers (5 minute averages) - conditionally allocated
    CircularBuffer<SolarData, SLOW_BUFFER_SIZE>* solarSlowBuffer;
    CircularBuffer<I2CSensorData, SLOW_BUFFER_SIZE>* i2cSlowBuffer;
    CircularBuffer<SPS30Data, SLOW_BUFFER_SIZE>* sps30SlowBuffer;
    CircularBuffer<IPSSensorData, SLOW_BUFFER_SIZE>* ipsSlowBuffer;
    CircularBuffer<MCP3424Data, SLOW_BUFFER_SIZE>* mcp3424SlowBuffer;
    CircularBuffer<ADS1110Data, SLOW_BUFFER_SIZE>* ads1110SlowBuffer;
    CircularBuffer<INA219Data, SLOW_BUFFER_SIZE>* ina219SlowBuffer;
    CircularBuffer<SHT40Data, SLOW_BUFFER_SIZE>* sht40SlowBuffer;
    CircularBuffer<CalibratedSensorData, SLOW_BUFFER_SIZE>* calibSlowBuffer;
    CircularBuffer<HCHOData, SLOW_BUFFER_SIZE>* hchoSlowBuffer;
    
    // Sensor enabled flags (cached from config)
    bool solarEnabled;
    bool i2cEnabled;
    bool sps30Enabled;
    bool ipsEnabled;
    bool mcp3424Enabled;
    bool ads1110Enabled;
    bool ina219Enabled;
    bool sht40Enabled;
    bool calibEnabled;
    bool hchoEnabled;
    
    // Averaged data storage
    SolarData solarFastAvg, solarSlowAvg;
    I2CSensorData i2cFastAvg, i2cSlowAvg;
    SPS30Data sps30FastAvg, sps30SlowAvg;
    IPSSensorData ipsFastAvg, ipsSlowAvg;
    MCP3424Data mcp3424FastAvg, mcp3424SlowAvg;
    ADS1110Data ads1110FastAvg, ads1110SlowAvg;
    INA219Data ina219FastAvg, ina219SlowAvg;
    SHT40Data sht40FastAvg, sht40SlowAvg;
    CalibratedSensorData calibFastAvg, calibSlowAvg;
    HCHOData hchoFastAvg, hchoSlowAvg;
    
    unsigned long lastFastUpdate = 0;
    unsigned long lastSlowUpdate = 0;
    
public:
    MovingAverageManager() {
        // Initialize all pointers to nullptr
        solarFastBuffer = nullptr;
        i2cFastBuffer = nullptr;
        sps30FastBuffer = nullptr;
        ipsFastBuffer = nullptr;
        mcp3424FastBuffer = nullptr;
        ads1110FastBuffer = nullptr;
        ina219FastBuffer = nullptr;
        sht40FastBuffer = nullptr;
        calibFastBuffer = nullptr;
        hchoFastBuffer = nullptr;
        
        solarSlowBuffer = nullptr;
        i2cSlowBuffer = nullptr;
        sps30SlowBuffer = nullptr;
        ipsSlowBuffer = nullptr;
        mcp3424SlowBuffer = nullptr;
        ads1110SlowBuffer = nullptr;
        ina219SlowBuffer = nullptr;
        sht40SlowBuffer = nullptr;
        calibSlowBuffer = nullptr;
        hchoSlowBuffer = nullptr;
        
        // Initialize all flags to false
        solarEnabled = false;
        i2cEnabled = false;
        sps30Enabled = false;
        ipsEnabled = false;
        mcp3424Enabled = false;
        ads1110Enabled = false;
        ina219Enabled = false;
        sht40Enabled = false;
        calibEnabled = false;
        hchoEnabled = false;
    }
    
    ~MovingAverageManager() {
        // Clean up dynamically allocated buffers
        delete solarFastBuffer;
        delete i2cFastBuffer;
        delete sps30FastBuffer;
        delete ipsFastBuffer;
        delete mcp3424FastBuffer;
        delete ads1110FastBuffer;
        delete ina219FastBuffer;
        delete sht40FastBuffer;
        delete calibFastBuffer;
        delete hchoFastBuffer;
        
        delete solarSlowBuffer;
        delete i2cSlowBuffer;
        delete sps30SlowBuffer;
        delete ipsSlowBuffer;
        delete mcp3424SlowBuffer;
        delete ads1110SlowBuffer;
        delete ina219SlowBuffer;
        delete sht40SlowBuffer;
        delete calibSlowBuffer;
        delete hchoSlowBuffer;
    }
    
    void initializeBuffers() {
        extern FeatureConfig config;
        
        // Cache enabled flags from config
        solarEnabled = config.enableSolarSensor;
        i2cEnabled = config.enableI2CSensors;
        sps30Enabled = config.enableSPS30;
        ipsEnabled = config.enableIPS;
        mcp3424Enabled = config.enableMCP3424;
        ads1110Enabled = config.enableADS1110;
        ina219Enabled = config.enableINA219;
        sht40Enabled = config.enableSHT40;
        calibEnabled = calibConfig.enableMovingAverages;
        hchoEnabled = config.enableHCHO;
        
        Serial.println("Initializing moving average buffers for enabled sensors:");
        
        // Allocate buffers only for enabled sensors
        if (solarEnabled) {
            solarFastBuffer = new CircularBuffer<SolarData, FAST_BUFFER_SIZE>();
            solarSlowBuffer = new CircularBuffer<SolarData, SLOW_BUFFER_SIZE>();
            Serial.println("  - Solar buffers allocated");
        }
        
        if (i2cEnabled) {
            i2cFastBuffer = new CircularBuffer<I2CSensorData, FAST_BUFFER_SIZE>();
            i2cSlowBuffer = new CircularBuffer<I2CSensorData, SLOW_BUFFER_SIZE>();
            Serial.println("  - I2C buffers allocated");
        }
        
        if (sps30Enabled) {
            sps30FastBuffer = new CircularBuffer<SPS30Data, FAST_BUFFER_SIZE>();
            sps30SlowBuffer = new CircularBuffer<SPS30Data, SLOW_BUFFER_SIZE>();
            Serial.println("  - SPS30 buffers allocated");
        }
        
        if (ipsEnabled) {
            ipsFastBuffer = new CircularBuffer<IPSSensorData, FAST_BUFFER_SIZE>();
            ipsSlowBuffer = new CircularBuffer<IPSSensorData, SLOW_BUFFER_SIZE>();
            Serial.println("  - IPS buffers allocated");
        }
        
        if (mcp3424Enabled) {
            mcp3424FastBuffer = new CircularBuffer<MCP3424Data, FAST_BUFFER_SIZE>();
            mcp3424SlowBuffer = new CircularBuffer<MCP3424Data, SLOW_BUFFER_SIZE>();
            Serial.println("  - MCP3424 buffers allocated");
        }
        
        if (ads1110Enabled) {
            ads1110FastBuffer = new CircularBuffer<ADS1110Data, FAST_BUFFER_SIZE>();
            ads1110SlowBuffer = new CircularBuffer<ADS1110Data, SLOW_BUFFER_SIZE>();
            Serial.println("  - ADS1110 buffers allocated");
        }
        
        if (ina219Enabled) {
            ina219FastBuffer = new CircularBuffer<INA219Data, FAST_BUFFER_SIZE>();
            ina219SlowBuffer = new CircularBuffer<INA219Data, SLOW_BUFFER_SIZE>();
            Serial.println("  - INA219 buffers allocated");
        }
        
        if (sht40Enabled) {
            sht40FastBuffer = new CircularBuffer<SHT40Data, FAST_BUFFER_SIZE>();
            sht40SlowBuffer = new CircularBuffer<SHT40Data, SLOW_BUFFER_SIZE>();
            Serial.println("  - SHT40 buffers allocated");
        }

        if (calibEnabled) {
            calibFastBuffer = new CircularBuffer<CalibratedSensorData, FAST_BUFFER_SIZE>();
            calibSlowBuffer = new CircularBuffer<CalibratedSensorData, SLOW_BUFFER_SIZE>();
            Serial.println("  - Calibration buffers allocated");
        }
        
        if (hchoEnabled) {
            hchoFastBuffer = new CircularBuffer<HCHOData, FAST_BUFFER_SIZE>();
            hchoSlowBuffer = new CircularBuffer<HCHOData, SLOW_BUFFER_SIZE>();
            Serial.println("  - HCHO buffers allocated");
        }
        
        // Calculate total memory usage estimation
        int enabledSensors = 0;
        if (solarEnabled) enabledSensors++;
        if (i2cEnabled) enabledSensors++;
        if (sps30Enabled) enabledSensors++;
        if (ipsEnabled) enabledSensors++;
        if (mcp3424Enabled) enabledSensors++;
        if (ads1110Enabled) enabledSensors++;
        if (ina219Enabled) enabledSensors++;
        if (sht40Enabled) enabledSensors++;
        if (calibEnabled) enabledSensors++;
        if (hchoEnabled) enabledSensors++;
        
        // Rough estimation: each buffer pair uses ~2KB
        size_t estimatedMemory = enabledSensors * 2048;
        
        Serial.print("Moving averages initialized for ");
        Serial.print(enabledSensors);
        Serial.print(" sensors, estimated memory usage: ~");
        Serial.print(estimatedMemory);
        Serial.println(" bytes");
    }
    
    void updateSensorData() {
        unsigned long currentTime = millis();
        
        // Add new samples to buffers (only for enabled sensors)
        extern SolarData solarData;
        extern I2CSensorData i2cSensorData;
        extern SPS30Data sps30Data;
        extern IPSSensorData ipsSensorData;
        extern MCP3424Data mcp3424Data;
        extern ADS1110Data ads1110Data;
        extern INA219Data ina219Data;
        extern SHT40Data sht40Data;
        extern CalibratedSensorData calibratedData;
        extern HCHOData hchoData;
        
        if (solarEnabled && solarFastBuffer && solarSlowBuffer && solarData.valid) {
            solarFastBuffer->push(solarData, currentTime);
            solarSlowBuffer->push(solarData, currentTime);
        }
        
        if (i2cEnabled && i2cFastBuffer && i2cSlowBuffer && i2cSensorData.valid) {
            i2cFastBuffer->push(i2cSensorData, currentTime);
            i2cSlowBuffer->push(i2cSensorData, currentTime);
        }
        
        if (sps30Enabled && sps30FastBuffer && sps30SlowBuffer && sps30Data.valid) {
            sps30FastBuffer->push(sps30Data, currentTime);
            sps30SlowBuffer->push(sps30Data, currentTime);
        }
        
        if (ipsEnabled && ipsFastBuffer && ipsSlowBuffer && ipsSensorData.valid) {
            ipsFastBuffer->push(ipsSensorData, currentTime);
            ipsSlowBuffer->push(ipsSensorData, currentTime);
        }
        
        if (mcp3424Enabled && mcp3424FastBuffer && mcp3424SlowBuffer && mcp3424Data.deviceCount > 0) {
            mcp3424FastBuffer->push(mcp3424Data, currentTime);
            mcp3424SlowBuffer->push(mcp3424Data, currentTime);
        }
        
        if (ads1110Enabled && ads1110FastBuffer && ads1110SlowBuffer && ads1110Data.valid) {
            ads1110FastBuffer->push(ads1110Data, currentTime);
            ads1110SlowBuffer->push(ads1110Data, currentTime);
        }
        
        if (ina219Enabled && ina219FastBuffer && ina219SlowBuffer && ina219Data.valid) {
            ina219FastBuffer->push(ina219Data, currentTime);
            ina219SlowBuffer->push(ina219Data, currentTime);
        }
        
        if (sht40Enabled && sht40FastBuffer && sht40SlowBuffer && sht40Data.valid) {
            sht40FastBuffer->push(sht40Data, currentTime);
            sht40SlowBuffer->push(sht40Data, currentTime);
        }
        
        if (calibEnabled && calibFastBuffer && calibSlowBuffer && calibratedData.valid) {
            calibFastBuffer->push(calibratedData, currentTime);
            calibSlowBuffer->push(calibratedData, currentTime);
        }
        
        if (hchoEnabled && hchoFastBuffer && hchoSlowBuffer && hchoData.valid) {
            hchoFastBuffer->push(hchoData, currentTime);
            hchoSlowBuffer->push(hchoData, currentTime);
        }
        
        // Update fast averages (every 5 seconds)
        if (currentTime - lastFastUpdate >= 5000) {
            lastFastUpdate = currentTime;
            
            if (solarEnabled && solarFastBuffer) {
                solarFastAvg = solarFastBuffer->getWeightedAverage(currentTime, FAST_PERIOD_MS);
            }
            if (i2cEnabled && i2cFastBuffer) {
                i2cFastAvg = i2cFastBuffer->getWeightedAverage(currentTime, FAST_PERIOD_MS);
            }
            if (sps30Enabled && sps30FastBuffer) {
                sps30FastAvg = sps30FastBuffer->getWeightedAverage(currentTime, FAST_PERIOD_MS);
            }
            if (ipsEnabled && ipsFastBuffer) {
                ipsFastAvg = ipsFastBuffer->getWeightedAverage(currentTime, FAST_PERIOD_MS);
            }
            if (mcp3424Enabled && mcp3424FastBuffer) {
                mcp3424FastAvg = mcp3424FastBuffer->getWeightedAverage(currentTime, FAST_PERIOD_MS);
            }
            if (ads1110Enabled && ads1110FastBuffer) {
                ads1110FastAvg = ads1110FastBuffer->getWeightedAverage(currentTime, FAST_PERIOD_MS);
            }
            if (ina219Enabled && ina219FastBuffer) {
                ina219FastAvg = ina219FastBuffer->getWeightedAverage(currentTime, FAST_PERIOD_MS);
            }
            if (sht40Enabled && sht40FastBuffer) {
                sht40FastAvg = sht40FastBuffer->getWeightedAverage(currentTime, FAST_PERIOD_MS);
            }
            if (calibEnabled && calibFastBuffer) {
                calibFastAvg = calibFastBuffer->getWeightedAverage(currentTime, FAST_PERIOD_MS);
            }
            if (calibEnabled && calibSlowBuffer) {
                calibSlowAvg = calibSlowBuffer->getWeightedAverage(currentTime, SLOW_PERIOD_MS);
            }
            if (hchoEnabled && hchoFastBuffer) {
                hchoFastAvg = hchoFastBuffer->getWeightedAverage(currentTime, FAST_PERIOD_MS);
            }
        }
        
        // Update slow averages (every 30 seconds)
        if (currentTime - lastSlowUpdate >= 30000) {
            lastSlowUpdate = currentTime;
            
            if (solarEnabled && solarSlowBuffer) {
                solarSlowAvg = solarSlowBuffer->getWeightedAverage(currentTime, SLOW_PERIOD_MS);
            }
            if (i2cEnabled && i2cSlowBuffer) {
                i2cSlowAvg = i2cSlowBuffer->getWeightedAverage(currentTime, SLOW_PERIOD_MS);
            }
            if (sps30Enabled && sps30SlowBuffer) {
                sps30SlowAvg = sps30SlowBuffer->getWeightedAverage(currentTime, SLOW_PERIOD_MS);
            }
            if (ipsEnabled && ipsSlowBuffer) {
                ipsSlowAvg = ipsSlowBuffer->getWeightedAverage(currentTime, SLOW_PERIOD_MS);
            }
            if (mcp3424Enabled && mcp3424SlowBuffer) {
                mcp3424SlowAvg = mcp3424SlowBuffer->getWeightedAverage(currentTime, SLOW_PERIOD_MS);
            }
            if (ads1110Enabled && ads1110SlowBuffer) {
                ads1110SlowAvg = ads1110SlowBuffer->getWeightedAverage(currentTime, SLOW_PERIOD_MS);
            }
            if (ina219Enabled && ina219SlowBuffer) {
                ina219SlowAvg = ina219SlowBuffer->getWeightedAverage(currentTime, SLOW_PERIOD_MS);
            }
            if (sht40Enabled && sht40SlowBuffer) {
                sht40SlowAvg = sht40SlowBuffer->getWeightedAverage(currentTime, SLOW_PERIOD_MS);
            }
            if (calibEnabled && calibFastBuffer) {
                calibFastAvg = calibFastBuffer->getWeightedAverage(currentTime, FAST_PERIOD_MS);
            }
            if (calibEnabled && calibSlowBuffer) {
                calibSlowAvg = calibSlowBuffer->getWeightedAverage(currentTime, SLOW_PERIOD_MS);
            }
            if (hchoEnabled && hchoSlowBuffer) {
                hchoSlowAvg = hchoSlowBuffer->getWeightedAverage(currentTime, SLOW_PERIOD_MS);
            }
        }
    }
    
    // Getter functions for averaged data (return empty data if sensor disabled)
    SolarData getSolarFastAverage() const { 
        if (solarEnabled && solarFastBuffer) return solarFastAvg; 
        return SolarData{}; 
    }
    SolarData getSolarSlowAverage() const { 
        if (solarEnabled && solarSlowBuffer) return solarSlowAvg; 
        return SolarData{}; 
    }
    
    I2CSensorData getI2CFastAverage() const { 
        if (i2cEnabled && i2cFastBuffer) return i2cFastAvg; 
        return I2CSensorData{}; 
    }
    I2CSensorData getI2CSlowAverage() const { 
        if (i2cEnabled && i2cSlowBuffer) return i2cSlowAvg; 
        return I2CSensorData{}; 
    }
    
    SPS30Data getSPS30FastAverage() const { 
        if (sps30Enabled && sps30FastBuffer) return sps30FastAvg; 
        return SPS30Data{}; 
    }
    SPS30Data getSPS30SlowAverage() const { 
        if (sps30Enabled && sps30SlowBuffer) return sps30SlowAvg; 
        return SPS30Data{}; 
    }
    
    IPSSensorData getIPSFastAverage() const { 
        if (ipsEnabled && ipsFastBuffer) return ipsFastAvg; 
        return IPSSensorData{}; 
    }
    IPSSensorData getIPSSlowAverage() const { 
        if (ipsEnabled && ipsSlowBuffer) return ipsSlowAvg; 
        return IPSSensorData{}; 
    }
    
    MCP3424Data getMCP3424FastAverage() const { 
        if (mcp3424Enabled && mcp3424FastBuffer) return mcp3424FastAvg; 
        return MCP3424Data{}; 
    }
    MCP3424Data getMCP3424SlowAverage() const { 
        if (mcp3424Enabled && mcp3424SlowBuffer) return mcp3424SlowAvg; 
        return MCP3424Data{}; 
    }
    
    ADS1110Data getADS1110FastAverage() const { 
        if (ads1110Enabled && ads1110FastBuffer) return ads1110FastAvg; 
        return ADS1110Data{}; 
    }
    ADS1110Data getADS1110SlowAverage() const { 
        if (ads1110Enabled && ads1110SlowBuffer) return ads1110SlowAvg; 
        return ADS1110Data{}; 
    }
    
    INA219Data getINA219FastAverage() const { 
        if (ina219Enabled && ina219FastBuffer) return ina219FastAvg; 
        return INA219Data{}; 
    }
    INA219Data getINA219SlowAverage() const { 
        if (ina219Enabled && ina219SlowBuffer) return ina219SlowAvg; 
        return INA219Data{}; 
    }
    
    SHT40Data getSHT40FastAverage() const { 
        if (sht40Enabled && sht40FastBuffer) return sht40FastAvg; 
        return SHT40Data{}; 
    }
    SHT40Data getSHT40SlowAverage() const { 
        if (sht40Enabled && sht40SlowBuffer) return sht40SlowAvg; 
        return SHT40Data{}; 
    }
    
    CalibratedSensorData getCalibratedFastAverage() const {
        if (calibEnabled && calibFastBuffer) return calibFastAvg;
        return CalibratedSensorData{};
    }
    CalibratedSensorData getCalibratedSlowAverage() const {
        if (calibEnabled && calibSlowBuffer) return calibSlowAvg;
        return CalibratedSensorData{};
    }
    
    HCHOData getHCHOFastAverage() const {
        if (hchoEnabled && hchoFastBuffer) return hchoFastAvg;
        return HCHOData{};
    }
    HCHOData getHCHOSlowAverage() const {
        if (hchoEnabled && hchoSlowBuffer) return hchoSlowAvg;
        return HCHOData{};
    }
    
    void printAverageStatus() {
        Serial.print("Enabled sensors - Fast buffers: ");
        if (solarEnabled && solarFastBuffer) {
            Serial.print("Solar="); Serial.print(solarFastBuffer->size()); Serial.print(" ");
        }
        if (i2cEnabled && i2cFastBuffer) {
            Serial.print("I2C="); Serial.print(i2cFastBuffer->size()); Serial.print(" ");
        }
        if (sps30Enabled && sps30FastBuffer) {
            Serial.print("SPS30="); Serial.print(sps30FastBuffer->size()); Serial.print(" ");
        }
        if (ipsEnabled && ipsFastBuffer) {
            Serial.print("IPS="); Serial.print(ipsFastBuffer->size()); Serial.print(" ");
        }
        if (mcp3424Enabled && mcp3424FastBuffer) {
            Serial.print("MCP3424="); Serial.print(mcp3424FastBuffer->size()); Serial.print(" ");
        }
        if (ads1110Enabled && ads1110FastBuffer) {
            Serial.print("ADS1110="); Serial.print(ads1110FastBuffer->size()); Serial.print(" ");
        }
        if (ina219Enabled && ina219FastBuffer) {
            Serial.print("INA219="); Serial.print(ina219FastBuffer->size());
        }
        if (sht40Enabled && sht40FastBuffer) {
            Serial.print("SHT40="); Serial.print(sht40FastBuffer->size());
        }
        if (hchoEnabled && hchoFastBuffer) {
            Serial.print("HCHO="); Serial.print(hchoFastBuffer->size());
        }
        Serial.println();
        
        Serial.print("Disabled sensors: ");
        if (!solarEnabled) Serial.print("Solar ");
        if (!i2cEnabled) Serial.print("I2C ");
        if (!sps30Enabled) Serial.print("SPS30 ");
        if (!ipsEnabled) Serial.print("IPS ");
        if (!mcp3424Enabled) Serial.print("MCP3424 ");
        if (!ads1110Enabled) Serial.print("ADS1110 ");
        if (!ina219Enabled) Serial.print("INA219 ");
        if (!sht40Enabled) Serial.print("SHT40 ");
        if (!hchoEnabled) Serial.print("HCHO ");
        Serial.println();
    }
};

// Global moving average manager instance
MovingAverageManager movingAverageManager;

// Public interface functions
void initializeMovingAverages() {
    movingAverageManager.initializeBuffers();
}

void updateMovingAverages() {
    movingAverageManager.updateSensorData();
}

void printMovingAverageStatus() {
    movingAverageManager.printAverageStatus();
}

// Getter functions for external access
SolarData getSolarFastAverage() {
    return movingAverageManager.getSolarFastAverage();
}

SolarData getSolarSlowAverage() {
    return movingAverageManager.getSolarSlowAverage();
}

I2CSensorData getI2CFastAverage() {
    return movingAverageManager.getI2CFastAverage();
}

I2CSensorData getI2CSlowAverage() {
    return movingAverageManager.getI2CSlowAverage();
}

SPS30Data getSPS30FastAverage() {
    return movingAverageManager.getSPS30FastAverage();
}

SPS30Data getSPS30SlowAverage() {
    return movingAverageManager.getSPS30SlowAverage();
}

IPSSensorData getIPSFastAverage() {
    return movingAverageManager.getIPSFastAverage();
}

IPSSensorData getIPSSlowAverage() {
    return movingAverageManager.getIPSSlowAverage();
}

MCP3424Data getMCP3424FastAverage() {
    return movingAverageManager.getMCP3424FastAverage();
}

MCP3424Data getMCP3424SlowAverage() {
    return movingAverageManager.getMCP3424SlowAverage();
}

ADS1110Data getADS1110FastAverage() {
    return movingAverageManager.getADS1110FastAverage();
}

ADS1110Data getADS1110SlowAverage() {
    return movingAverageManager.getADS1110SlowAverage();
}

INA219Data getINA219FastAverage() {
    return movingAverageManager.getINA219FastAverage();
}

INA219Data getINA219SlowAverage() {
    return movingAverageManager.getINA219SlowAverage();
}

SHT40Data getSHT40FastAverage() {
    return movingAverageManager.getSHT40FastAverage();
}

SHT40Data getSHT40SlowAverage() {
    return movingAverageManager.getSHT40SlowAverage();
}


CalibratedSensorData getCalibratedFastAverage() {
    return movingAverageManager.getCalibratedFastAverage();
}

CalibratedSensorData getCalibratedSlowAverage() {
    return movingAverageManager.getCalibratedSlowAverage();
}

HCHOData getHCHOFastAverage() {
    return movingAverageManager.getHCHOFastAverage();
}

HCHOData getHCHOSlowAverage() {
    return movingAverageManager.getHCHOSlowAverage();
}
