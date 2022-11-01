//
// Created by Nigel Tiany on 20/10/22.
//

#include <grpcpp/grpcpp.h>
#include <memory>
#include <iostream>
#include <chrono>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/gimbal/gimbal.h>
#include <future>
#include <thread>
#include "gimbal.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using Service = mavsdk::rpc::gimbal::GimbalService::Service;
using mavsdk::rpc::gimbal::GimbalResult;
using mavsdk::rpc::gimbal::SetModeResponse;
using std::chrono::seconds;
using std::this_thread::sleep_for;

class GremsyMAVSDK final : public Service {

private:
    mavsdk::Gimbal gimbal;
    mavsdk::Telemetry telemetry;

public:

    explicit GremsyMAVSDK(std::shared_ptr<mavsdk::System> mavsdkSys) : gimbal{ mavsdkSys }, telemetry{ mavsdkSys } {
        telemetry.subscribe_camera_attitude_euler([](mavsdk::Telemetry::EulerAngle angle) {
            std::cout << "Gimbal angle pitch: " << angle.pitch_deg << " deg, yaw: " << angle.yaw_deg << " yaw\n";
        });
    }

    ~GremsyMAVSDK() override = default;;

    Status SetPitchAndYaw(::grpc::ServerContext *context, const ::mavsdk::rpc::gimbal::SetPitchAndYawRequest *request,
                          ::mavsdk::rpc::gimbal::SetPitchAndYawResponse *response) override {

        auto result = new GimbalResult();
        mavsdk::Gimbal::Result exec_result = gimbal.set_pitch_and_yaw(request->pitch_deg(), request->yaw_deg());

        if (exec_result != mavsdk::Gimbal::Result::Success) {
            result->set_result(mavsdk::rpc::gimbal::GimbalResult_Result_RESULT_ERROR);
            return { grpc::UNKNOWN, "An unknown error occurred" };
        }

        result->set_result(mavsdk::rpc::gimbal::GimbalResult_Result_RESULT_SUCCESS);
        return Status::OK;

    }

    Status SetPitchRateAndYawRate(::grpc::ServerContext *context,
                                  const ::mavsdk::rpc::gimbal::SetPitchRateAndYawRateRequest *request,
                                  ::mavsdk::rpc::gimbal::SetPitchRateAndYawRateResponse *response) override {
        return Status(grpc::UNIMPLEMENTED, "Unimplemented");
    }

    Status SetMode(::grpc::ServerContext *context, const ::mavsdk::rpc::gimbal::SetModeRequest *request,
                   ::mavsdk::rpc::gimbal::SetModeResponse *response) override {

        auto result = new GimbalResult();
        mavsdk::Gimbal::Result exec_result;

        switch (request->gimbal_mode()) {
            case mavsdk::rpc::gimbal::GIMBAL_MODE_YAW_FOLLOW:
                exec_result = gimbal.set_mode(mavsdk::Gimbal::GimbalMode::YawFollow);
            case mavsdk::rpc::gimbal::GIMBAL_MODE_YAW_LOCK:
                exec_result = gimbal.set_mode(mavsdk::Gimbal::GimbalMode::YawLock);
            case mavsdk::rpc::gimbal::GimbalMode_INT_MIN_SENTINEL_DO_NOT_USE_:
            case mavsdk::rpc::gimbal::GimbalMode_INT_MAX_SENTINEL_DO_NOT_USE_:
                return { grpc::INVALID_ARGUMENT, "Invalid argument" };
        }

        if (exec_result != mavsdk::Gimbal::Result::Success) {
            result->set_result(mavsdk::rpc::gimbal::GimbalResult_Result_RESULT_ERROR);
            return { grpc::UNKNOWN, "An unknown error occurred" };
        }

        result->set_result(mavsdk::rpc::gimbal::GimbalResult_Result_RESULT_SUCCESS);
        response->set_allocated_gimbal_result(result);
        return Status::OK;

    }

    Status SetRoiLocation(::grpc::ServerContext *context, const ::mavsdk::rpc::gimbal::SetRoiLocationRequest *request,
                          ::mavsdk::rpc::gimbal::SetRoiLocationResponse *response) override {
        return Status(grpc::UNIMPLEMENTED, "Unimplemented");
    }

