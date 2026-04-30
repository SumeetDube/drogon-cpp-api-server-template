#pragma once

namespace signalutils
{
    /**
     * @brief Installs handlers for common signals.
     */
    void registerSignals();

    /**
     * @brief Polls the captured signal flag inside the Drogon event loop and triggers shutdown.
     */
    void attachDrogonSignalWatcher();

    /**
     * @brief Returns the most recently received signal (0 if none).
     */
    int getLastSignal();
}
