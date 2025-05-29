#!/bin/bash


PROJECT_DIR=$(pwd)

VENV_PATH="${PROJECT_DIR}/venv"

PORT=8000

cleanup() {
    echo "[INFO] Script interrupted. Stopping server if running..."
    SERVER_PID=$(ps aux | grep "python3 -m http.server $PORT" | grep -v grep | awk '{print $2}')
    if [ -n "$SERVER_PID" ]; then
        echo "[INFO] Killing server with PID: $SERVER_PID"
        kill $SERVER_PID
    fi
    exit 1
}

trap cleanup INT


if [ -z "$1" ]; then
    echo "[ERROR] Usage: $0 <username>"
    exit 1
fi
USERNAME="$1"
echo "[INFO] Processing for user: $USERNAME"

# 檢查有無 py 虛擬環境，不存在則使用默認虛擬環境
if [ -d "$VENV_PATH" ] && [ -f "$VENV_PATH/bin/activate" ]; then
    echo "[INFO] Activating Python virtual environment..."
    source "$VENV_PATH/bin/activate"
else
    echo "[WARNING] Python virtual environment not found at $VENV_PATH. Assuming system Python."
fi

cd "$PROJECT_DIR" || { echo "[ERROR] Failed to change directory to $PROJECT_DIR"; exit 1; }

# 編譯
echo "[INFO] Compiling C program..."
make main
MAKE_EXIT_STATUS=$?
if [ $MAKE_EXIT_STATUS -ne 0 ]; then
    echo "[ERROR] Compilation (make) failed with status $MAKE_EXIT_STATUS."
    exit $MAKE_EXIT_STATUS
fi
echo "[INFO] Compilation finished."

# 運行
echo "[INFO] Running C program to generate CSV files for user '$USERNAME'..."
if [ ! -f "./main" ]; then
    echo "[ERROR] './main' executable not found. Please compile the C program first."
    exit 1
fi

./main "$USERNAME"
C_EXIT_STATUS=$?
if [ $C_EXIT_STATUS -ne 0 ]; then
    echo "[ERROR] C program exited with status $C_EXIT_STATUS."
    exit $C_EXIT_STATUS
fi
echo "[INFO] C program finished."

echo "[INFO] Running Python script to generate ECharts JSON for user '$USERNAME'..."
if [ ! -f "./gen.py" ]; then
    echo "[ERROR] './gen.py' script not found."
    exit 1
fi
python3 gen.py "$USERNAME"

PYTHON_EXIT_STATUS=$? 
if [ $PYTHON_EXIT_STATUS -ne 0 ]; then
    echo "[ERROR] Python script exited with status $PYTHON_EXIT_STATUS."
    exit $PYTHON_EXIT_STATUS
fi
echo "[INFO] Python script finished."

echo "[INFO] Starting HTTP server on port $PORT..."
python3 -m http.server "$PORT" & 
SERVER_PID=$! 
echo "[INFO] HTTP server started with PID: $SERVER_PID. Access at http://localhost:$PORT/"
echo "[INFO] HTML report will be opened at: http://localhost:$PORT/index.html?user=$USERNAME"

sleep 2

REPORT_URL="http://localhost:$PORT/index.html?user=$USERNAME"
echo "[INFO] Opening report in default browser: $REPORT_URL"

if [[ "$OSTYPE" == "linux-gnu"* ]]; then # linux
    xdg-open "$REPORT_URL" > /dev/null 2>&1 & 
elif [[ "$OSTYPE" == "darwin"* ]]; then # macos
    open "$REPORT_URL" &
else 
    echo "[INFO] Please open the URL manually in your browser: $REPORT_URL"
fi


echo ""
echo "[IMPORTANT] The HTTP server is running in the background (PID: $SERVER_PID)."
echo "[IMPORTANT] To stop the server, run: kill $SERVER_PID"
echo "[IMPORTANT] Or press Ctrl+C in this terminal if you remove the '&' from the server command (but then the script won't open the browser)."
echo "[IMPORTANT] Alternatively, after closing the browser tab, you can press Ctrl+C in THIS terminal ONCE to stop the script and the server (due to the trap)."

wait $SERVER_PID 2>/dev/null