    Status TakeControl(::grpc::ServerContext *context, const ::mavsdk::rpc::gimbal::TakeControlRequest *request,
                       ::mavsdk::rpc::gimbal::TakeControlResponse *response) override {

        std::cout << "take control" << std::endl;
        auto exec_result = gimbal.take_control(mavsdk::Gimbal::ControlMode::Primary);

        auto result = new GimbalResult();
        response->set_allocated_gimbal_result(result);

        if (exec_result != mavsdk::Gimbal::Result::Success) {
            result->set_result(mavsdk::rpc::gimbal::GimbalResult_Result_RESULT_ERROR);
            return { grpc::UNKNOWN, "An unknown error occurred" };
        }
        result->set_result(mavsdk::rpc::gimbal::GimbalResult_Result_RESULT_SUCCESS);
        return Status::OK;

    }

    Status ReleaseControl(::grpc::ServerContext *context, const ::mavsdk::rpc::gimbal::ReleaseControlRequest *request,
                          ::mavsdk::rpc::gimbal::ReleaseControlResponse *response) override {

        auto exec_result = gimbal.release_control();

        auto result = new GimbalResult();
        response->set_allocated_gimbal_result(result);

        if (exec_result != mavsdk::Gimbal::Result::Success) {
            result->set_result(mavsdk::rpc::gimbal::GimbalResult_Result_RESULT_ERROR);
            return { grpc::UNKNOWN, "An unknown error occurred" };
        }
        result->set_result(mavsdk::rpc::gimbal::GimbalResult_Result_RESULT_SUCCESS);
        return Status::OK;

    }

    Status
    SubscribeControl(::grpc::ServerContext *context, const ::mavsdk::rpc::gimbal::SubscribeControlRequest *request,
                     ::grpc::ServerWriter<::mavsdk::rpc::gimbal::ControlResponse> *writer) override {
        return Status(grpc::UNIMPLEMENTED, "Unimplemented");
    }

};

void RunServer(std::shared_ptr<mavsdk::System> mavsdkSys) {

    std::string server_address{"localhost:11520"};
    GremsyMAVSDK service(mavsdkSys);

    // Build server
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server{builder.BuildAndStart()};

    // Run server
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();

}

void parse_commandline(int argc, char **argv, char *&uart_name, int &baudrate) {
    // string for command line usage
    const char *commandline_usage = "usage: mavlink_serial -d <devicename> -b <baudrate>";

    // Read input arguments
    for (int i = 1; i < argc; i++) { // argv[0] is "mavlink"

        // Help
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("%s\n",commandline_usage);
            throw EXIT_FAILURE;
        }

        // UART device ID
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--device") == 0) {
            if (argc > i + 1) {
                uart_name = argv[i + 1];

            } else {
                printf("%s\n",commandline_usage);
                throw EXIT_FAILURE;
            }
        }

        // Baud rate
        if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--baud") == 0) {
            if (argc > i + 1) {
                baudrate = atoi(argv[i + 1]);

            } else {
                printf("%s\n",commandline_usage);
                throw EXIT_FAILURE;
            }
        }
    }
    // end: for each input argument

    // Done!
    return;

}

std::shared_ptr<mavsdk::System> get_system(mavsdk::Mavsdk& mavsdk) {

    std::cout << "Waiting to discover system...\n";
    auto prom = std::promise<std::shared_ptr<mavsdk::System>>{};
    auto fut = prom.get_future();

    // We wait for new systems to be discovered, once we find one that has an
    // autopilot, we decide to use it.
    mavsdk::Mavsdk::NewSystemHandle handle = mavsdk.subscribe_on_new_system([&mavsdk, &prom, &handle]() {
        auto system = mavsdk.systems().back();

        if (system->has_autopilot()) {
            std::cout << "Discovered autopilot\n";

            // Unsubscribe again as we only want to find one system.
            mavsdk.unsubscribe_on_new_system(handle);
            prom.set_value(system);
        }
    });

    // We usually receive heartbeats at 1Hz, therefore we should find a
    // system after around 3 seconds max, surely.
    if (fut.wait_for(seconds(4)) == std::future_status::timeout) {
        std::cerr << "No autopilot found.\n";
        return {};
    }

    // Get discovered system now.
    return fut.get();
}


int main(int argc, char** argv) {

#ifdef __APPLE__
    char *uart_name = (char*)"/dev/tty.usbmodem1";
#else
    char *uart_name = (char*)"/dev/ttyACM0";
#endif
    int baudrate = 57600;

    parse_commandline(argc, argv, uart_name, baudrate);

    mavsdk::Mavsdk mavsdk;
    std::string device = uart_name;
    std::string connection = "serial://" + device + ":" + std::to_string(baudrate);
    std::cout << "Connecting through: " + connection << std::endl;
    mavsdk.add_any_connection(connection);

    auto mavsdkSystem = get_system(mavsdk);
    if (!mavsdkSystem) {
        return 1;
    }

    RunServer(mavsdkSystem);

    return 0;

}