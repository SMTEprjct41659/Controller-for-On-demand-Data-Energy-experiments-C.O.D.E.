#ifndef CONFIG_H
#define CONFIG_H

// ========== I²C bus 1 (primary) ==========
// INA226 Module I (0x40) + BH1750 (0x23) share this bus - no address conflict
#define INA226ISDA      21
#define INA226ISCL      22

// ========== I²C bus 2 (secondary) ==========
// INA226 Module O lives alone here, so it keeps its default 0x40 address
// without conflicting with Module I. Needs external 4.7k pull-ups on both
// SDA and SCL to 3.3V, since Wire1 doesn't reliably enable internal ones.
#define INA226OSDA      16
#define INA226OSCL      17

// ========== I²C addresses ==========
#define INA226_ADDR_A   0x40   // INA226 input side (bus 1, all experiments)
#define INA226_ADDR_B   0x40   // INA226 output side (bus 2, Experiment 3) - own bus, same addr is fine
// #define BH1750_ADDR     0x23   // <-- REMOVED (no longer used)

// ========== 1‑Wire (DS18B20) ==========
#define ONEWIRE_PIN     19     // Data line for both DS18B20 sensors

// ========== Relay pins ==========
// Power source gate relay – connects generator/solar panel to circuit
#define RELAY_GENERATOR     4

// Capacitor branch relays (Experiment 2)
#define RELAY_CAP1          23
#define RELAY_CAP2          13
#define RELAY_CAP3          14
#define RELAY_CAP4          25
#define RELAY_CAP5          26

// Capacitor discharge relay (Experiment 2)
#define RELAY_CAP_DISCHARGE 32

// Battery discharger relay (Experiment 4)
#define RELAY_BAT_DISCHARGE 33

// ========== Relay active logic ==========
// false → relay turns ON with HIGH signal
// true  → relay turns ON with LOW signal
#define RELAY_ACTIVE_LOW    false

// ========== Experiment timing (all experiments) ==========
#define EXP1_SETTLE_MS      2000    // initial settling (first run only)
#define EXP1_PRE_DELAY_MS   10000   // pre‑measurement delay
#define EXP1_MEASURE_MS     300000  // active measurement (5 minutes) - Experiments 1, 2, 4
#define EXP3_MEASURE_MS     3600000UL // active measurement (1 hour) - Experiment 3 only, temperature drift study
#define EXP1_BREAK_MS       10000   // inter‑run break
#define EXP1_TOTAL_RUNS     9       // runs per experiment

// ========== Sampling ==========
#define SAMPLE_INTERVAL_MS  1000    // 1 sample per second
#define PROGRESS_INTERVAL   60      // Telegram update every N samples (60 = every minute)

#endif
