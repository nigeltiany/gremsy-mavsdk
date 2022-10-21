/*******************************************************************************
 * Copyright (c) 2018, The GremsyCo
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are strictly prohibited without prior permission of The GremsyCo.
 *
 * @file    gimbal_interface.h
 * @author  The GremsyCo
 * @version V1.0.0
 * @date    August-021-2018
 * @brief   This file contains expand of gMavlink
 *
 ******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#ifndef GIMBAL_INTERFACE_H_
#define GIMBAL_INTERFACE_H_

// ------------------------------------------------------------------------------
//   Includes
// ------------------------------------------------------------------------------

#include "serial_port.h"

#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <mutex>

// #include <common/mavlink.h>
#include <ardupilotmega/mavlink.h>

// ------------------------------------------------------------------------------
//   Defines
// ------------------------------------------------------------------------------
#define SYSID_ONBOARD                       4

#ifndef PI
#define PI          3.141592654f
#endif

#define PI2ANGLE    (180.f/PI)
#define ANGLE2PI    (PI/180.f)

// ------------------------------------------------------------------------------
//   Prototypes
// ------------------------------------------------------------------------------


// helper functions
uint64_t get_time_usec();
uint64_t get_time_msec();
void* start_gimbal_interface_read_thread(void *args);
void* start_gimbal_interface_write_thread(void *args);


// ------------------------------------------------------------------------------
//   Data Structures
// ------------------------------------------------------------------------------

/* Math struct */
struct attitude3f_t
{	
    float roll;
    float pitch;
    float yaw;
};

struct attitude3d_t
{
    int16_t roll;
    int16_t pitch;
    int16_t yaw;
};

struct vector3d_t
{	
    int16_t x;
    int16_t y;
    int16_t z;
};

/* Pose and sensor data type */
struct attitude3f_mutex_t
{
    attitude3f_t data;
    std::mutex mutex;
};

struct attitude3d_mutex_t
{
    attitude3d_t data;
    std::mutex mutex;
};

struct imu_t
{
    vector3d_t accel;
    vector3d_t gyro;
};

struct imu_mutex_t
{
    imu_t data;
    std::mutex mutex;
};

/* */
enum gimbal_state_t {
    GIMBAL_STATE_NOT_PRESENT = 0,
    GIMBAL_STATE_PRESENT_INITIALIZING,
    GIMBAL_STATE_PRESENT_ALIGNING,
    GIMBAL_STATE_PRESENT_RUNNING
};

struct Time_Stamps
{
    Time_Stamps()
    {
        reset_timestamps();
    }

    uint64_t heartbeat;
    uint64_t sys_status;
    uint64_t mount_status;
    uint64_t mount_orientation;
    uint64_t raw_imu;
    uint64_t info;
    uint64_t command_ack;

    void
    reset_timestamps()
    {
        heartbeat 			= 0;
        sys_status 			= 0;
        mount_status 		= 0;
        mount_orientation 	= 0;
        raw_imu 			= 0;
        command_ack 		= 0;
    }

};

struct Msg_Rate
{
    float heartbeat;
    float sys_status;
    float mount_status;
    float mount_orientation;
    float raw_imu;
    float info;

    Msg_Rate()
    {
        heartbeat 			= 0.f;
        sys_status 			= 0.f;
        mount_status 		= 0.f;
        mount_orientation 	= 0.f;
        raw_imu 			= 0.f;
        info 				= 0.f;
    }
};

struct Sequence_Numbers
{
    Sequence_Numbers()
    {
        reset_seq_num();
    }

    uint8_t heartbeat;
    uint8_t sys_status;
    uint8_t mount_status;
    uint8_t mount_orientation;
    uint8_t raw_imu;
    uint8_t command_ack;

    void reset_seq_num()
    {
        heartbeat 				= 0;
        sys_status 				= 0;
        mount_status 			= 0;
        mount_orientation 		= 0;
        raw_imu 				= 0;
        command_ack 			= 0;
    }
};  

// Struct containing information on the MAV we are currently connected to
struct Mavlink_Messages 
{

    int sysid;
    int compid;

    // Heartbeat
    mavlink_heartbeat_t 		heartbeat;

    // System Status
    mavlink_sys_status_t 		sys_status;

    // Mount status contains the encoder count value. Resolution 2^16
    mavlink_mount_status_t 		mount_status;

    // Mount orientation
    mavlink_mount_orientation_t mount_orientation;

    // Attitude
    mavlink_raw_imu_t 			raw_imu;

