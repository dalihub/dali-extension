# This script set environment value for dali

# 1. Introduction
#  this script is executed when device is booted.
#  when device is booted, environment values are applied across system wide.
#  it means every dali applications are affected to this script.
#  if developer want specific environment value,
#  please remove '#' from the desired environment value and reboot to apply the changes

# 2. Example

# @ Enable ttrace for checking
#  This environment value can enable ttrace
#
#   DALI_PERFORMANCE_TIMESTAMP_OUTPUT=4
#
#  Where timestamped events for update/render/event and custom events are output.
#  See TimeStampOutput in performance-interface.h for values
#
#   0:NO_TIME_STAMP_OUTPUT, 1:OUTPUT_DALI_LOG, 2:OUTPUT_KERNEL_TRACE, 4:OUTPUT_SYSTEM_TRACE, 8:OUTPUT_NETWORK

#export DALI_PERFORMANCE_TIMESTAMP_OUTPUT=4


# @ Enable log for checking performance
#  What performance statistics are logged out to dlog
#  see StatisticsLogOptions in performance-interface.h for values
#
#   0:DISABLED, 1:LOG_EVERYTHING, 2:LOG_UPDATE_RENDER, 4:LOG_EVENT_PROCESS, 8:LOG_CUSTOM_MARKERS

#export DALI_LOG_PERFORMANCE_STAT=1


# @ Allow control and monitoring of DALi via the network
#export DALI_NETWORK_CONTROL=1

# @ environment variable for enabling/disabling fps tracking
#export DALI_FPS_TRACKING=1
#export DALI_UPDATE_STATUS_INTERVAL=1
#export DALI_OBJECT_PROFILER_INTERVAL=1

# @ Set the the graphics status time
#export DALI_GLES_CALL_TIME=5

# @ Whether or not to accumulate gles call statistics
#export DALI_GLES_CALL_ACCUMULATE=1

# @ Set dali window status
#export DALI_WINDOW_WIDTH=400
#export DALI_WINDOW_HEIGHT=400
#export DALI_WINDOW_NAME=DALI_ENV_WINDOW
#export DALI_WINDOW_CLASS_NAME=DALI_ENV_CLASS

# @ Set the rate of render refresh
#export DALI_REFRESH_RATE=30

# @ Set the number of samples required in multisample buffers
#export DALI_MULTI_SAMPLING_LEVEL=4

# @ Set the maximum texture size that GL can handle
#export DALI_MAX_TEXTURE_SIZE=512

# @ Set the number of frames
#  The number of frames that are going to be rendered into the Frame Buffer Object
#  but the last one which is going to be rendered into the Frame Buffer.
#export DALI_RENDER_TO_FBO=2

# @ Disable depth buffer
#export DALI_DISABLE_DEPTH_BUFFER=1

# @ Disable stencil buffer
#export DALI_DISABLE_STENCIL_BUFFER=1

# @ Set the configuration of Pan-Gesture
# @ Prediction Modes 1 & 2:

# @ prediction mode for pan gestures
#export DALI_PAN_PREDICTION_MODE=1
#export DALI_PAN_PREDICTION_AMOUNT=1
#export DALI_PAN_SMOOTHING_MODE=1

# @ Prediction Mode 1:
#export DALI_PAN_MAX_PREDICTION_AMOUNT=1
#export DALI_PAN_MIN_PREDICTION_AMOUNT=1
#export DALI_PAN_PREDICTION_AMOUNT_ADJUSTMENT=1
#export DALI_PAN_SMOOTHING_AMOUNT=1

# @ Prediction Mode 2 :
#export DALI_PAN_USE_ACTUAL_TIMES=1
#export DALI_PAN_INTERPOLATION_TIME_RANGE=1
#export DALI_PAN_SCALAR_ONLY_PREDICTION_ENABLED=1
#export DALI_PAN_TWO_POINT_PREDICTION_ENABLED=1
#export DALI_PAN_TWO_POINT_PAST_INTERPOLATE_TIME=1
#export DALI_PAN_TWO_POINT_VELOCITY_BIAS=1
#export DALI_PAN_TWO_POINT_ACCELERATION_BIAS=1
#export DALI_PAN_MULTITAP_SMOOTHING_RANGE=1

# @ Pan-Gesture miscellaneous :
#export DALI_LOG_PAN_GESTURE=1
#export DALI_PAN_MINIMUM_DISTANCE=1
#export DALI_PAN_MINIMUM_EVENTS=1


# @ Enable filter for checking detail log
#  If you enable filter, you can check detail log.
#  but for use it, you need to build dali pkg using debug option
#
#   gbs build -A armv7l --include-al --define "%enable_debug 1"
#
#  After build and install dali pkg using debug option, you can enable the filter you want.
#  Most filters set 0 level, so you need to set 1~3 level using environment value.
#
#   enum LogLevel
#   {
#   NoLogging   = 0,
#   Concise     = 1,
#   General     = 2,
#   Verbose     = 3
#   };

#export LOG_KEYBOARD_FOCUS_MANAGER=3
#export LOG_TEXT_RENDERING=3
#export LOG_WINDOW=3


