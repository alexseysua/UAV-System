#include <control.hpp>

namespace EMIRO
{
    Control::Control() {}

    float Control::get_linear_speed() { return get().control_get_linear_speed(); }

    int Control::get_rotate_speed() { return get().control_get_rotate_speed(); }
    void Control::set_linear_speed_limit(const float &limit_m_s)
    {
        return get().control_set_linear_speed_limit(limit_m_s);
    }
    void Control::set_rotate_speed_limit(const int &limit_deg_s)
    {
        get().control_set_rotate_speed_limit(limit_deg_s);
    }
    void Control::set_PID(const double &Kp, const double &Ki, const double &Kd)
    {
        get().control_set_PID(Kp, Ki, Kd);
    }
    void Control::go(const float &x, const float &y, const float &z, const int &yaw,
                     const float &pos_precision, const int &yaw_precision)
    {
        get().control_go(x, y, z, yaw, pos_precision, yaw_precision);
    }

    // Implementation

    float Control::control_get_linear_speed() const { return linear_speed_limit; }

    int Control::control_get_rotate_speed() const
    {
        return rotate_speed_limit * 180 / M_PI;
    }

    void Control::control_set_linear_speed_limit(const float &limit_m_s) const
    {
        if (limit_m_s < 0.1f)
        {
            std::cout << C_YELLOW << S_BOLD << "Warning :" << C_RESET
                      << " Speed limit too small. Set Speed limit " << C_RED
                      << "[REJECTED]" << C_RESET << '\n';
            return;
        }
        linear_speed_limit = limit_m_s;
    }

    void Control::control_set_rotate_speed_limit(const int &limit_deg_s) const
    {
        if (limit_deg_s < 1)
        {
            std::cout << C_YELLOW << S_BOLD << "Warning :" << C_RESET
                      << "Rotation limit too small. Set Speed limit" << C_RED
                      << "[REJECTED]" << C_RESET << '\n';
            return;
        }
        else if (limit_deg_s > 180)
        {
            std::cout << C_YELLOW << S_BOLD << "Warning :" << C_RESET
                      << "Rotation limit too large (>180). Set Speed limit" << C_RED
                      << "[REJECTED]" << C_RESET << '\n';
            return;
        }
        rotate_speed_limit = limit_deg_s * M_PI / 180.0f;
    }

    void Control::control_set_PID(const double &Kp, const double &Ki,
                                  const double &Kd)
    {
        if (Kp < 0.f || Ki < 0.f || Kd < 0.f)
        {
            Copter::get_logger().write_show(
                LogLevel::ERROR,
                "Invalid PID (%.2f, %.2f, %.2f). PID Kp, Ki, Kd must be positive",
                Kp, Ki, Kd);
            return;
        }
        this->Kp = Kp;
        this->Ki = Ki;
        this->Kd = Kd;
    }

    void Control::control_go(bool yaw_control)
    {
        LinearSpeed speed = {vx, vy, vz};
        GPS::get_gps().convert(speed);
        if (yaw_control)
            Copter::get().set_vel(vx, vy, vz, avx, avy, avz);
        else
            Copter::get().set_vel(vx, vy, vz, 0.0f, 0.0f, 0.0f);
    }

    void Control::control_go(const float &x, const float &y, const float &z,
                             const int &yaw, const float &pos_precision,
                             const int &angular_precision)
    {
        Copter::get_logger().write_show(LogLevel::INFO, "Go : %.2f, %.2f, %.2f, %d",
                                        x, y, z, yaw);

        // Loop variable
        ros::Rate r(5);
        Position pos;
        Quaternion quat;
        Euler eul;
        float _yaw = 0.f;

        Position target_point = {x, y, z};
        ThreeAxisPID pid(target_point, Kp, Ki, Kd);
        pid.set_speed_limit(linear_speed_limit);

        std::cout << std::fixed << std::setprecision(3);
        while (ros::ok())
        {
            // Get current position and orientation
            Copter::get().get_pose(&pos, &quat);
            eul = to_euler(quat.w, quat.x, quat.y, quat.z);
            eul.roll *= (180.0f / M_PI);
            eul.pitch *= (180.0f / M_PI);
            eul.yaw *= (180.0f / M_PI);
            eul.yaw = eul.yaw > 180 ? eul.yaw - 360 : eul.yaw;
            _yaw = eul.yaw;

            // Close if position in target zone
            if (std::fabs(pos.x - x) < pos_precision &&
                std::fabs(pos.y - y) < pos_precision &&
                std::fabs(pos.z - z) < pos_precision &&
                std::fabs(eul.yaw - yaw) < angular_precision)
                break;

            LinearSpeed _out_pid;
            pid.get_control(pos, _out_pid);

            float diff_x = x - pos.x;
            float diff_y = y - pos.y;
            float diff_z = z - pos.z;
            float diff_yaw = yaw - eul.yaw;

            diff_yaw = diff_yaw >= 180 ? -(360 - diff_yaw) : diff_yaw;

            avz = diff_yaw * 3.14 / 180.f;
            if (std::fabs(avz) > rotate_speed_limit)
                avz = (avz > 0.f) ? rotate_speed_limit : -rotate_speed_limit;
            else if (std::fabs(eul.yaw - yaw) < angular_precision)
                avz = 0.f;

            // go(true);
            Copter::get().set_vel(_out_pid.linear_x, _out_pid.linear_y,
                                  _out_pid.linear_z, 0.f, 0.f, avz);

            // Print position
            std::cout << C_MAGENTA << S_BOLD << " >>> " << C_RESET << "Target ("
                      << diff_x << ", " << diff_y << ", " << diff_z << ", "
                      << diff_yaw << "°)     \r" << std::flush;

            ros::spinOnce();
            r.sleep();
        }
        Copter::get_logger().write_show(
            LogLevel::INFO,
            "Reached (%.2f, %.2f, %.2f, %d°) => (%.2f, %.2f, %.2f, %d°)", x, y, z,
            pos.x, pos.y, pos.z, (int)yaw, (int)_yaw);
    }