    mavlink_component_information_t info;

    // Time Stamps
    Time_Stamps time_stamps;

    // Msg Rate
    Msg_Rate 	msg_rate;

    // Command acknowledgement. MAV_CMD_DO_MOUNT_CONFIGURE
    uint8_t result_cmd_ack_msg_configure;

    // Command acknowledgement. MAV_CMD_DO_MOUNT_CONTROL
    uint8_t result_cmd_ack_msg_control;

    void
    reset_timestamps()
    {
        time_stamps.reset_timestamps();
    }

    // Sequence number of last packet received
    Sequence_Numbers current_seq_rx;

    void 
    reset_seq_num()
    {
        current_seq_rx.reset_seq_num();
    }
};


/**
 * @brief control_motor_t
 * Command control motor is on/off
 */
typedef enum _control_direction
{
    DIR_CW 	= 0x00,
    DIR_CCW = 0x01
} control_gimbal_direction_t;

/**
 * @brief control_motor_t
 * Command control motor is on/off
 */
typedef enum _control_gimbal_motor_t
{
    TURN_OFF    = 0x00,
    TURN_ON     = 0x01
} control_gimbal_motor_t;

/**
 * @brief control_mode_t
 * Command control gimbal mode lock/follow
 */
typedef enum gimbal_mode_t
{
    GIMBAL_TURN_OFF  	= 0x00,
    GIMBAL_LOCK_MODE   	= 0x01,
    GIMBAL_FOLLOW_MODE 	= 0x02,

    GIMBAL_RESET_MODE   = 0x04
} gimbal_mode_t;


/**
 * @brief control_mode_t
 * Command control gimbal mode lock/follow
 */
typedef enum _gimbal_rotation_mode_t
{
    GIMBAL_ROTATION_MODE_RELATIVE_ANGLE	= 0,
    GIMBAL_ROTATION_MODE_ABSOLUTE_ANGLE	= 1,
    GIMBAL_ROTATION_MODE_SPEED			= 2,		
} gimbal_rotation_mode_t;



/**
 * @brief _control_gimbal_axis_input_mode
 * Command control gimbal input mode for each axis
 */
typedef enum _control_gimbal_axis_input_mode
{
    CTRL_ANGLE_BODY_FRAME       = 0,
    CTRL_ANGULAR_RATE           = 1,
    CTRL_ANGLE_ABSOLUTE_FRAME   = 2,
} control_gimbal_axis_input_mode_t;

/**
 * @brief gimbal_state_t
 * State of gimbal
 */
typedef enum 
{
    GIMBAL_STATE_OFF            = 0x00,     /*< Gimbal is off*/
    GIMBAL_STATE_INIT           = 0x01,     /*< Gimbal is initializing*/
    GIMBAL_STATE_ON             = 0x02,     /*< Gimbal is on */
    GIMBAL_STATE_LOCK_MODE      = 0x04,     
    GIMBAL_STATE_FOLLOW_MODE    = 0x08,
    GIMBAL_STATE_SEARCH_HOME    = 0x10,
    GIMBAL_STATE_SET_HOME       = 0x20,
    GIMBAL_STATE_ERROR          = 0x40
} gimbal_state_operation_t;

/**
 * @brief gimbal_state_t
 * State of gimbal's sensor
 */
typedef enum _sensor_state
{
    SENSOR_OK                   = 0x00,     /* Gimbal's sensor is healthy */
    SENSOR_IMU_ERROR            = 0x01,     /* IMU error*/
    SENSOR_EN_TILT              = 0x02,     /* Encoder sensor is error at tilt axis*/
    SENSOR_EN_ROLL              = 0x03,     /* Encoder sensor is error at roll axis*/
    SENSOR_EN_PAN               = 0x04,     /* Encoder sensor is error at pan axis*/
} sensor_state_;


typedef struct _version
{
    uint8_t x;
    uint8_t y;
    uint8_t z;
    const char* type;
} fw_version_t;


/**
 * @brief _control_gimbal_axis_mode_t
 * Command control gimbal for each axis
 */
typedef struct _control_gimbal_axis_mode_t
{
    /* stabilize? (1 = yes, 0 = no)*/
    uint8_t stabilize;   
    
    control_gimbal_axis_input_mode_t    input_mode;
    
}control_gimbal_axis_mode_t;


/**
 * @brief gimbal_state_t
 * State of gimbal's sensor
 */
