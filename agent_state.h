#ifndef AGENT_STATE_H
#define AGENT_STATE_H

#include <string>

enum class WorkStatus
{
    FOCUS,
    DISTRACTED,
    TIRED,
    IDLE
};

class AgentState
{
private:
    WorkStatus m_status;
    int m_focusScore;
    bool m_llmNeedCall;

public:
    AgentState();
    void tick();
    std::string getStateName() const;
    int getFocusScore() const;
    bool needLLMInfer() const;
    void markLLMFinished();
};

#endif
