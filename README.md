# kinesis_video_streamer


## Overview
The Kinesis Video Streams ROS package enables robots to stream video to the cloud for analytics, playback, and archival use. Out of the box, the nodes provided make it possible to encode & stream image data (e.g. video feeds and LIDAR scans)
from a ROS “Image” topic to the cloud, enabling you to view the live video feed through the Kinesis Video Console, consume the stream via other applications, or perform intelligent analysis, face detection and face recognition
using Amazon Rekognition.

The node will transmit standard `sensor_msgs::Image` data from ROS topics to Kinesis Video streams, optionally encoding the images as h264 video frames along the way (using the included h264_video_encoder),
and optionally fetches Amazon Rekognition results from corresponding Kinesis Data Streams and publishing them to local ROS topics.

Note: h.264 hardware encoding is supported out of the box for OMX encoders and has been tested to
work on the Raspberry Pi 3. In all other cases, software encoding would be used, which is significantly more computing intensive and may affect overall system performance. If you wish to use a custom ffmpeg/libav encoder, you may
pass a `codec` ROS parameter to the encoder node (the name provided must be discoverable by [avcodec_find_encoder_by_name]). Certain scenarios may require offline caching of video streams which is not yet performed by this node.

**Amazon Kinesis Video Streams**: Amazon Kinesis Video Streams makes it easy to securely stream video from connected
devices to AWS for analytics, machine learning (ML), playback, and other processing. Kinesis Video Streams automatically provisions and elastically scales all the infrastructure needed to ingest streaming video data from millions
of devices. It also durably stores, encrypts, and indexes video data in your streams, and allows you to access your data through easy-to-use APIs. Kinesis Video Streams enables you to playback video for live and on-demand
viewing, and quickly build applications that take advantage of computer vision and video analytics through integration with Amazon Recognition Video, and libraries for ML frameworks such as Apache MxNet, TensorFlow, and OpenCV.

**Amazon Rekognition**: The easy-to-use Rekognition API allows you to automatically identify objects, people, text, scenes, and activities, as well as detect any inappropriate content. Developers can quickly build a searchable
content library to optimize media workflows, enrich recommendation engines by extracting text in images, or integrate secondary authentication into existing applications to enhance end-user security. With a wide variety of use
cases, Amazon Rekognition enables you to easily add the benefits of computer vision to your business.

**Keywords**: ROS, AWS, Kinesis Video Streams

### License
The source code is released under [Apache 2.0].

**Author**: AWS RoboMaker<br/>
**Affiliation**: [Amazon Web Services (AWS)]<br/>