typedef struct _gimbal_status_t
{
    uint16_t    load; /*< [ms] Maximum usage the mainloop time. Values: [0-1000] - should always be below 1000*/
    uint16_t    voltage_battery; /*< [V] Battery voltage*/
    uint8_t     sensor; /*< Specific sensor occur error (encorder, imu) refer sensor_state_*/
    uint16_t    state;  /* System state of gimbal. Refer gimbal_state_t*/
    uint8_t     mode;   /*< Gimbal mode is running*/
    uint32_t    seq;
} gimbal_status_t;


/**
 * @brief _gimbal_config_t
 * This structure will contain the gimbal configuration related to speed, smooth, direction
 */
typedef struct _gimbal_config_axis_t
{
    int8_t 	dir;
    uint8_t	speed_control;
    uint8_t smooth_control;

    uint8_t speed_follow;
    uint8_t smooth_follow;
    uint8_t	window_follow;

} gimbal_config_axis_t;


/**
 * @brief config_mavlink_message_t
 * This structure will contain the configuration related to mavlink message
 */
typedef struct _config_mavlink_message_t
{
    uint8_t emit_heatbeat;
    uint8_t status_rate;
    uint8_t enc_value_rate; 
    uint8_t enc_type_send;
    uint8_t orientation_rate;
    uint8_t imu_rate;
    
} config_mavlink_message_t;


/**
 * @brief _gimbal_motor_control_t
 * stifness: Stiffness setting has a significant impact on the performance of the Gimbal. 
 *			This setting adjusts the degrees to which the gimbal tries to correct 
 *			for unwanted camera movement and hold the camera stable. 
 * 			The higher you can run the setting without vibration or oscillation, the better.
 * Holdstrength: Power level required for the corresponding axis. 
 *				This option is only recommended for advanced users. Set 40 as defaults
 */
typedef struct _gimbal_motor_control_t
{
    uint8_t stiffness;
    uint8_t	holdstrength;
} gimbal_motor_control_t;


/**
 * @brief Limit angle data structure 
 */
typedef struct _limit_angle
{
    int16_t angle_min;
    int16_t angle_max;

}limit_angle_t;


/**
 * @brief Reset mode of gimbal.
 */
typedef enum  {
    /*! Only reset yaw axis of gimbal. Reset angle of yaw axis to the sum of yaw axis angle of aircraft and fine tune angle
      of yaw axis of gimbal. */
    GIMBAL_RESET_MODE_YAW = 1,
    /*! Reset yaw axis and pitch axis of gimbal. Reset angle of yaw axis to sum of yaw axis angle of aircraft and fine tune 
      angle of yaw axis of gimbal, and reset pitch axis angle to the fine tune angle. */
    GIMBAL_RESET_MODE_PITCH_AND_YAW = 3,
    /*! Reset yaw axis and pitch axis of gimbal. Reset angle of yaw axis to sum of yaw axis angle of aircraft and fine tune
     * angle of yaw axis of gimbal, and reset pitch axis angle to sum of -90 degree and fine tune angle if gimbal
      downward, sum of 90 degree and fine tune angle if upward. */
    GIMBAL_RESET_MODE_PITCH_DOWNWARD_UPWARD_AND_YAW = 11,
    /*! Reset pitch axis of gimbal. Reset pitch axis angle to sum of -90 degree and fine tune angle if gimbal downward,
      sum of 90 degree and fine tune angle if upward. */
    GIMBAL_RESET_MODE_PITCH_DOWNWARD_UPWARD = 12,
    /*! Reset pitch axis of gimbal. Reset pitch axis angle to mapping angle */
    GIMBAL_RESET_MODE_PITCH_MAPPING = 13,
} gimbal_reset_mode_t;

/**
 * @brief control_motor_t
 * Command control motor is on/off
 */
enum param_state_t
{
    PARAM_STATE_NOT_YET_READ 		= 0,	// parameter has yet to be initialized
    PARAM_STATE_FETCH_AGAIN			= 1,	// parameter is being fetched
    PARAM_STATE_ATTEMPTING_TO_SET   = 2,	// parameter is being set
    PARAM_STATE_CONSISTENT			= 3,	// parameter is consistent
    PARAM_STATE_NONEXISTANT			= 4		// parameter does not seem to exist
};


/**
 * @brief param_index_t
 * Gimbal opens some parameters for setting. Please refer to user manual to learn more how to set 
 * that parameters
 */
enum param_index_t
{

    GMB_PARAM_VERSION_X = 0,
    GMB_PARAM_VERSION_Y,
    GMB_PARAM_VERSION_Z,