    Control::~Control() {}

    void PIDControl::set_linear_speed(const float &linear_speed_m_s)
    {
        _linear_speed = linear_speed_m_s;
    }
    void PIDControl::set_rotation_speed(const float &rotation_speed_deg_s)
    {
        _rotation_speed = rotation_speed_deg_s;
    }
    void PIDControl::set_target_point(const WayPoint &wp)
    {
        _target_point = wp;
    }
    void PIDControl::set_linear_tolerance(float tolerance)
    {
    }
    void PIDControl::set_rotation_tolerance(float tolerance)
    {
    }

    WayPoint &PIDControl::get_target_point()
    {
        return _target_point;
    }
    float &PIDControl::get_linear_speed()
    {
        return _linear_speed;
    }
    float &PIDControl::get_rotation_speed()
    {
        return _rotation_speed;
    }
    bool PIDControl::go_wait()
    {
        // Reset PID
        _integral.clear();
        _prev_error.clear();

        PIDOut __output_pid;
        WayPoint __current_pos;
        const float x_low = _target_point.x - _linear_tolerance;
        const float x_high = _target_point.x + _linear_tolerance;
        const float y_low = _target_point.y - _linear_tolerance;
        const float y_high = _target_point.y + _linear_tolerance;
        const float z_low = _target_point.z - _linear_tolerance;
        const float z_high = _target_point.z + _linear_tolerance;

        ros::Rate r(5);
        while (true)
        {
            if (!ros::ok())
                return false;
            Copter::get_position(__current_pos);
            if (std::fabs(__current_pos.x - _target_point.x) < _linear_tolerance &&
                std::fabs(__current_pos.x - _target_point.y) < _linear_tolerance &&
                std::fabs(__current_pos.x - _target_point.z) < _linear_tolerance &&
                std::fabs(__current_pos.yaw - _target_point.yaw) < _rotation_tolerance)
                break;
            calculate(__current_pos, __output_pid);

            Copter::get().set_vel(__output_pid.x_out, __output_pid.y_out, __output_pid.z_out, 0.f, 0.f, __output_pid.yaw_out);

            std::cout << CLEAR_LINE << '\r' << C_MAGENTA << S_BOLD << " >>> " << C_RESET << "To target\t["
                      << __output_pid.x_out << ", " << __output_pid.y_out << ", " << __output_pid.z_out << ", "
                      << __output_pid.yaw_out << "°]" << std::flush;

            ros::spinOnce();
            r.sleep();
        }
        Copter::get_logger().write_show(
            LogLevel::INFO,
            "Reached (%.2f, %.2f, %.2f, %d°) => (%.2f, %.2f, %.2f, %d°)", _target_point.x, _target_point.y, _target_point.z, (int)_target_point.yaw,
            __current_pos.x, __current_pos.y, __current_pos.z, (int)__current_pos.yaw);
        return true;
    }

    void PIDControl::calculate(WayPoint &current_pos, PIDOut &out)
    {
        WayPoint __wp_error = _target_point - current_pos;
        _integral += __wp_error;
        out.x_out = _Kp * __wp_error.x + _Ki * _integral.x + _Kd * (__wp_error.x - _prev_error.x);
        out.y_out = _Kp * __wp_error.y + _Ki * _integral.y + _Kd * (__wp_error.y - _prev_error.y);
        out.z_out = _Kp * __wp_error.z + _Ki * _integral.z + _Kd * (__wp_error.z - _prev_error.z);
        out.yaw_out = _Kp * __wp_error.yaw + _Ki * _integral.yaw + _Kd * (__wp_error.yaw - _prev_error.yaw) * 3.14 / 180.f;

        _prev_error = __wp_error;
    }
} // namespace EMIRO