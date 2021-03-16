#pragma once

#include <aws/core/auth/AWSCredentialsProviderChain.h>
#include <aws_common/sdk_utils/auth/service_credentials_provider.h>
#include <kinesis-video-producer/Auth.h>

namespace Aws
{
	namespace Auth
	{
		/**
		 * Creates an AWSCredentialsProviderChain which uses in order IotRoleCredentialsProvider and EnvironmentAWSCredentialsProvider.
		 */
		class CustomAWSCredentialsProviderChain : public AWSCredentialsProviderChain
		{
		public:
			CustomAWSCredentialsProviderChain() = default;

			/**
			 * Initializes the provider chain with IotRoleCredentialsProvider and EnvironmentAWSCredentialsProvider in that order.
			 *
			 * @param config Configuration for available credential providers
			 */
			CustomAWSCredentialsProviderChain(const ServiceAuthConfig &config);
		};

	} // namespace Auth
} // namespace Aws


namespace Aws {
	namespace Kinesis {
		/**
 * Credentials provider which uses the AWS SDK's default credential provider chain.
 * @note You need to have called Aws::InitAPI before using this provider.
 */
		class CustomProducerSdkAWSCredentialsProvider : public com::amazonaws::kinesis::video::CredentialProvider
		{
		public:
			CustomProducerSdkAWSCredentialsProvider(std::shared_ptr<Aws::Auth::AWSCredentialsProvider>
													aws_credentials_provider = nullptr);
		private:
			std::shared_ptr<Aws::Auth::AWSCredentialsProvider> aws_credentials_provider_;

			void updateCredentials(com::amazonaws::kinesis::video::Credentials & producer_sdk_credentials) override;
		};

	}  // namespace Kinesis
}  // namespace Aws
