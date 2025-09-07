#pragma once

#include <ServoInput.h>

class RCInput {
    public:
        enum ControlState { autonomous, calibration, remote_control };
        enum KillState { not_killed, killed };
    
    private:
        ServoInputSignal& m_aux1; // 3 states - Manual / Paused / Autonomous
        // ServoInputPin<rc_constants::GEAR_PIN> m_gear; // 2 states
        ServoInputSignal& m_rudd; // Continuous
        ServoInputSignal& m_elev; // Continuous
        ServoInputSignal& m_aile; // Continuous
        ServoInputSignal& m_thro; // Continuous

        int m_srg{};
        int m_swy{};
        int m_yaw{};

        ControlState m_ctr_state {ControlState::calibration};
        KillState m_kill_state {KillState::not_killed};

        void center_pin(ServoInputSignal& input_pin);

        bool check_pin_calibrated(ServoInputSignal& input_pin);

        void center_rc();

    public:
        RCInput(ServoInputSignal& aux1,
                ServoInputSignal& rudd,
                ServoInputSignal& elev,
                ServoInputSignal& aile,
                ServoInputSignal& thro);
        bool check_calibration_ready();

        void calibrate();

        void read();

        int get_srg() const;
        int get_swy() const;
        int get_yaw() const;

        int get_ctr_state() const;
};

