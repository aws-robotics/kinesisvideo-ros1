#include <aws/core/platform/Environment.h>
#include <aws/core/utils/logging/LogMacros.h>

#include <kinesis_video_streamer/credentials.h>

using namespace Aws::Auth;

/// Logging tag used for all messages emitting from this module
static const char AWS_LOG_TAG[] = "CustomAWSCredentialsProviderChain";
static const char AWS_ECS_CONTAINER_CREDENTIALS_RELATIVE_URI[] = "AWS_CONTAINER_CREDENTIALS_RELATIVE_URI";
static const char AWS_ECS_CONTAINER_CREDENTIALS_FULL_URI[] = "AWS_CONTAINER_CREDENTIALS_FULL_URI";
static const char AWS_ECS_CONTAINER_AUTHORIZATION_TOKEN[] = "AWS_CONTAINER_AUTHORIZATION_TOKEN";
static const char AWS_EC2_METADATA_DISABLED[] = "AWS_EC2_METADATA_DISABLED";

namespace Aws
{
  namespace Auth
  {

    /**
     * \brief Validates an instance of an IotRoleConfig struct
     * @param config The struct to validate
     * @return True if the struct is valid, meaning all the config needed to connect is there
     */
    static bool IsIotConfigValid(const IotRoleConfig & config)
    {
      return config.cafile.length() > 0 && config.certfile.length() > 0 &&
          config.keyfile.length() > 0 && config.host.length() > 0 && config.role.length() > 0 &&
          config.name.length() > 0 && config.connect_timeout_ms > 0 && config.total_timeout_ms > 0;
    }

    CustomAWSCredentialsProviderChain::CustomAWSCredentialsProviderChain(const ServiceAuthConfig &config):
      AWSCredentialsProviderChain()
    {
      // Add IoT credentials provider if valid
      if (IsIotConfigValid(config.iot)) {
        AWS_LOG_INFO(AWS_LOG_TAG, "Found valid IoT auth config, adding IotRoleCredentialsProvider");
        auto provider = Aws::MakeShared<IotRoleCredentialsProvider>(__func__, config.iot);
        AddProvider(provider);
      } else {
        AWS_LOG_INFO(AWS_LOG_TAG, "No valid IoT auth config, skipping IotRoleCredentialsProvider");
      }

      // Add environment credentials provider
      AddProvider(Aws::MakeShared<EnvironmentAWSCredentialsProvider>(AWS_LOG_TAG));
      AddProvider(Aws::MakeShared<ProfileConfigFileAWSCredentialsProvider>(AWS_LOG_TAG));

      //ECS TaskRole Credentials only available when ENVIRONMENT VARIABLE is set
      const auto relativeUri = Aws::Environment::GetEnv(AWS_ECS_CONTAINER_CREDENTIALS_RELATIVE_URI);
      AWS_LOGSTREAM_DEBUG(AWS_LOG_TAG, "The environment variable value " << AWS_ECS_CONTAINER_CREDENTIALS_RELATIVE_URI
                          << " is " << relativeUri);

      const auto absoluteUri = Aws::Environment::GetEnv(AWS_ECS_CONTAINER_CREDENTIALS_FULL_URI);
      AWS_LOGSTREAM_DEBUG(AWS_LOG_TAG, "The environment variable value " << AWS_ECS_CONTAINER_CREDENTIALS_FULL_URI
                          << " is " << absoluteUri);

      const auto ec2MetadataDisabled = Aws::Environment::GetEnv(AWS_EC2_METADATA_DISABLED);
      AWS_LOGSTREAM_DEBUG(AWS_LOG_TAG, "The environment variable value " << AWS_EC2_METADATA_DISABLED
                          << " is " << ec2MetadataDisabled);

      if (!relativeUri.empty())
      {
        AddProvider(Aws::MakeShared<TaskRoleCredentialsProvider>(AWS_LOG_TAG, relativeUri.c_str()));
        AWS_LOGSTREAM_INFO(AWS_LOG_TAG, "Added ECS metadata service credentials provider with relative path: ["
                           << relativeUri << "] to the provider chain.");
      }
      else if (!absoluteUri.empty())
      {
        const auto token = Aws::Environment::GetEnv(AWS_ECS_CONTAINER_AUTHORIZATION_TOKEN);
        AddProvider(Aws::MakeShared<TaskRoleCredentialsProvider>(AWS_LOG_TAG,
                                                                 absoluteUri.c_str(), token.c_str()));

        //DO NOT log the value of the authorization token for security purposes.
        AWS_LOGSTREAM_INFO(AWS_LOG_TAG, "Added ECS credentials provider with URI: ["
                           << absoluteUri << "] to the provider chain with a" << (token.empty() ? "n empty " : " non-empty ")
                           << "authorization token.");
      }
      else if (Aws::Utils::StringUtils::ToLower(ec2MetadataDisabled.c_str()) != "true")
      {
        AddProvider(Aws::MakeShared<InstanceProfileCredentialsProvider>(AWS_LOG_TAG));
        AWS_LOGSTREAM_INFO(AWS_LOG_TAG, "Added EC2 metadata service credentials provider to the provider chain.");
      }
    }

  } // namespace Auth
} // namespace Aws

namespace Aws {
  namespace Kinesis {
    /**
 * Credentials provider which uses the AWS SDK's default credential provider chain.
 * @note You need to have called Aws::InitAPI before using this provider.
 */
    CustomProducerSdkAWSCredentialsProvider::CustomProducerSdkAWSCredentialsProvider(
        std::shared_ptr<Aws::Auth::AWSCredentialsProvider> aws_credentials_provider)
    {
      if (aws_credentials_provider) {
        aws_credentials_provider_ = aws_credentials_provider;
      } else {
        aws_credentials_provider_ =
          Aws::MakeShared<Aws::Auth::DefaultAWSCredentialsProviderChain>(__func__);
      }
    }

    void CustomProducerSdkAWSCredentialsProvider::updateCredentials(
        com::amazonaws::kinesis::video::Credentials & producer_sdk_credentials)
    {
      Aws::Auth::AWSCredentials aws_sdk_credentials =
        aws_credentials_provider_->GetAWSCredentials();
      producer_sdk_credentials.setAccessKey(aws_sdk_credentials.GetAWSAccessKeyId().c_str());
      producer_sdk_credentials.setSecretKey(aws_sdk_credentials.GetAWSSecretKey().c_str());
      producer_sdk_credentials.setSessionToken(aws_sdk_credentials.GetSessionToken().c_str());
      auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch());
      auto refresh_interval = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::milliseconds(Aws::Auth::REFRESH_THRESHOLD));
      producer_sdk_credentials.setExpiration(now + refresh_interval);
    }

  }  // namespace Kinesis
}  // namespace Aws
