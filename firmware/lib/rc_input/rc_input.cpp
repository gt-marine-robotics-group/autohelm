#include "rc_input.h"

RCInput::RCInput(ServoInputSignal& aux1,
                 ServoInputSignal& rudd,
                 ServoInputSignal& elev,
                 ServoInputSignal& aile,
                 ServoInputSignal& thro)
    : m_aux1(aux1), m_rudd(rudd), m_elev(elev), m_aile(aile), m_thro(thro) {}

void RCInput::center_pin(ServoInputSignal& input_pin) {
    int center = input_pin.getRangeCenter(); 
    input_pin.setRange(center, center); 
}

bool RCInput::check_pin_calibrated(ServoInputSignal& input_pin) {
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
    //Serial.println(buffer);
    if (input_pin.getRange() < 50 and input_pin.mapDeadzone(-100,100, .02) != 0){
        return false;
    }
    return true;
}

void RCInput::center_rc() {
    center_pin(m_elev);
    center_pin(m_aile);
    center_pin(m_rudd);
}

bool RCInput::check_calibration_ready() {
    bool e_s = check_pin_calibrated(m_elev);
    bool a_s = check_pin_calibrated(m_aile);
    bool r_s = check_pin_calibrated(m_rudd);
    char buffer[100];
    sprintf(buffer, "Calibrated Values | Elev: %4i  Aile: %4i  Rudd: %4i", 
        m_elev.mapDeadzone(-100, 100, 0.1), 
        m_aile.mapDeadzone(-100, 100, 0.1), 
        m_rudd.mapDeadzone(-100, 100, 0.1)
    );
    // //Serial.println(buffer);
    return (e_s && a_s && r_s);
}

void RCInput::calibrate() {
    center_rc();

    uint32_t loop_time = millis();

    bool calibration_ready = false;
    int calibration_zero_check = 0;

    while(m_ctr_state != ControlState::calibration || !calibration_ready || !(calibration_zero_check < 15)) {
        loop_time = millis();
        read();
        calibration_ready = check_calibration_ready();
        if (abs(m_srg) + abs(m_swy) + abs(m_yaw) <= 10) {
            calibration_zero_check += 1;
        }
        else {
            calibration_zero_check = 0;
        }
    }
}

void RCInput::read() {
    m_srg = m_elev.mapDeadzone(-100, 101, 0.1);
    m_swy = m_aile.mapDeadzone(-100, 101, 0.1);
    m_yaw = m_rudd.mapDeadzone(-100, 101, 0.1);
    m_ctr_state = static_cast<ControlState>(m_aux1.map(0, 2));
    // m_kill_state = static_cast<KillState>(m_gear.map(1, 0)); # WRONG ?
    char buffer[100];
    sprintf(buffer, "RC | SRG: %4i  SWY: %4i  YAW: %4i CTR: %1i KIL: %1i", 
        m_srg, m_swy, m_yaw, m_ctr_state, m_kill_state);
    //Serial.println(buffer);
}

int RCInput::get_srg() const {
    return m_srg;
}
int RCInput::get_swy() const {
    return m_swy;
}
int RCInput::get_yaw() const {
    return m_yaw;
}

int RCInput::get_ctr_state() const {
    return m_ctr_state;
}
