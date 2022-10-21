.PHONY: generate

SRC_DIR := ./proto
DST_DIR := ./service

generate:
	protoc -I=$(SRC_DIR) --cpp_out=$(DST_DIR) $(SRC_DIR)/gimbal.proto
	protoc -I=$(SRC_DIR) --grpc_out=$(DST_DIR) --plugin=protoc-gen-grpc=/usr/bin/grpc_cpp_plugin $(SRC_DIR)/gimbal.proto
