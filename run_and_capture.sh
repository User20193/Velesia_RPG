#!/bin/bash
xvfb-run -a --server-args="-screen 0 1024x768x24" bash -c "
  python3 game/main.py &
  GAME_PID=\$!
  sleep 5
  python3 -c \"import pyautogui; pyautogui.screenshot('/tmp/menu_screenshot.png')\"
  kill \$GAME_PID
"