    GMB_PARAM_STIFFNESS_PITCH,
    GMB_PARAM_STIFFNESS_ROLL,
    GMB_PARAM_STIFFNESS_YAW,

    GMB_PARAM_HOLDSTRENGTH_PITCH,
    GMB_PARAM_HOLDSTRENGTH_ROLL,
    GMB_PARAM_HOLDSTRENGTH_YAW,

    GMB_PARAM_OUTPUT_FILTER,
    GMB_PARAM_GYRO_FILTER,
    GMB_PARAM_GAIN,

    GMB_PARAM_SPEED_FOLLOW_PITCH,
    GMB_PARAM_SPEED_FOLLOW_YAW,

    GMB_PARAM_SMOOTH_FOLLOW_PITCH,
    GMB_PARAM_SMOOTH_FOLLOW_YAW,

    GMB_PARAM_WINDOW_FOLLOW_PITCH,
    GMB_PARAM_WINDOW_FOLLOW_YAW,

    GMB_PARAM_SPEED_CONTROL_PITCH,
    GMB_PARAM_SPEED_CONTROL_ROLL,
    GMB_PARAM_SPEED_CONTROL_YAW,

    GMB_PARAM_SMOOTH_CONTROL_PITCH,
    GMB_PARAM_SMOOTH_CONTROL_ROLL,
    GMB_PARAM_SMOOTH_CONTROL_YAW,

    GMB_PARAM_AXIS_DIR,

    GMB_PARAM_HEATBEAT_EMIT,
    GMB_PARAM_STATUS_RATE,
    GMB_PARAM_ENCODER_VALUE_RATE,
    GMB_PARAM_ENCODER_TYPE,
    GMB_PARAM_ORIENTATION_RATE,
    GMB_PARAM_RAW_IMU_RATE,

    GMB_PARAM_MIN_LIMIT_ANGLE_PITCH,
    GMB_PARAM_MAX_LIMIT_ANGLE_PITCH,
    GMB_PARAM_MIN_LIMIT_ANGLE_ROLL,
    GMB_PARAM_MAX_LIMIT_ANGLE_ROLL,
    GMB_PARAM_MIN_LIMIT_ANGLE_YAW,
    GMB_PARAM_MAX_LIMIT_ANGLE_YAW,
    
   GIMBAL_NUM_TRACKED_PARAMS
};
// ----------------------------------------------------------------------------------
//   Gimbal Interface Class
// ----------------------------------------------------------------------------------
/*
 * Gimbal Interface Class
 *
 * This starts two threads for read and write over MAVlink. The read thread
 * listens for any MAVlink message and pushes it to the current_messages
 * attribute.  The write thread at the moment only streams a heartbeat 1hz
 */
class Gimbal_Interface
{

public:

    Gimbal_Interface();
    Gimbal_Interface(Serial_Port *serial_port_);
    ~Gimbal_Interface();

    char reading_status;
    char writing_status;
    uint64_t write_count;
    uint64_t write_heartbeat_count;

    /// Id of gimbal if it has mounted
    int system_id;
    int gimbal_id;
    int companion_id;

    void read_messages();
    int  write_message(mavlink_message_t message);

    void start();
    void stop();

    void start_read_thread();
    void start_write_thread(void);

    void handle_quit( int sig );

    bool get_flag_exit(void);

    bool get_connection(void);

    bool present();
    /**
     * @brief  This function shall reboot the gimbal
     * @param: NONE
     * @ret: None
     */
    void set_gimbal_reboot(void);

    /**
     * @brief  This function shall reboot the gimbal
     * @param: NONE
     * @ret: @HAVE_ENUM_MAV_RESULT 
     */
    uint8_t set_gimbal_rc_input(void);	

    /**
     * @brief  This function shall turn on/off gimbal
     * @param: type see control_gimbal_motor_t
     * @ret: None
     */
    void set_gimbal_motor_mode(control_gimbal_motor_t type);

    /**
     * @brief  This function shall set gimbal mode
     * @param: type see gimbal_mode_t
     * @ret: @HAVE_ENUM_MAV_RESULT 
     */
    uint8_t set_gimbal_mode(gimbal_mode_t mode);

 
     /**
     * @brief  This function shall set gimbal mode
     * @param: type see gimbal_mode_t
     * @ret: gimbal_mode_t
     */
    gimbal_mode_t get_gimbal_mode(void);

