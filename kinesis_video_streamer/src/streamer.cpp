/*
 * Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <log4cplus/configurator.h>

#include <aws/core/Aws.h>
#include <aws/core/utils/logging/LogMacros.h>
#include <aws_common/sdk_utils/aws_error.h>
#include <aws_common/sdk_utils/client_configuration_provider.h>
#include <aws_ros1_common/sdk_utils/logging/aws_ros_logger.h>
#include <aws_ros1_common/sdk_utils/ros1_node_parameter_reader.h>
#include <kinesis_video_streamer/ros_stream_subscription_installer.h>
#include <kinesis_video_streamer/streamer.h>
#include <kinesis_video_streamer/subscriber_callbacks.h>
#include <ros/ros.h>
#include <std_msgs/Bool.h>

using namespace Aws::Client;
using namespace Aws::Kinesis;


#ifndef RETURN_CODE_MASK
#define RETURN_CODE_MASK (0xff) /* Process exit code is in range (0, 255) */
#endif

#ifndef UNKNOWN_ERROR_KINESIS_VIDEO_EXIT_CODE
#define UNKNOWN_ERROR_KINESIS_VIDEO_EXIT_CODE (0xf0)
#endif

constexpr char kNodeName[] = "kinesis_video_streamer";
const char * kSpinnerThreadCountOverrideParameter = "spinner_thread_count";

