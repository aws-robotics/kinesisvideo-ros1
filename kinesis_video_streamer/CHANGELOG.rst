^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package kinesis_video_streamer
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

2.0.1 (2019-03-29)
------------------
* Setting frame's trackId to the default for compatibility with newer v… (`#21 <https://github.com/aws-robotics/kinesisvideo-ros1/issues/21>`_)
  * Setting frame's trackId to the default for compatibility with newer version of the Kinesis Producer SDK.
  * Update version in package.xml
* Contributors: AAlon

2.0.3 (2020-09-01)
------------------
* Merge pull request `#49 <https://github.com/aws-robotics/kinesisvideo-ros1/issues/49>`_ from aws-robotics/master
  * Ignore test files in coverage calculation
  * Update README.md to install build tool (`#40 <https://github.com/aws-robotics/kinesisvideo-ros1/issues/40>`_)
  * Remove "using namespace std" from headers, even in tests (`#47 <https://github.com/aws-robotics/kinesisvideo-ros1/issues/47>`_)
  Co-authored-by: AAlon <avishaya@amazon.com>
  Co-authored-by: Abby Xu <30247381+xabxx@users.noreply.github.com>
* Remove "using namespace std" from headers, even in tests (`#47 <https://github.com/aws-robotics/kinesisvideo-ros1/issues/47>`_)
* Contributors: Emerson Knapp

2.0.2 (2019-08-01)
------------------
* increment patch version (`#38 <https://github.com/aws-robotics/kinesisvideo-ros1/issues/38>`_)
  Signed-off-by: Miaofei <miaofei@amazon.com>
* Remove conditions on ROS version in package.xml (`#35 <https://github.com/aws-robotics/kinesisvideo-ros1/issues/35>`_)
* Use standard CMake macros for adding gtest/gmock tests (`#33 <https://github.com/aws-robotics/kinesisvideo-ros1/issues/33>`_)
  * modify kinesis_video_streamer to use add_rostest_gmock()
  Signed-off-by: Miaofei <miaofei@amazon.com>
  * update travis.yml to be compatible with specifying multiple package names
  Signed-off-by: Miaofei <miaofei@amazon.com>
* Update changelog (`#23 <https://github.com/aws-robotics/kinesisvideo-ros1/issues/23>`_)
  * Release 2.0.1
  * 2.0.1
* Setting frame's trackId to the default for compatibility with newer v… (`#21 <https://github.com/aws-robotics/kinesisvideo-ros1/issues/21>`_)
  * Setting frame's trackId to the default for compatibility with newer version of the Kinesis Producer SDK.
  * Update version in package.xml
* Contributors: AAlon, M. M

2.0.0 (2019-03-20)
------------------
* Improve GMock dependency resolution in CMakeLists.txt
* Fix gmock dependency in CMakeLists.txt
* Remove extra space from subscription_installer
* Refactor StreamerNode class, add tests
  - Refactor the StreamerNode into its own class without the main()
  function so that it can have its own test suite.
  - Split Initialize function into two to be able to unit tests the initialization
  without mocking out the remote kinesis calls in stream setup.
  - Add more tests to improve code coverage.
* use log4cplus from apt
* Update to use non-legacy ParameterReader API (`#11 <https://github.com/aws-robotics/kinesisvideo-ros1/issues/11>`_)
* Update to use new ParameterReader API (`#10 <https://github.com/aws-robotics/kinesisvideo-ros1/issues/10>`_)
  * adjust usage of the ParameterReader API
  * remove unnecessary dependencies for travis build
  * increment major version number in package.xml
* Use separate node and stream config in example (`#5 <https://github.com/aws-robotics/kinesisvideo-ros1/issues/5>`_)
* Contributors: Avishay Alon, Juan Rodriguez Hortala, M. M, Miaofei, Ross Desmond, Tim Robinson
