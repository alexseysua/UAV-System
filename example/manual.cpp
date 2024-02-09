#include <control.hpp>
#include <jsonio.hpp>

using namespace EMIRO;

int main(int argc, char **argv) {
    ros::init(argc, argv, "manual_node");
    // logger->init("Manual", FileType::CSV);
    // logger->start(true);

    Copter::init("Manual", FileType::CSV);

    // gps->init(copter, logger);
    // std::shared_ptr<Control> control =
    //     std::make_shared<Control>(copter, logger, gps);

    // // Takeoff Copter
    // copter->takeoff(1);
    // ros::Duration(10).sleep();

    // // Read JSON point
    // JsonIO reader;
    // reader = COPTER_DIR + "/docs/plan.json";
    // std::vector<Target> target = reader.get_data_vector();

    // // Set Speed limit
    // control->set_linear_speed_limit(0.5f);
    // for (Target &t : target) {
    //     if (!ros::ok()) {
    //         copter->Land();
    //         exit(EXIT_FAILURE);
    //     }
    //     std::cout << C_GREEN << S_BOLD << '[' << t.header << ']' << C_RESET
    //               << '\n';
    //     control->go(t.wp.x, t.wp.y, t.wp.z, t.wp.yaw, 0.05f, 5);
    // }

    return 0;
}