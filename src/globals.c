#include "main.h"

/* ---------------- QUOTES ---------------- */
const char *LOGIN_QUOTES[] = {
    "If you can change your mind, you can change your life.",
    "Consistency creates greatness.",
    "A new day, a new list to conquer.",
    "Track your goals. Shape your success.",
    "Plan it. Do it. Achieve it.",
    "You can't change what you did before, but you can change what you do next.",
    "Let your smile change the world, but don't let the world change your smile.",
    "To change your life, you have to change yourself. To change yourself, you have to change your mindset."
};
const int LOGIN_QUOTES_N = sizeof(LOGIN_QUOTES)/sizeof(LOGIN_QUOTES[0]);

/* ---------------- GLOBALS (Definitions) ---------------- */
Task *head = NULL;
Action undo_stack[MAX_STACK]; int undo_top = 0;
Action redo_stack[MAX_STACK]; int redo_top = 0;
int next_id = 1;
int total_points = 0;
char current_user_id[64] = {0};
Task **schedule_arr = NULL;
int schedule_count = 0;
//specific variable for handling windows api
HWND hUser, hPass, hName, hUserSignup, hPassSignup;
HWND hTitle, hDesc, hCat, hDeadline, hPriority, hDeps;
HWND hGetIdInput;
HWND hViewList;
HWND hMain;
Task *editingTask = NULL;

enum AppState currentState = STATE_LOGIN;
int get_id_next_action = 0;