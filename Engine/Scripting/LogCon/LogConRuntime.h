/**
 * @file LogConRuntime.h
 * @brief Временная заглушка для LogCon Runtime.
 *
 * Полноценный рантайм будет реализован по мере восстановления
 * интерпретатора и игровых API. Текущая версия лишь сообщает
 * пользователю, что исполнение LogCon пока недоступно.
 */

#pragma once

#include <string>

namespace SAGE::Scripting::LogCon {

class LogConRuntime {
public:
    explicit LogConRuntime(std::string scriptPath);

    /**
     * @brief Запускает прототип рантайма.
     * @return Код выхода приложения.
     */
    int Run();

private:
    std::string m_ScriptPath;
};

} // namespace SAGE::Scripting::LogCon