    /**
     * @brief  This function shall reset gimbal with some mode
     * @param: type see gimbal_reset_mode_t
     * @ret: @HAVE_ENUM_MAV_RESULT 
     */
    uint8_t set_gimbal_reset_mode(gimbal_reset_mode_t reset_mode);
    

     /**
     * @brief  This function shall set gimbal mode
     * @param: type see gimbal_mode_t
     * @ret: * @ret: @HAVE_ENUM_MAV_RESULT 
     */
    uint8_t set_gimbal_lock_mode_sync(void);

    /**
     * @brief  This function shall set gimbal mode
     * @param: type see gimbal_mode_t
     * @ret: * @ret: @HAVE_ENUM_MAV_RESULT 
     */
    uint8_t set_gimbal_follow_mode_sync(void);


     /**
     * @brief  This function shall set mode for each axis
     * @param: type see control_gimbal_axis_mode_t
     * @note: DEPRECATED: Replaced by gimbal motation mode used in gimbal_rotation_mode_t
     * @ret: None
     */
    void set_gimbal_axes_mode(control_gimbal_axis_mode_t tilt,
                        control_gimbal_axis_mode_t roll,
                        control_gimbal_axis_mode_t pan);

    /**
     * @brief  This function shall set mode for each axis.
     * The gimbal will move following the gimbal axes mode.
     * @param: type see control_gimbal_axis_mode_t
     * @ret: * @ret: @HAVE_ENUM_MAV_RESULT 
     */
    uint8_t set_gimbal_rotation_sync(float tilt, float roll, float pan, gimbal_rotation_mode_t mode);

    /**
     * @brief  This function get gimbal status
     * @param: None
     * @ret: Gimbal status
     */
    gimbal_status_t get_gimbal_status(void);

    /**
     * @brief  This function get gimbal imu raw values
     * @param: None
     * @ret: Gimbal status
     */
    imu_t get_gimbal_raw_imu(void);

    /**
     * @brief  This function get gimbal mount orientation
     * @param: None
     * @ret: Gimbal status
     */
    attitude3f_t get_gimbal_mount_orientation(void);

    /**
     * @brief  This function get gimbal mount status
     * @param: None
     * @ret: Gimbal status
     */
    attitude3d_t get_gimbal_encoder(void);

    /**
     * @brief  This function get gimbal time stamps 
     * @param: None
     * @ret: Gimbal status
     */
    Time_Stamps get_gimbal_time_stamps(void);

    /**
     * @brief  This function get gimbal the sequence numbers 
     * @param: None
     * @ret: Gimbal status
     */
    Sequence_Numbers get_gimbal_seq_num(void);

    /**
     * @brief  This function get the feedback from gimbal after sending 
     * MAV_CMD_DO_MOUNT_CONFUGURE
     * @param: None
     * @ret: In-progress or Accepted. Refer to @MAV_RESULT
     */

    uint8_t get_command_ack_do_mount_configure(void);

    /**
     * @brief  This function get the feedback from gimbal after sending 
     * MAV_CMD_DO_MOUNT_CONTROL
     * @param: None
     * @ret: In-progress or Accepted. Refer to @MAV_RESULT
     */

    uint8_t get_command_ack_do_mount_control(void);

    /**
     * @brief  This function get the feedback from gimbal after sending 
     * MAV_CMD_DO_MOUNT_CONTROL
     * @param: None
     * @ret: In-progress or Accepted. Refer to @MAV_RESULT
     */
    uint8_t get_command_ack_gimbal_mode(void);

     /**
     * @brief  This function get the firmware version from gimbal
     * 
     * @param: None
     * @ret: see fw_version_t structure
     */
    fw_version_t get_gimbal_version(void)
    {
        fw_version_t fw;

        fw.x = 	_params_list[GMB_PARAM_VERSION_X].value;
        fw.y = 	_params_list[GMB_PARAM_VERSION_Y].value;
        fw.z = 	(_params_list[GMB_PARAM_VERSION_Z].value & 0x3F);

        if((_params_list[GMB_PARAM_VERSION_Z].value & 0xC0) == FIRMWARE_VERSION_TYPE_ALPHA)
        {
            fw.type = this->alpha;
        }
        else if((_params_list[GMB_PARAM_VERSION_Z].value & 0xC0) == FIRMWARE_VERSION_TYPE_BETA)
        {
            fw.type = this->beta;
        }
        else if((_params_list[GMB_PARAM_VERSION_Z].value & 0xC0) == FIRMWARE_VERSION_TYPE_RC)
        {
            fw.type = this->preview;
        }
        else if((_params_list[GMB_PARAM_VERSION_Z].value & 0xC0) == 00)
        {
            fw.type = this->official;
        }

        return fw;
    }

