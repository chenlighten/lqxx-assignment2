#include <tasklists/tasklistsWorker.h>
#include <api/tasklistContent.h>
#include <db/DB.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

class MockedDB : public DB {
public:
    MOCK_METHOD(returnCode, 
                createTaskListNode, 
                (const std::string &user_pkey, (const std::map<std::string, std::string> &) task_list_info), 
                (override));
    MOCK_METHOD(returnCode,
                getTaskListNode,
                (const std::string &user_pkey,
                 const std::string &task_list_pkey,
                 (std::map<std::string, std::string> &) task_list_info),
                (override));
    MOCK_METHOD(returnCode,
                deleteTaskListNode,
                (const std::string &user_pkey,
                 const std::string &task_list_pkey),
                 (override));
    MOCK_METHOD(returnCode,
                reviseTaskListNode,
                (const std::string &user_pkey,
                const std::string &task_list_pkey,
                (const std::map<std::string, std::string> &) task_list_info),
                (override));
    
    MockedDB() : DB("localhost") {}
};

class TaskListTest : public ::testing::Test {
protected:

    void SetUp() override {
        tasklistsWorker = std::make_shared<TaskListsWorker>(mockedDB);
    }

    void TearDown() override {
        // pass
    }

    MockedDB mockedDB;
    std::shared_ptr<TaskListsWorker> tasklistsWorker;
    RequestData data;
    TasklistContent in;
    TasklistContent out;

};

using namespace ::testing;

TEST_F(TaskListTest, Query) {
    // setup input
    data.user_key = "user0";
    data.tasklist_key = "tasklist0";
    out = TasklistContent();
    std::map<std::string, std::string> task_list_info;
    std::map<std::string, std::string> new_task_list_info = {{"name", "tasklist0"}, 
                                                             {"content", "this is tasklist #0"},
                                                             {"date", "10/15/2022"}};

    // normal get, should be successful
    EXPECT_CALL(mockedDB, getTaskListNode(data.user_key, data.tasklist_key, task_list_info))
        .WillOnce(DoAll(SetArgReferee<2>(new_task_list_info),Return(SUCCESS)));
    EXPECT_EQ(tasklistsWorker->Query(data, out), SUCCESS);
    EXPECT_EQ(out.name, "tasklist0");
    EXPECT_EQ(out.content, "this is tasklist #0");
    EXPECT_EQ(out.date, "10/15/2022");

    // request tasklist_key is empty 
    data.user_key = "user0";
    data.tasklist_key = "";
    out = TasklistContent();
    EXPECT_EQ(tasklistsWorker->Query(data, out), ERR_KEY);
    EXPECT_EQ(out.name, "");
    EXPECT_EQ(out.content, "");
    EXPECT_EQ(out.date, "");

    // request user_key is empty
    data.user_key = "";
    data.tasklist_key = "";
    out = TasklistContent();
    EXPECT_EQ(tasklistsWorker->Query(data, out), ERR_KEY);
    EXPECT_EQ(out.name, "");
    EXPECT_EQ(out.content, "");
    EXPECT_EQ(out.date, "");

    // TODO: test for user DNE
}