RoboMaker cloud extensions rely on third-party software licensed under open-source licenses and are provided for demonstration purposes only. Incorporation or use of RoboMaker cloud extensions in connection with your production workloads or commercial product(s) or devices may affect your legal rights or obligations under the applicable open-source licenses. License information for this repository can be found [here](https://github.com/aws-robotics/kinesisvideo-ros1/blob/master/LICENSE). AWS does not provide support for this cloud extension. You are solely responsible for how you configure, deploy, and maintain this cloud extension in your workloads or commercial product(s) or devices.

### Supported ROS Distributions
- Kinetic
- Melodic

## Installation

### AWS Credentials
You will need to create an AWS Account and configure the credentials to be able to communicate with AWS services. You may find [AWS Configuration and Credential Files] helpful.

The IAM user will need permissions for the following actions:
- `kinesisvideo:CreateStream`
- `kinesisvideo:TagStream`
- `kinesisvideo:DescribeStream`
- `kinesisvideo:GetDataEndpoint`
- `kinesisvideo:PutMedia`

For [Amazon Rekognition] integration, the user will also need permissions for these actions:
- `kinesis:ListShards`
- `kinesis:GetShardIterator`
- `kinesis:GetRecords`

### Building from Source

To build from source you'll need to create a new workspace, clone and checkout the latest release branch of this repository, install all the dependencies, and compile. If you need the latest development features you can clone from the `master` branch instead of the latest release branch. While we guarantee the release branches are stable, __the `master` should be considered to have an unstable build__ due to ongoing development. 

- Install build tool: please refer to `colcon` [installation guide](https://colcon.readthedocs.io/en/released/user/installation.html)

- Create a ROS workspace and a source directory

        mkdir -p ~/ros-workspace/src

- Clone the package into the source directory . 

        cd ~/ros-workspace/src
        git clone https://github.com/aws-robotics/kinesisvideo-ros1.git -b release-latest

- Install dependencies

        cd ~/ros-workspace 
        sudo apt-get update && rosdep update
        rosdep install --from-paths src --ignore-src -r -y
        
_Note: If building the master branch instead of a release branch you may need to also checkout and build the master branches of the packages this package depends on._

- Build the packages

        cd ~/ros-workspace && colcon build

- Configure ROS library Path

        source ~/ros-workspace/install/setup.bash

- Build and run the unit tests

        colcon build --packages-select kinesis_video_streamer --cmake-target tests
        colcon test --packages-select kinesis_video_streamer kinesis_manager && colcon test-result --all


## Launch Files
A launch file called `kinesis_video_streamer.launch` is included in this package that gives an example of how to include a stream configuration file when configuring the parameter server for this node. The launch file uses the following arguments:

| Arg Name | Description |
| -------- | ----------- |
| stream_config | A path to a rosparam config file for the (first) stream. If not provided, the launch file will default to using the `sample_configuration.yaml` that was provided with this package. |

An example launch file called `sample_application.launch` is included in this project that gives an example of how you can include this node in your project and provide it with arguments.


## Usage

### Run the node
1. Configure the nodes (for more details, see the extended configuration section below).
  - Set up your AWS credentials and make sure you have the required IAM permissions.
  - Encoding: review [H264 Video Encoder sample configuration file] and pay attention to subscription_topic (camera output - expects a `sensor_msgs::Image` topic) and publication_topic.
  - Streaming: review [Kinesis Video Streamer sample configuration file] - make sure subscription_topic matches the encoder's publication_topic.
2. To use Amazon Rekognition for face detection and face recognition, follow the steps on the Rekognition guide (skip steps 8 & 9 as they are already performed by this node): https://docs.aws.amazon.com/rekognition/latest/dg/recognize-faces-in-a-video-stream.html
3. Example: running on a Raspberry Pi
  - `roslaunch `[`raspicam_node`]`camerav2_410x308_30fps.launch`
  - `roslaunch h264_video_encoder sample_application.launch`
  - `roslaunch kinesis_video_streamer sample_application.launch`
  - Log into your AWS Console to see the availabe Kinesis Video stream.
    - For other platforms, replace step 1 with an equivalent command to launch your camera node. Reconfigure the topic names accordingly.


## Configuration File and Parameters
Applies to the `kinesis_video_streamer` node. For configuring the encoder node, please see the README for the [H264 Video Encoder node]. An example configuration file called `stream_sample_configuration.yaml` is provided. When the parameters are absent in
the ROS parameter server, default values are used. Since this node makes HTTP requests to AWS endpoints, valid AWS credentials must be provided (this can be done via the environment variables `AWS_ACCESS_KEY_ID`, `AWS_SECRET_ACCESS_KEY` - see https://docs.aws.amazon.com/cli/latest/userguide/cli-environment.html).

### Node-wide configuration parameters
The parameters below apply to the node as a whole and are not specific to any one stream.

| Parameter Name | Description | Type |
| -------------- | -----------------------------------------------------------| ------------- |
| aws_client_configuration/region | The AWS region which the video should be streamed to. | *string* |
| kinesis_video/stream_count | The number of streams you wish to load and transmit. Each stream should have its corresponding parameter set as described below. | *int* |
| kinesis_video/log4cplus_config | (optional) Config file path for the log4cplus logger, which is used by the Kinesis Video Producer SDK. | *string* |

### Stream-specific configuration parameters
The parameters below should be provided per stream, with the prefix being `kinesis_video/stream<id>/<parameter name>`.

| Parameter Name | Description | Type |
| ------------- | -----------------------------------------------------------| ------------- |
| subscription_queue_size | (optional) The maximum number of incoming and outgoing messages to be queued towards the subscribed and publishing topics. | *int* |
| subscription_topic | Topic name to subscribe for the stream's input. | *string* |
| topic_type | Specifier for the transport protocol (message type) used. '1' for KinesisVideoFrame (supports h264 streaming), '2' for sensor_msgs::Image transport, '3' for KinesisVideoFrame with AWS Rekognition support. | *int* |
| stream_name | the name of the stream resource in AWS Kinesis Video Streams. | *string* |
| rekognition_data_stream | (optional - required if topic type == 3) The name of the Kinesis Data Stream from which AWS Rekognition analysis output should be read. | *string* |
| rekognition_topic_name | (optional - required if topic type == 3) The ROS topic to which the analysis results should be published. | *string* |

Additional stream-specific parameters such as frame_rate can be provided to further customize the stream definition structure. See [Kinesis header stream definition] for the remaining parameters and their default values.


## Performance and Benchmark Results
We evaluated the performance of this node by runnning the following scenario on a Raspberry Pi 3 Model B Plus connected to a Raspberry Pi camera module. The camera output was setup at a rate of 30 fps and resolution of 410x308 pixels, and encoded at a bitrate of 2mbps.
- Launch a baseline graph containing the talker and listener nodes from the [roscpp_tutorials package](https://wiki.ros.org/roscpp_tutorials), plus two additional nodes that collect CPU and memory usage statistics. Allow the nodes to run for 60 seconds.
- Following the instructions in the "Quick Start" section above, launch a `raspicam_node` node to get the images from the camera module, then launch a `h264_video_encoder` node to encode the images, and finally launch a `kinesis_video_streamer` node to send the image frames to the Amazon Kinesis Video Streams service. Allow the nodes to run for 180 seconds.
- Terminate the `raspicam_node`, `h264_video_encoder` and `kinesis_video_streamer` nodes, and allow the remaining nodes to run for 60 seconds.

The following graph shows the CPU usage during that scenario. After we start launching the kinesis nodes at second 60, the 1 minute average CPU usage increases from an initial 5.5% for the baseline graph up to a peak of 20.25%, and stabilizes around 15% until we stop the nodes around second 260. 

![cpu usage](wiki/images/cpu.svg)

The following graph shows the memory usage during that scenario. Free memory also accounts for additional memory available through a swap partition. After launching the kinesis nodes around second 60, the memory increases from the 292 MB for the baseline graph up to a peak of 392 MB (+34.25%), and stabilizes around 374 MB (+28.08% wrt. baseline graph). The memory usage goes down to 318 MB after stopping the kinesis nodes. 

![memory usage](wiki/images/memory.svg) 


## Node Details
Applies to the `kinesis_video_streamer` node; Please see the following README for encoder-specific configuration.
  - [H264 Video Encoder node]

### Subscribed Topics
The number of subscriptions is configurable and is determined by the `kinesis_video/stream_count` parameter. Each subscription is of the following form:

| Topic Name | Message Type | Description |
| ---------- | ------------ | ----------- |
| *Configurable* | *Configurable* (kinesis_video_msgs/KinesisVideoFrame or sensor_msgs/Image) | The node will subscribe to a topic of a given name. The data is expected to be either images (such as from a camera node publishing Image messages), or video frames (such as from an encoder node publishing KinesisVideoFrame messages). |

[`raspicam_node`]: https://github.com/UbiquityRobotics/raspicam_node
[Amazon Rekognition]: https://docs.aws.amazon.com/rekognition/latest/dg/streaming-video.html
[Amazon Web Services (AWS)]: https://aws.amazon.com/
[Apache 2.0]: https://aws.amazon.com/apache-2-0/
[avcodec_find_encoder_by_name]: https://ffmpeg.org/doxygen/2.7/group__lavc__encoding.html#gaa614ffc38511c104bdff4a3afa086d37
[AWS Configuration and Credential Files]: https://docs.aws.amazon.com/cli/latest/userguide/cli-config-files.html
[H264 Video Encoder node]: https://github.com/aws-robotics/kinesisvideo-encoder-ros1/blob/master/README.md
[H264 Video Encoder sample configuration file]: https://github.com/aws-robotics/kinesisvideo-encoder-ros1/blob/master/config/sample_configuration.yaml
[Issue Tracker]: https://github.com/aws-robotics/kinesisvideo-ros1/issues
[Kinesis header stream definition]: https://docs.aws.amazon.com/kinesisvideostreams/latest/dg/how-data.html#how-data-header-streamdefinition
[Kinesis Video Streamer sample configuration file]: kinesis_video_streamer/config/sample_configuration.yaml
[ROS]: http://www.ros.org