    /**
     * @brief  This function shall configure on the tilt axis
     * 
     * @param: config see  gimbal_config_axis_t structure
     * @note: The smooth starts with a low value of 50 
     *			Slowly increase this setting until you feel an oscillation in the pan axis, 
     *			then reduce the setting until the oscillation subsides.
     * @ret: None
     */
    void set_gimbal_config_tilt_axis(gimbal_config_axis_t config);

    /**
     * @brief  This function get the config of tilt axis
     * 
     * @param: None
     * @ret: gimbal_config_axis_t contains setting related to tilt axis
     */
    gimbal_config_axis_t get_gimbal_config_tilt_axis(void);

    /**
     * @brief  This function shall configure on the roll axis
     * 
     * @param: config see  gimbal_config_axis_t structure
     * @note: The smooth starts with a low value of 50 
     *			Slowly increase this setting until you feel an oscillation in the pan axis, 
     *			then reduce the setting until the oscillation subsides.
     * @ret: None
     */
    void set_gimbal_config_roll_axis(gimbal_config_axis_t config);

    /**
     * @brief  This function get the config of roll axis
     * 
     * @param: None
     * @ret: gimbal_config_axis_t contains setting related to roll axis
     */
    gimbal_config_axis_t get_gimbal_config_roll_axis(void);

    /**
     * @brief  This function shall configure on the pan axis
     * 
     * @param: config see  gimbal_config_axis_t structure
     * @note: The smooth starts with a low value of 50 
     *			Slowly increase this setting until you feel an oscillation in the pan axis, 
     *			then reduce the setting until the oscillation subsides.
     * @ret: None
     */
    void set_gimbal_config_pan_axis(gimbal_config_axis_t config);

    /**
     * @brief  This function get the config of pan axis
     * 
     * @param: None
     * @ret: gimbal_config_axis_t contains setting related to pan axis
     */
    gimbal_config_axis_t get_gimbal_config_pan_axis(void);

    /**
     * @brief  This function set the enable or disable the reduce drift of the gimbal by using attitude of the aircarf
     * 
     * @param: flag - enable/disable the recude drift of the gimbal by combining attitude from the aircraft
     * @ret: None
     */
    void set_gimbal_combine_attitude(bool flag);

    /**
     * @brief  This function set motor controls setting
     * @param: tilt, roll, pan - stiffness and holdstrengtg, see user_manual (https://gremsy.com/gremsy-t3-manual/)
     * @param: def_gyro_filter - The coefficent for denoising the sensor filter
     * @param: def_output_filter - The coefficent for denoising the output filter
     * @param: def_gain - Defines how fast each axis will return to commanded position. 
     * @ret: gimbal_motor_control_t contains setting related to tilt axis
     * 
     * 
     *	GYRO FILTER 	2
     *	OUTPUT FILTER 	3
     *
     *	HOLD STRENGTH 	TILT 	ROLL 	PAN
     *					40 		40 		40
     * 	GAIN 			120		120		120
     */
    void set_gimbal_motor_control(	gimbal_motor_control_t tilt, 
                                    gimbal_motor_control_t roll,
                                    gimbal_motor_control_t pan, 
                                    uint8_t gyro_filter, uint8_t output_filter, uint8_t gain);

    /**
     * @brief  This function get motor controls setting
     * @param: tilt, roll, pan - stiffness and holdstrengtg, see user_manual (https://gremsy.com/gremsy-t3-manual/)
     * @param: def_gyro_filter - The coefficent for denoising the sensor filter
     * @param: def_output_filter - The coefficent for denoising the output filter
     * @param: def_gain - Defines how fast each axis will return to commanded position. 
     * @ret: gimbal_motor_control_t contains setting related to tilt axis
     * 
     * 
     *	GYRO FILTER 	2
     *	OUTPUT FILTER 	3
     *
     *	HOLD STRENGTH 	TILT 	ROLL 	PAN
     *					40 		40 		40
     * 	GAIN 			120		120		120
     */
    void get_gimbal_motor_control(	gimbal_motor_control_t& tilt, 
                                    gimbal_motor_control_t& roll,
                                    gimbal_motor_control_t& pan, 
                                    uint8_t& gyro_filter, uint8_t& output_filter, uint8_t& gain);



