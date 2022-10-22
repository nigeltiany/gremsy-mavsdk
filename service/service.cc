//
// Created by Nigel Tiany on 20/10/22.
//

#include "gimbal.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <memory>
#include <iostream>

#include "serial_port.h"
#include "gremsy_interface.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using Service = mavsdk::rpc::gimbal::GimbalService::Service;
using mavsdk::rpc::gimbal::GimbalResult;
using mavsdk::rpc::gimbal::SetModeResponse;

class GremsyMAVSDK final : public Service {

private:

    Serial_Port serialPort;
    Gimbal_Interface gimbalInterface;

public:

    GremsyMAVSDK (char *uart, int baudrate) : serialPort(uart, baudrate), gimbalInterface(&serialPort) {
        serialPort.start();
        gimbalInterface.start();
    }


    ~GremsyMAVSDK() override {
        gimbalInterface.stop();
        serialPort.stop();
    };

    Status SetPitchAndYaw(::grpc::ServerContext *context, const ::mavsdk::rpc::gimbal::SetPitchAndYawRequest *request,
                          ::mavsdk::rpc::gimbal::SetPitchAndYawResponse *response) override {

        int res = gimbalInterface.set_gimbal_rotation_sync(request->pitch_deg(), 0, request->yaw_deg(), GIMBAL_ROTATION_MODE_RELATIVE_ANGLE);
        if (res != MAV_RESULT_ACCEPTED && res != MAV_RESULT_IN_PROGRESS) {
            return Status(grpc::DATA_LOSS, "Possible data loss");
        }

        auto result = new GimbalResult();
        result->set_result(mavsdk::rpc::gimbal::GimbalResult_Result_RESULT_SUCCESS);
        response->set_allocated_gimbal_result(result);
        return Status::OK;

    }

    Status SetPitchRateAndYawRate(::grpc::ServerContext *context,
                                  const ::mavsdk::rpc::gimbal::SetPitchRateAndYawRateRequest *request,
                                  ::mavsdk::rpc::gimbal::SetPitchRateAndYawRateResponse *response) override {
        return Status(grpc::UNIMPLEMENTED, "Unimplemented");
    }

    Status SetMode(::grpc::ServerContext *context, const ::mavsdk::rpc::gimbal::SetModeRequest *request,
                   ::mavsdk::rpc::gimbal::SetModeResponse *response) override {

        switch (request->gimbal_mode()) {
            case mavsdk::rpc::gimbal::GIMBAL_MODE_YAW_FOLLOW:
                gimbalInterface.set_gimbal_follow_mode_sync();
            case mavsdk::rpc::gimbal::GIMBAL_MODE_YAW_LOCK:
                gimbalInterface.set_gimbal_lock_mode_sync();
            case mavsdk::rpc::gimbal::GimbalMode_INT_MIN_SENTINEL_DO_NOT_USE_:
            case mavsdk::rpc::gimbal::GimbalMode_INT_MAX_SENTINEL_DO_NOT_USE_:
                return Status(grpc::INVALID_ARGUMENT, "Invalid argument");
        }

        auto result = new GimbalResult();
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
        return Status(grpc::UNIMPLEMENTED, "Unimplemented");
    }

    Status ReleaseControl(::grpc::ServerContext *context, const ::mavsdk::rpc::gimbal::ReleaseControlRequest *request,
                          ::mavsdk::rpc::gimbal::ReleaseControlResponse *response) override {
        return Status(grpc::UNIMPLEMENTED, "Unimplemented");
    }

    Status
    SubscribeControl(::grpc::ServerContext *context, const ::mavsdk::rpc::gimbal::SubscribeControlRequest *request,
                     ::grpc::ServerWriter<::mavsdk::rpc::gimbal::ControlResponse> *writer) override {
        return Status(grpc::UNIMPLEMENTED, "Unimplemented");
    }

};

void RunServer(char *u, int b) {

    std::string server_address{"localhost:11520"};
    GremsyMAVSDK service(u, b);

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

int main(int argc, char** argv) {

#ifdef __APPLE__
    char *uart_name = (char*)"/dev/tty.usbmodem1";
#else
    char *uart_name = (char*)"/dev/ttyUSB0";
#endif
    int baudrate = 115200;

    parse_commandline(argc, argv, uart_name, baudrate);

    RunServer(uart_name, baudrate);

    return 0;

}