namespace Aws {
namespace Kinesis {
  
StreamerNode::StreamerNode(const std::string & ns = std::string()) : ros::NodeHandle(ns)
{
  parameter_reader_ = std::make_shared<Ros1NodeParameterReader>();
  subscription_installer_ = std::make_shared<RosStreamSubscriptionInstaller>(*this);

  /* Log4cplus setup for the Kinesis Producer SDK */
  std::string log4cplus_config;
  parameter_reader_->ReadParam(
    GetKinesisVideoParameter(kStreamParameters.log4cplus_config), log4cplus_config);
  if (!log4cplus_config.empty()) {
    log4cplus::PropertyConfigurator::doConfigure(log4cplus_config);
  } else {
    log4cplus::BasicConfigurator configurator;
    configurator.configure();
  }
}

KinesisManagerStatus StreamerNode::Initialize()
{
  ClientConfigurationProvider configuration_provider(parameter_reader_);
  ClientConfiguration aws_sdk_config =
    configuration_provider.GetClientConfiguration();
  /* Set up subscription callbacks */
  if (!subscription_installer_->SetDefaultCallbacks()) {
    AWS_LOG_FATAL(__func__, "Failed to set up subscription callbacks.");
    return KINESIS_MANAGER_STATUS_ERROR_BASE;
  }
  auto kinesis_client = std::unique_ptr<KinesisClient>(
    Aws::New<Aws::Kinesis::KinesisClientFacade>(__func__, aws_sdk_config));
  stream_manager_ = std::make_shared<KinesisStreamManager>(
    parameter_reader_.get(), &stream_definition_provider_, subscription_installer_.get(),
    std::move(kinesis_client));
  subscription_installer_->set_stream_manager(stream_manager_.get());
  /* Initialization of video producer */
  KinesisManagerStatus initialize_video_producer_result =
    stream_manager_->InitializeVideoProducer(aws_sdk_config.region.c_str());
  if (KINESIS_MANAGER_STATUS_FAILED(initialize_video_producer_result)) {
    fprintf(stderr, "Failed to initialize video producer");
    AWS_LOGSTREAM_FATAL(__func__, "Failed to initialize video producer. Error code: "
                                    << initialize_video_producer_result);
    return initialize_video_producer_result;
  }
  // Set up command service server
  srv_command_ = advertiseService("command", &StreamerNode::CommandCb, this);
  AWS_LOGSTREAM_INFO(__func__, "Service server created: " << srv_command_.getService());
  // Enable on start if needed
  parameter_reader_->ReadParam(GetKinesisVideoParameter("enabled"), enabled_);
  pub_enabled_ = advertise<std_msgs::Bool>("enabled", 1, true); // Latched
  if (enabled_) {
    KinesisManagerStatus status = InitializeStreamSubscriptions();
    if (KINESIS_MANAGER_STATUS_FAILED(status)) {
      return status;
    }
  }
  PublishStatus(enabled_);

  return KINESIS_MANAGER_STATUS_SUCCESS;
}

KinesisManagerStatus StreamerNode::InitializeStreamSubscriptions() 
{
  /* Set up subscriptions and get ready to start streaming */
  KinesisManagerStatus streamer_setup_result = stream_manager_->KinesisVideoStreamerSetup();
  if (KINESIS_MANAGER_STATUS_SUCCEEDED(streamer_setup_result)) {
    AWS_LOG_DEBUG(__func__, "KinesisVideoStreamerSetup completed successfully.");
  } else {
    fprintf(stderr, "Failed to setup the kinesis video streamer");
    AWS_LOGSTREAM_ERROR(__func__, "KinesisVideoStreamerSetup failed with error code : "
                                    << streamer_setup_result << ". Exiting");
    return streamer_setup_result;
  }
  
  return KINESIS_MANAGER_STATUS_SUCCESS;
}

KinesisManagerStatus StreamerNode::UninitializeStreamSubscriptions()
{
  // Unsubscribe and free all Kinesis streams to stop streaming
  int video_stream_count = 0;
  parameter_reader_->ReadParam(GetKinesisVideoParameter(kStreamParameters.stream_count), video_stream_count);
  for (int stream_idx = 0; stream_idx < video_stream_count; stream_idx++) {
    std::string stream_name, topic_name;
    parameter_reader_->ReadParam(GetStreamParameterPath(stream_idx, kStreamParameters.stream_name), stream_name);
    parameter_reader_->ReadParam(GetStreamParameterPath(stream_idx, kStreamParameters.topic_name), topic_name);
    subscription_installer_->Uninstall(topic_name);
    stream_manager_->FreeStream(stream_name);
  }
  return KINESIS_MANAGER_STATUS_SUCCESS;
}

bool StreamerNode::CommandCb(kinesis_video_streamer::Command::Request &req, kinesis_video_streamer::Command::Response &res)
{
  // Request node shutdown if start/stop streaming failed
  if (!Command(req.enable)) {
    ros::requestShutdown();
    return false;
  }
  else
    return true;
}

bool StreamerNode::Command(bool enable)
{
  // Act only on mode switch
  if (enable == enabled_)
    return true;

  KinesisManagerStatus status;
  // Enable
  if (enable) {
    status = InitializeStreamSubscriptions(); // Can partially succeed, but returned status will be failed
  }
  // Disable
  else {
    status = UninitializeStreamSubscriptions();
  }

  // Set enabled if succeeded only
  if (KINESIS_MANAGER_STATUS_SUCCEEDED(status)) {
    enabled_ = enable;
    PublishStatus(enabled_);
    return true;
  }
  else
    return false;
}

void StreamerNode::PublishStatus(bool enable)
{
  std_msgs::Bool msg;
  msg.data = enable;
  pub_enabled_.publish(msg);
}

void StreamerNode::Spin()
{
  uint32_t spinner_thread_count = kDefaultNumberOfSpinnerThreads;
  int spinner_thread_count_input;
  if (Aws::AwsError::AWS_ERR_OK ==
      parameter_reader_->ReadParam(ParameterPath(kSpinnerThreadCountOverrideParameter),
                                 spinner_thread_count_input)) {
    spinner_thread_count = static_cast<uint32_t>(spinner_thread_count_input);
  }
  ros::MultiThreadedSpinner spinner(spinner_thread_count);
  spinner.spin();
}

void StreamerNode::set_subscription_installer(std::shared_ptr<RosStreamSubscriptionInstaller> subscription_installer)
{
  subscription_installer_ = subscription_installer;
}

}  // namespace Kinesis
}  // namespace Aws