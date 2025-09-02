#pragma once

#include <ServoInput.h>

template <uint8_t AUX1, uint8_t RUDD, uint8_t ELEV, uint8_t AILE, uint8_t THRO>
class RCInput {
    public:
        enum ControlState {
            autonomous, calibration, remote_control
        };

        enum KillState {
            not_killed, killed
        };
    
    private:
        ServoInputPin<AUX1> m_rcrx_aux1; // 3 states - Manual / Paused / Autonomous
        // ServoInputPin<rc_constants::RCRX_GEAR_PIN> m_rcrx_gear; // 2 states
        ServoInputPin<RUDD> m_rcrx_rudd; // Continuous
        ServoInputPin<ELEV> m_rcrx_elev; // Continuous
        ServoInputPin<AILE> m_rcrx_aile; // Continuous
        ServoInputPin<THRO> m_rcrx_thro; // Continuous

        int m_srg{};
        int m_swy{};
        int m_yaw{};
        ControlState m_ctr_state {ControlState::calibration};
        KillState m_kill_state {KillState::not_killed};

        template <uint8_t N>
        void center_pin(ServoInputPin<N>& input_pin) {
            int center = input_pin.getRangeCenter(); 
            input_pin.setRange(center, center); 
        }

        template <uint8_t N>
        bool RCInput::check_pin_calibrated(ServoInputPin<N>& input_pin) {
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

        void center_rc() {
            center_pin(m_rcrx_elev);
            center_pin(m_rcrx_aile);
            center_pin(m_rcrx_rudd);
        }

    public:

        bool check_calibration_ready() {
            bool e_s = check_pin_calibrated(m_rcrx_elev);
            bool a_s = check_pin_calibrated(m_rcrx_aile);
            bool r_s = check_pin_calibrated(m_rcrx_rudd);
            char buffer[100];
            sprintf(buffer, "Calibrated Values | Elev: %4i  Aile: %4i  Rudd: %4i", 
                m_rcrx_elev.mapDeadzone(-100, 100, 0.1), 
                m_rcrx_aile.mapDeadzone(-100, 100, 0.1), 
                m_rcrx_rudd.mapDeadzone(-100, 100, 0.1)
            );
            Serial.println(buffer);
            return (e_s && a_s && r_s);
        }

        void calibrate() {
            center_rc();

            uint32_t loop_time = millis();

            bool calibration_ready = false;
            int calibration_zero_check = 0;

            while(m_ctr_state + 1 != ControlState::calibration || !calibration_ready || !(calibration_zero_check < 15)) {
                loop_time = millis();
                read();
                calibration_ready = check_calibration_ready();
                if (abs(m_srg) + abs(m_swy) + abs(m_yaw) <= 4) {
                calibration_zero_check += 1;
                }
                else {
                calibration_zero_check = 0;
                }
            }
        }

        void read() {
            m_srg = m_rcrx_elev.mapDeadzone(-100, 101, 0.05);
            m_swy = m_rcrx_aile.mapDeadzone(-100, 101, 0.05);
            m_yaw = m_rcrx_rudd.mapDeadzone(-100, 101, 0.05);
            m_ctr_state = static_cast<ControlState>(m_rcrx_aux1.map(0, 2));
            // m_kill_state = static_cast<KillState>(m_rcrx_gear.map(1, 0)); # WRONG ?
            char buffer[100];
            sprintf(buffer, "RC | SRG: %4i  SWY: %4i  YAW: %4i CTR: %1i KIL: %1i", 
                m_srg, m_swy, m_yaw, m_ctr_state, m_kill_state);
            Serial.println(buffer);
        }

        int get_srg() const {
            return m_srg;
        }
        int get_swy() const {
            return m_swy;
        }
        int get_yaw() const {
            return m_yaw;
        }

        int get_ctr_state() const {
            return m_ctr_state;
        }
};