TEST_F(TaskListTest, Create) {
    // setup input
    data.user_key = "user0";
    data.tasklist_key = "tasklist0";
    std::string name = "tasklist0";
    std::string content = "this is tasklist #0";
    std::string date = "10/15/2022";
    in = TasklistContent(name, content, date);

    std::map<std::string, std::string> task_list_info;
    task_list_info["name"] = name;
    task_list_info["content"] = content;
    task_list_info["date"] = date;
    std::string outName;

    // normal create, should be successful
    EXPECT_CALL(mockedDB, createTaskListNode(data.user_key, task_list_info))
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(tasklistsWorker->Create(data, in, outName), SUCCESS);
    EXPECT_EQ(outName, "tasklist0");

    // request tasklist_key is empty, should be successful since task name is provided in tasklistContent
    data.user_key = "user0";
    data.tasklist_key = "";
    outName = "";
    EXPECT_CALL(mockedDB, createTaskListNode(data.user_key, task_list_info))
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(tasklistsWorker->Create(data, in, outName), SUCCESS);
    EXPECT_EQ(outName, "tasklist0");

    // request user_key is empty
    data.user_key = "";
    data.tasklist_key = "";
    outName = "";
    EXPECT_EQ(tasklistsWorker->Create(data, in, outName), ERR_KEY);
    EXPECT_EQ(outName, "");

    // multiple calls to createTaskListNode if previous name is duplicated
    data.user_key = "user0";
    data.tasklist_key = "tasklist0";
    outName = "";
    EXPECT_CALL(mockedDB, createTaskListNode(data.user_key, task_list_info))
        .WillOnce(Return(ERR_DUP_NODE));
    task_list_info["name"] = "tasklist0(1)";
    EXPECT_CALL(mockedDB, createTaskListNode(data.user_key, task_list_info))
        .WillOnce(Return(ERR_DUP_NODE));
    task_list_info["name"] = "tasklist0(2)";
    EXPECT_CALL(mockedDB, createTaskListNode(data.user_key, task_list_info))
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(tasklistsWorker->Create(data, in, outName), SUCCESS);
    EXPECT_EQ(outName, "tasklist0(2)");

    // TODO: test for user DNE
}

TEST_F(TaskListTest, Delete) {
    // setup input
    data.user_key = "user0";
    data.tasklist_key = "tasklist0";

    // normal delete, should be successful
    EXPECT_CALL(mockedDB, deleteTaskListNode(data.user_key, data.tasklist_key))
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(tasklistsWorker->Delete(data), SUCCESS);
    
    // request tasklist_key is empty 
    data.user_key = "user0";
    data.tasklist_key = "";
    EXPECT_EQ(tasklistsWorker->Delete(data), ERR_KEY);

    // request user_key is empty
    data.user_key = "";
    data.tasklist_key = "";
    EXPECT_EQ(tasklistsWorker->Delete(data), ERR_KEY);

    // TODO: test for user DNE
}

TEST_F(TaskListTest, Revise) {
    // setup input
    data.user_key = "user0";
    data.tasklist_key = "tasklist0";

    std::string newName = "tasklist1";
    std::string newContent = "this is tasklist #1";
    std::string newDate = "16/10/2022";
    in = TasklistContent(newName, newContent, newDate);

    std::map<std::string, std::string> task_list_info;
    task_list_info["name"] = newName;
    task_list_info["content"] = newContent;
    task_list_info["date"] = newDate;

    // normal revise, should be successful
    EXPECT_CALL(mockedDB, reviseTaskListNode(data.user_key, data.tasklist_key, task_list_info))
        .WillOnce(Return(SUCCESS));
    EXPECT_EQ(tasklistsWorker->Revise(data, in), SUCCESS);
    
    // request tasklist_key empty
    data.tasklist_key = "";
    EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_KEY);

    // request user_key empty
    data.user_key = "";
    data.tasklist_key = "";
    EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_KEY);

    // fields are empty
    data.user_key = "user0";
    data.tasklist_key = "tasklist0";
    in = TasklistContent();
    task_list_info.clear();
    EXPECT_CALL(mockedDB, reviseTaskListNode(data.user_key, data.tasklist_key, task_list_info))
        .WillOnce(Return(ERR_RFIELD));
    EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_RFIELD);

    // user not exist
    data.user_key = "not_exist_user";
    data.tasklist_key = "tasklist0";
    EXPECT_CALL(mockedDB, reviseTaskListNode(data.user_key, data.tasklist_key, task_list_info))
        .WillOnce(Return(ERR_NO_NODE));
    EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_NO_NODE);

    // tasklist not exist
    data.user_key = "user0";
    data.tasklist_key = "not_exist_tasklist";
    EXPECT_CALL(mockedDB, reviseTaskListNode(data.user_key, data.tasklist_key, task_list_info))
        .WillOnce(Return(ERR_NO_NODE));
    EXPECT_EQ(tasklistsWorker->Revise(data, in), ERR_NO_NODE);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