    /**
     * @brief  This function set the configuration the message mavink with rate 
     * 
     * @param: emit_heatbeat - enable the heartbeat when lost connection or not enable = 1, disable = 0
     * @param: status_rate - the time rate of the system status. Gimbal sends as default 10Hz
     * @param: enc_value_rate - the time rate of the encoder values. Gimbal sends as default 50Hz
     * @param: enc_type_send - Set the type of encoder has been sent from gimbal is angle or count (Resolution 2^16)
     * @param: orien_rate - the time rate of the mount orientation of gimbal.Gimbal sends as default 50Hz
     * @param: imu_rate - the time rate of the raw_imu value. Gimbal sends as default 10Hz
     * @NOTE The range [0 - 100Hz]. 0 will disable that message
     * @ret: None
     */

    void set_gimbal_config_mavlink_msg(uint8_t emit_heatbeat = 1, 
                                        uint8_t status_rate = 10, 
                                        uint8_t enc_value_rate = 50, 
                                        uint8_t enc_type_send = 0,
                                        uint8_t orien_rate = 50,
                                        uint8_t imu_rate = 10);

    /**
     * @brief  This function get the mavlink configuration message
     * 
     * @param: None
     * @ret: config_mavlink_message_t contains setting related to mavlink configuration message
     */
    config_mavlink_message_t get_gimbal_config_mavlink_msg(void);


    /**
     * @brief Set limit angle for pitch.
     * @details Please refer to Gremsy site <gremsy.com> for
     * details about default limit angle of Gimbal.
     * @param limitAngle: limit angle.
     * @return None
     */
    void set_limit_angle_pitch(limit_angle_t limit_angle);

    /**
     * @brief Get limit angle for pitch.
     * @details Please refer to Gremsy site <gremsy.com> for
     * details about default limit angle of Gimbal.
     * @param limitAngle: limit angle.
     * @return None
     */
    void get_limit_angle_pitch(limit_angle_t &limit_angle);

    /**
     * @brief Set limit angle for yaw.
     * @details Please refer to Gremsy site <gremsy.com> for
     * details about default limit angle of Gimbal.
     * @param limitAngle: limit angle.
     * @return None
     */
    void set_limit_angle_yaw(limit_angle_t limit_angle);

    /**
     * @brief Get limit angle for yaw.
     * @details Please refer to Gremsy site <gremsy.com> for
     * details about default limit angle of Gimbal.
     * @param limitAngle: limit angle.
     * @return None
     */
    void get_limit_angle_yaw(limit_angle_t &limit_angle);

    /**
     * @brief Set limit angle for roll.
     * @details Please refer to Gremsy site <gremsy.com> for
     * details about default limit angle of Gimbal.
     * @param limitAngle: limit angle.
     * @return None
     */
    void set_limit_angle_roll(limit_angle_t limit_angle);

    /**
     * @brief Get limit angle for roll.
     * @details Please refer to Gremsy site <gremsy.com> for
     * details about default limit angle of Gimbal.
     * @param limitAngle: limit angle.
     * @return None
     */
    void get_limit_angle_roll(limit_angle_t &limit_angle);

    /**
     * @brief Get set autopilot attitude to send to gimbal
     * @details This method is used to update autopilot attitude
     * for gimbal to reduce pan drift
     * @param attitude: autopilot attitude
     * @return None
     */
    void set_autopilot_attitude(attitude3f_t &attitude);

    float get_heartbeat_rate(void) {
        return current_messages.msg_rate.heartbeat;
    }

    float get_sys_status_rate(void) {
        return current_messages.msg_rate.sys_status;
    }

    float get_mount_status_rate(void) {
        return current_messages.msg_rate.mount_status;
    }

    float get_mount_orientation_rate(void) {
        return current_messages.msg_rate.mount_orientation;
    }

    float get_raw_imu_rate(void) {
        return current_messages.msg_rate.raw_imu;
    }

    float get_comp_info_rate(void) {
        return current_messages.msg_rate.info;
    }

    void reset_params();

private:

    Serial_Port *serial_port;

    bool time_to_exit;
    bool has_detected;

    bool reduce_pan_drift_enable;

    uint64_t _last_report_msg_us;

    uint8_t is_received_ack;
    uint8_t is_wait_ack;

    pthread_t read_tid;
    pthread_t write_tid;

    void read_thread();
    void write_thread(void);

    void write_setpoint();
    void write_heartbeat(void);
    void write_test(void);
    void send_autopilot_attitude(void);

