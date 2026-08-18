#include "agent_state.h"
#include <cstdlib>
#include <ctime>

AgentState::AgentState()
    : m_status(WorkStatus::IDLE), m_focusScore(60), m_llmNeedCall(false)
{
    srand((unsigned)time(nullptr));
}

void AgentState::tick()
{
    int delta = (rand() % 21) - 10;
    m_focusScore += delta;
    if(m_focusScore < 0) m_focusScore = 0;
    if(m_focusScore > 100) m_focusScore = 100;

    WorkStatus old = m_status;
    if(m_focusScore >=71)
        m_status = WorkStatus::FOCUS;
    else if(m_focusScore >=41)
        m_status = WorkStatus::DISTRACTED;
    else if(m_focusScore >=10)
        m_status = WorkStatus::TIRED;
    else
        m_status = WorkStatus::IDLE;

    if(old != m_status)
    {
        m_llmNeedCall = true;
    }
}

std::string AgentState::getStateName() const
{
    switch(m_status)
    {
        case WorkStatus::FOCUS: return "专注工作";
        case WorkStatus::DISTRACTED: return "分心走神";
        case WorkStatus::TIRED: return "疲劳状态";
        case WorkStatus::IDLE: return "空闲状态";
        default: return "未知";
    }
}

int AgentState::getFocusScore() const
{
    return m_focusScore;
}

bool AgentState::needLLMInfer() const
{
    return m_llmNeedCall;
}

void AgentState::markLLMFinished()
{
    m_llmNeedCall = false;
}
