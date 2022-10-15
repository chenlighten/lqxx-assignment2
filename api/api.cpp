#include <api/api.h>
#include <common/utils.h>
#include <nlohmann/json.hpp>
#include <liboauthcpp/src/base64.h>
#include <jwt/jwt.hpp>

#define DefineHttpHandler(name)\
    void API::name(const httplib::Request& req, httplib::Response& res) noexcept

#define AddHttpHandler(server, path, method, func)\
    do {\
        (server)->method((path), \
        [this](const httplib::Request& req, httplib::Response& res){this->func(req, res);});\
    } while (false)

static inline bool DecodeEmailAndPasswordFromBasicAuth(
    const std::string& auth, std::string* email, std::string* password) noexcept {
    if (email == nullptr || password == nullptr) {
        return false;
    }

    const auto splited_auth = Common::Split(auth, " ");
    if (splited_auth.size() != 2 || splited_auth[0] != "Basic") {
        return false;
    }

    const auto email_password = Common::Split(base64_decode(splited_auth[1]), ":");
    if (email_password.size() != 2) {
        return false;
    }

    *email = email_password[0];
    *password = email_password[1];
    return true;
}

static inline std::string EncodeTokenFromEmail(
    const std::string& email,
    const std::chrono::seconds& expire_seconds,
    const std::string& secret_key) noexcept {

    jwt::jwt_object jwt_obj{
        jwt::params::algorithm("HS256"),
        jwt::params::secret(secret_key),
        jwt::params::payload({{"email", email}})};
    
    jwt_obj.add_claim("exp", std::chrono::system_clock::now() + expire_seconds);
    return jwt_obj.signature();
}

static inline std::string DecodeEmailFromToken(
    const std::string& token,
    const std::string& secret_key) noexcept {
    
    std::error_code err;
    const auto jwt_obj = jwt::decode(
        jwt::string_view(token),
        jwt::params::algorithms({"HS256"}),
        err,
        jwt::params::secret(secret_key),
        jwt::params::verify(true));
    
    // token not valid or expired
    if (err) {
        return {};
    }
    return jwt_obj.payload().get_claim_value<std::string>("email");
} 

static inline std::string DecodeTokenFromBasicAuth() noexcept {
    return {};
}

API::API(std::shared_ptr<Users> _users, std::shared_ptr<httplib::Server> _svr):
    users(_users), svr(_svr),
    token_secret_key(Common::RandomString(128)) {

    if (!users) {
        users = std::make_shared<Users>();
    }
    if (!svr) {
        svr = std::make_shared<httplib::Server>();
    }
}

API::~API() {
    Stop();
}

DefineHttpHandler(UsersRegister) {
    std::string user_name;
    std::string user_passwd;
    std::string user_email;
    nlohmann::json result;

    const auto auth_header = req.headers.find("Authentication");
    if (auth_header == req.headers.cend() ||
        !DecodeEmailAndPasswordFromBasicAuth(auth_header->second, &user_email, &user_passwd)) {
        result["msg"] = "failed";
        res.set_content(result.dump(), "text/plain");
    }
    
    try {
        const auto json_body = nlohmann::json::parse(req.body);
        user_name = json_body.at("name");
    } catch (std::exception& e) {
        res.set_content("Request body format error.", "text/plain");
        return;
    }

    // check if user email is duplicated
    if (users->DuplicatedEmail(user_email)) {
        result["msg"] = "failed";
        res.set_content(result.dump(), "text/plain");
        return;
    }

    // create user
    if (users->Create(user_name, user_email, user_passwd)) {
        result["msg"] = "success";
    } else {
        result["msg"] = "failed";
    }
    res.set_content(result.dump(), "text/plain");
}

DefineHttpHandler(UsersLogin) {
    std::string user_passwd;
    std::string user_email;
    nlohmann::json result;

    try {
        const auto json_body = nlohmann::json::parse(req.body);
        // to be modified: get from basic auth or some auth
        user_passwd = json_body.at("passwd");
        user_email = json_body.at("email");
    } catch (std::exception& e) {
        res.set_content("Request body format error.", "text/plain");
        return;
    }

    if (users->Validate({}, user_email, user_passwd)) {
        result["msg"] = "success";
        result["token"] = "some token";
    } else {
        result["msg"] = "failed";
    }

    res.set_content(result.dump(), "text/plain");
}

DefineHttpHandler(UsersLogout) {
    nlohmann::json result;
    const std::string token = req.headers.find("Authentication")->second;

    // do something with token
    const std::string decoded_token = base64_decode(token);
    // get user_id from decoded_token, format to be discussed
    // const std::string user_id = some_function(decoded_token);

    // do something with user_id
    
    result["msg"] = "success";
    res.set_content(result.dump(), "text/plain");
}

DefineHttpHandler(TaskLists) {
    const std::vector<std::string> splited_path = Common::Split(req.path, "/");

    // special resolve for "/v1/task_lists/{task_list_name}/tasks/{task_name}"
    if (splited_path.size() >= 2 && *(splited_path.rbegin() + 1) == "tasks") {
        Tasks(req, res);
        return;
    }

    nlohmann::json result;
    const std::string token = req.headers.find("Authentication")->second;

    // do something with token
    const std::string decoded_token = base64_decode(token);
    // get user_id from decoded_token, format to be discussed
    // const std::string user_id = some_function(decoded_token);

    // do something with user_id
    
    result["msg"] = "success";
    res.set_content(result.dump(), "text/plain");
}

DefineHttpHandler(TaskListsCreate) {
    // do it later
}

DefineHttpHandler(Tasks) {
    const std::vector<std::string> splited_path = Common::Split(req.path, "/");

    // some ugly handling for unresolved path

    if (splited_path.size() > 0 && splited_path.back() == "create") {
        TasksCreate(req, res);
        return;
    }

    if (splited_path.size() < 3) {
        // bad path
    }

    std::string user_id;
    std::string task_list_name = *(splited_path.rbegin() + 2);
    std::string task_name = splited_path.back();

    // do some logic
}

DefineHttpHandler(TasksCreate) {
    const std::vector<std::string> splited_path = Common::Split(req.path, "/");

    std::string user_id;
    std::string task_list_name = *(splited_path.rbegin() + 2);
    std::string task_name;

    // unfinised, do it later
}

void API::Run(const std::string& host, uint32_t port) {
    AddHttpHandler(svr, "/v1/users/register", Post, UsersRegister);
    AddHttpHandler(svr, "/v1/users/login", Post, UsersLogin);
    AddHttpHandler(svr, "/v1/users/logout", Post, UsersLogout);
    AddHttpHandler(svr, "/v1/task_lists/", Get, TaskLists);
    AddHttpHandler(svr, "/v1/task_lists/create", Post, TaskListsCreate);

    svr->listen(host, port);
}

void API::Stop() {
    if (svr && svr->is_running()) {
        svr->stop();
    }
}
