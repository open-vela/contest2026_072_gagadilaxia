#include <iostream>
#include <string>
#include <cstring>
#include <curl/curl.h>
#include <unistd.h>
#include "agent_state.h"

const std::string MIMO_API_KEY = "";
const std::string MIMO_URL = "https://api.mimo.xiaomi.com/v2.5/chat/completions";
const std::string SYSTEM_PROMPT = R"(
你是MindFlow专注调节智能体，根据用户专注分数给出简短干预建议：
1.分数≥80：夸赞，鼓励保持专注
2.50≤分数<80：轻度分心，给出简单静心方法
3.分数<50：疲惫拖延，建议短时休息、拆分任务
回复精简，只输出建议文本，不要多余符号
)";

static size_t writeCallback(char *buf, size_t size, size_t nmemb, std::string *out)
{
    size_t total = size * nmemb;
    out->append(buf, total);
    return total;
}

std::string callMimoAPI(int score)
{
    CURL *curl = curl_easy_init();
    std::string resBody;
    if (!curl) return "curl初始化失败";

    std::string userMsg = "用户当前专注分数：" + std::to_string(score);
    std::string jsonReq = R"({
        "model":"mimo-v2.5",
        "messages":[
            {"role":"system","content":")" + SYSTEM_PROMPT + R"("},
            {"role":"user","content":")" + userMsg + R"("}
        ],
        "temperature":0.6
    })";

    struct curl_slist *headers = nullptr;
    std::string auth = "Authorization: Bearer " + MIMO_API_KEY;
    headers = curl_slist_append(headers, auth.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, MIMO_URL.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonReq.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resBody);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode ret = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (ret != CURLE_OK)
    {
        return "API请求失败，网络不通";
    }
    return resBody;
}

int main()
{
    AgentState agent;
    curl_global_init(CURL_GLOBAL_ALL);
while (true)
{
    agent.tick();

    int score = agent.getFocusScore();
    std::string state = agent.getStateName();

    std::cout << "当前专注分数：" << score << std::endl;
    std::cout << "当前状态：" << state << std::endl;

    if (agent.needLLMInfer())
    {
        std::cout << "检测到状态变化，准备调用 MiMo..." << std::endl;

        std::string resp = callMimoAPI(score);

        std::cout << "AI干预建议：" << resp
                  << "\n------------------------\n";

        agent.markLLMFinished();
    }

    sleep(2);
}    curl_global_cleanup();
    return 0;
}
