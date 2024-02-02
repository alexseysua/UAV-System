#pragma once

#include <enum.hpp>

namespace EMIRO
{
    class PID
    {

    public:
        PID(const double &Kp, const double &Ki, double Kd);

        float get_control(const double &set_point, const double &current_point);

        ~PID();

    private:
        double Kp, Ki, Kd;
        double pre_error;
        double integral;
    };

    class ThreeAxisPID
    {

    public:
        ThreeAxisPID(const Position &target_point, const double &Kp, const double &Ki, double Kd);

        void set_speed_limit(LinearSpeed limit_m_s);

        void set_speed_limit(float limit_m_s);

        void get_control(Position &current_point, LinearSpeed &out_speed);

        ~ThreeAxisPID();

    private:
        PID pid_x, pid_y, pid_z;
        Position set_point;
        LinearSpeed speed_limit{0.0f, 0.0f, 0.0f};
        bool is_limit = false;
    };
}