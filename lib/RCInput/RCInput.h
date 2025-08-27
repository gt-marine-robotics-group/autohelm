#pragma once

#include <ServoInput.h>

namespace rc_constants {
    constexpr int RCRX_AUX1_PIN = 43; // checks if killed - need to figure out reset check
    constexpr int RCRX_GEAR_PIN = 45; // Kill switch
    constexpr int RCRX_RUDD_PIN = 47; // yaw
    constexpr int RCRX_ELEV_PIN = 49; // WAM-V translate forward / backward
    constexpr int RCRX_AILE_PIN = 51; // WAM-V translate left / right
    constexpr int RCRX_THRO_PIN = 53;
}

class RemoteControl {
    public:
        enum ControlState {
            autonomous, calibration, remote_control
        };

        enum KillState {
            not_killed, killed
        };
    
    private:

        ServoInputPin<rc_constants::RCRX_AUX1_PIN> m_rcrx_aux1; // 3 states - Manual / Paused / Autonomous
        ServoInputPin<rc_constants::RCRX_GEAR_PIN> m_rcrx_gear; // 2 states
        ServoInputPin<rc_constants::RCRX_RUDD_PIN> m_rcrx_rudd; // Continuous
        ServoInputPin<rc_constants::RCRX_ELEV_PIN> m_rcrx_elev; // Continuous
        ServoInputPin<rc_constants::RCRX_AILE_PIN> m_rcrx_aile; // Continuous
        ServoInputPin<rc_constants::RCRX_THRO_PIN> m_rcrx_thro; // Continuous

        int m_srg{};
        int m_swy{};
        int m_yaw{};
        ControlState m_ctr_state {ControlState::calibration};
        KillState m_kill_state {KillState::not_killed};

        template <uint8_t N>
        static void center_pin(ServoInputPin<N>& input_pin);

        template <uint8_t N>
        static bool check_pin_calibrated(ServoInputPin<N>& input_pin);

        void center_rc();

    public:

        bool check_calibration_ready();

        void calibrate(LightTower& lt);

        void read();

        int get_srg() const;
        int get_swy() const;
        int get_yaw() const;

        int get_ctr_state() const;
};

template <uint8_t N>
void RemoteControl::center_pin(ServoInputPin<N>& input_pin) {
    int center = input_pin.getRangeCenter(); 
    input_pin.setRange(center, center); 
}

template <uint8_t N>
bool RemoteControl::check_pin_calibrated(ServoInputPin<N>& input_pin) {
    const uint16_t pulse = (uint16_t) input_pin.getPulseRaw();
    // Check + store range min/max
    if (pulse < input_pin.getRangeMin()) {
        input_pin.setRangeMin(pulse);
    }
    else if (pulse > input_pin.getRangeMax()) {
        input_pin.setRangeMax(pulse);
    }
    char buffer[100];
    sprintf(buffer, "Servo PWM (us) | Min: %4u  Val: %4u  Max: %4u | Range: %4u", 
        input_pin.getRangeMin(), pulse, input_pin.getRangeMax(), input_pin.getRange());
    Serial.println(buffer);
    if (input_pin.getRange() < 50 and input_pin.mapDeadzone(-100,100, .02) != 0){
        return false;
    }
    return true;
}

#endif