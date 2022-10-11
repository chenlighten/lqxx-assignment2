#include <api.h>
#include <users.h>
#include <googletest/googletest/include/gtest/gtest.h>
#include <json/include/nlohmann/json.hpp>

class MockedUsers: public Users {
public:
    ~MockedUsers() {}

    bool Create(const std::string& name, const std::string& email, const std::string& password) override {
        return create_mock_return;
    }

    bool Validate(const std::string& name, const std::string& email, const std::string& password) override {
        return validate_mock_return;
    }

    void SetCreateResult(bool value) {
        create_mock_return = value;
    }

    void SetValidateResult(bool value) {
        validate_mock_return = value;
    }

private:
    bool create_mock_return;
    bool validate_mock_return;
};

class APITest: public ::testing::Test {
protected:

    void SetUp() override {
        mocked_users = std::make_shared<MockedUsers>();
        api = std::make_shared<API>(mocked_users);
        running_server = std::make_shared<std::thread>([&](){this->api->Run(this->test_host, this->test_port);});
    }

    void TearDown() override {
        api->Stop();
        running_server->join();
    }

protected:
    std::shared_ptr<API> api;
    std::shared_ptr<MockedUsers> mocked_users;
    const std::string test_host = "0.0.0.0";
    const uint32_t test_port = 3301;
    std::shared_ptr<std::thread> running_server;
};

TEST_F(APITest, UsersRegister) {
    // const std::string test_path = test_host + ":" + std::to_string(test_port) + "/v1/users/register";

    {
        // wait for some time for the service to be set up
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        httplib::Client client(test_host, test_port);
        nlohmann::json req_body;
        req_body["name"] = "Alice";
        req_body["passwd"] = "123456";
        req_body["email"] = "alice@columbia.edu";
        mocked_users->SetCreateResult(true);
        auto result = client.Post("/v1/users/register", req_body.dump(), "text/plain");
        EXPECT_EQ(result.error(), httplib::Error::Success);
    }
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}