    Mavlink_Messages current_messages;

    gimbal_status_t gimbal_status;

    gimbal_state_t _state;

    attitude3f_mutex_t gimbal_attitude;
    attitude3f_mutex_t autopilot_attitude;

    imu_mutex_t gimbal_imu;
    attitude3d_mutex_t gimbal_encoder;

    constexpr static const char* alpha  		= "ALPHA";
    constexpr static const char* beta    	 	= "BETA";
    constexpr static const char* preview    	= "PREVIEW";
    constexpr static const char* official   	= "OFFICIAL";

    //Gimbal params
    // void reset_params();
    bool params_initialized();
    bool params_received_all();
    void fetch_params();

    void get_param(param_index_t param, int16_t& value, int16_t def_value = 0);
    void set_param(param_index_t param, int16_t value);

    void param_update();
    void param_process(void);
    void handle_param_value(mavlink_message_t *msg);

    const char* get_param_name(param_index_t param)
    {	
        return _params_list[param].gmb_id;
    }
    const uint8_t get_gmb_index(param_index_t param)
    {
        return _params_list[param].gmb_idx;
    }

    const uint32_t	_time_lost_connection = 60000000;
    const uint32_t 	_retry_period	= 100;  //100ms
    const uint8_t 	_max_fetch_attempts = 10; // times

    struct 
    {
        const uint8_t gmb_idx;
        const char* gmb_id;
        int16_t value;

        param_state_t state;
        uint8_t	fetch_attempts;
        bool seen;

        std::mutex mutex;

    } _params_list[GIMBAL_NUM_TRACKED_PARAMS] = {

        // Gimbal version
        {0, "VERSION_X", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {67, "VERSION_Y", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {68, "VERSION_Z", 0, PARAM_STATE_NOT_YET_READ, 0, false},

        // Gimbal stiffness
        {2, "PITCH_P", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {5, "ROLL_P", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {8, "YAW_P", 0, PARAM_STATE_NOT_YET_READ, 0, false},

        // Gimbal hold strength
        {11, "PITCH_POWER", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {12, "ROLL_POWER", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {13, "YAW_POWER", 0, PARAM_STATE_NOT_YET_READ, 0, false},

        {9, "YAW_I", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {29, "GYRO_LPF", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {3, "PITCH_I", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        

        // Gimbal speed follow
        {14, "PITCH_FOLLOW", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {16, "YAW_FOLLOW", 0, PARAM_STATE_NOT_YET_READ, 0, false},

        // Gimbal follow filter
        {17, "PITCH_FILTER", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {19, "YAW_FILTER", 0, PARAM_STATE_NOT_YET_READ, 0, false},

        // Gimbal follow windown
        {57, "TILT_WINDOW", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {58, "PAN_WINDOW", 0, PARAM_STATE_NOT_YET_READ, 0, false},

        // Gimbal speed control
        {60, "RC_PITCH_SPEED", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {61, "RC_ROLL_SPEED", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {62, "RC_YAW_SPEED", 0, PARAM_STATE_NOT_YET_READ, 0, false},

        // Gimbal smooth control
        {36, "RC_PITCH_LPF", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {37, "RC_ROLL_LPF", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {38, "RC_YAW_LPF", 0, PARAM_STATE_NOT_YET_READ, 0, false},

        // Direction
        {63, "JOY_AXIS", 0, PARAM_STATE_NOT_YET_READ, 0, false},

        // Setting message rate
        {72, "HEARTBEAT_EMIT", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {73, "STATUS_RATE", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {74, "ENC_CNT_RATE", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {75, "ENC_TYPE_SEND", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {76, "ORIEN_RATE", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {77, "IMU_RATE", 0, PARAM_STATE_NOT_YET_READ, 0, false},

        {30, "TRAVEL_MIN_PIT", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {31, "TRAVEL_MAX_PIT", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {32, "TRAVEL_MIN_ROLL", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {33, "TRAVEL_MAX_ROLL", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {69, "TRAVEL_MIN_PAN", 0, PARAM_STATE_NOT_YET_READ, 0, false},
        {70, "TRAVEL_MAX_PAN", 0, PARAM_STATE_NOT_YET_READ, 0, false},
    };

    uint64_t _last_request_ms;
    uint64_t _last_set_ms;
};


#endif // GIMBAL_INTERFACE_H_

/*********** Portions COPYRIGHT 2018 Gremsy.Co., Ltd.*****END OF FILE**********/