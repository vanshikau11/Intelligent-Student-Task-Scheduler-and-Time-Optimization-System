#ifndef GUI_H_
#define GUI_H_

#include "main.h"

/* Screen Changers */
void ShowLoginScreen(HWND hwnd);
void ShowDashboard(HWND hwnd);
void ShowAddTaskForm(HWND hwnd);
void ShowGetIdForm(HWND hwnd, const char* prompt, int next_action);
void ShowAnalyticsScreen(HWND hwnd);
void ShowViewTasksScreen(HWND hwnd);
void ShowScheduleScreen(HWND hwnd);
void ClearScreen(HWND hwnd);

/* Drawing */
void DrawGradientBackground(HDC hdc, RECT rc);

/* Misc */
void ensure_random();
void show_random_login_quote(HWND hwnd);


#endif // GUI_H_