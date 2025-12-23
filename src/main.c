    
#include "main.h"
#include <commctrl.h>

/* ---------------- Main WinMain ---------------- */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    ensure_data_folder();
    init_undo_system();

    // -- Initialize Common Controls (FOR CALENDAR AND LISTVIEW) --
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_DATE_CLASSES | ICC_LISTVIEW_CLASSES; // Register Calendar AND Table controls
    InitCommonControlsEx(&icex);
    // ------------------------------------------------

    const char CLASS_NAME[] = "TaskSchedulerWindowClass";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    hMain = CreateWindowEx(0, CLASS_NAME, "Task Scheduler & Optimizer - Final", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, 680, 560, 
                           NULL, NULL, hInstance, NULL);
    if (!hMain) return 0;
    ShowWindow(hMain, nCmdShow);

    MSG msg = {0}; while (GetMessage(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); }
    return 0;
}

/* ---------------- Message handling & UI logic ---------------- */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
 
        case WM_CREATE:
            ShowLoginScreen(hwnd);
            AnimateWindow(hwnd, 350, AW_BLEND);
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
            if (!dis) break;
            if (dis->CtlType == ODT_BUTTON) {
                char buf[128] = {0};
                GetWindowTextA(dis->hwndItem, buf, sizeof(buf));
                COLORREF bg = RGB(200, 200, 255);
                if (dis->itemState & ODS_SELECTED) bg = RGB(180, 180, 230);
                HBRUSH br = CreateSolidBrush(bg);
                FillRect(dis->hDC, &dis->rcItem, br);
                DeleteObject(br);
                HPEN p = CreatePen(PS_SOLID, 1, RGB(30,30,30));
                HPEN oldp = (HPEN)SelectObject(dis->hDC, p);
                Rectangle(dis->hDC, dis->rcItem.left, dis->rcItem.top, dis->rcItem.right, dis->rcItem.bottom);
                SelectObject(dis->hDC, oldp);
                DeleteObject(p);
                SetBkMode(dis->hDC, TRANSPARENT);
                SetTextColor(dis->hDC, RGB(0,0,0));
                DrawTextA(dis->hDC, buf, -1, &dis->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                return TRUE;
            }
            break;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                // --- AUTHENTICATION ---
                case ID_SUBMIT_LOGIN: {
                    char user[64] = {0}, pass[64] = {0};
                    GetWindowTextA(hUser, user, 64); 
                    GetWindowTextA(hPass, pass, 64);

                    if (checkCredentials(user, pass)) {
                        if (head) { free_all_tasks(head); head = NULL; }
                        strncpy(current_user_id, user, 63);
                        current_user_id[63] = '\0';

                        load_tasks_from_file(&head, current_user_id);
                        load_points(current_user_id);

                        ShowDashboard(hwnd);
                        show_random_login_quote(hwnd);
                    }
                    else MessageBox(hwnd, "User not found or incorrect password.", "Login Failed", MB_OK | MB_ICONERROR);
                    break;
                }

                case ID_SUBMIT_SIGNUP: {
                    char name[64] = {0}, user[64] = {0}, pass[64] = {0};
                    GetWindowTextA(hName, name, 64); 
                    GetWindowTextA(hUserSignup, user, 64); 
                    GetWindowTextA(hPassSignup, pass, 64);

                    if (!isValidName(name)) {
                        MessageBox(hwnd, "Invalid Name.\nName must contain only letters (A-Z, a-z) and spaces.", "Signup Failed", MB_OK | MB_ICONERROR);
                        break;
                    }
                    if (strlen(user) == 0) {
                        MessageBox(hwnd, "User ID cannot be empty.", "Signup Failed", MB_OK | MB_ICONERROR);
                        break;
                    }
                    if (!isValidPassword(pass)) {
                        MessageBox(hwnd, "Invalid Password.\nPassword must be:\n - At least 8 characters long\n - EX-Abcd123@", "Signup Failed", MB_OK | MB_ICONERROR);
                        break;
                    }

                    if (idExists(user)) {
                        MessageBox(hwnd, "This User ID already exists.", "Signup Failed", MB_OK | MB_ICONERROR);
                    } else { 
                        addUser(name, user, pass); 
                        MessageBox(hwnd, "User registered successfully!", "Success", MB_OK); 
                        ShowLoginScreen(hwnd); 
                    }
                    break;
                }

                // --- DASHBOARD ACTIONS ---
                case ID_DASHBOARD_ADD: ShowAddTaskForm(hwnd); break;
                
                case ID_DASHBOARD_VIEW: {
                    ShowViewTasksScreen(hwnd); // Opens the new Table View
                    break;
                }

                case ID_DASHBOARD_EDIT: {
                    if (!head) { MessageBox(hwnd, "No tasks to edit.", "Info", MB_OK); break; }
                    ShowGetIdForm(hwnd, "Enter Task ID to Edit:", ID_DASHBOARD_EDIT);
                    break;
                }

                case ID_DASHBOARD_COMPLETE:
                    if (!head) { MessageBox(hwnd, "No tasks to mark.", "Info", MB_OK); break; }
                    ShowGetIdForm(hwnd, "Enter Task ID to mark as complete:", ID_DASHBOARD_COMPLETE);
                    break;
                case ID_DASHBOARD_DELETE:
                    if (!head) { MessageBox(hwnd, "No tasks to delete.", "Info", MB_OK); break; }
                    ShowGetIdForm(hwnd, "Enter Task ID to delete:", ID_DASHBOARD_DELETE);
                    break;
                
                case ID_DASHBOARD_SCHEDULE: {
                    if (!head) { MessageBox(hwnd, "No tasks to schedule.", "Info", MB_OK); break; }
                    int count=0; Task **sched = generate_schedule(head, &count);
                    if (!sched || count==0) { if (sched) free(sched); MessageBox(hwnd,"No pending tasks to schedule.","Schedule",MB_OK); break; }
                    char *buffer = (char*)calloc(32768, sizeof(char));
                    if (!buffer) { MessageBox(hwnd, "Failed to allocate memory for schedule.", "Error", MB_OK|MB_ICONERROR); free(sched); break; }
                    safe_strncat(buffer,"--- SCHEDULED TASKS (Sorted by Deadline, then Priority) ---\n\n", 32768);
                    for (int i=0;i<count;i++){
                        char depsbuf[256] = {0};
                        if (sched[i]->depCount==0) strcpy(depsbuf,"None");
                        else{ for (int d=0; d<sched[i]->depCount; ++d) { char t[16]; snprintf(t,sizeof(t),"%d",sched[i]->dependencyIDs[d]); safe_strncat(depsbuf,t,sizeof(depsbuf)); if (d < sched[i]->depCount-1) safe_strncat(depsbuf,",",sizeof(depsbuf)); } }
                        char task_str[512];
                        snprintf(task_str,sizeof(task_str), "%d. %s | Prio:%d | Deadline:%s | Depends on: %s\n", sched[i]->id, sched[i]->title, sched[i]->priority, sched[i]->deadline, depsbuf);
                        safe_strncat(buffer, task_str, 32768);
                    }
                    MessageBoxA(hwnd, buffer, "Generated Schedule", MB_OK);
                    free(sched);
                    free(buffer);
                    break;
                }

                case ID_DASHBOARD_CONFLICTS:
                    detect_conflicts_msgbox(head);
                    break;
                case ID_DASHBOARD_SUMMARY:
                    save_weekly_summary(head, current_user_id);
                    break;
                case ID_DASHBOARD_ANALYTICS:
                    ShowAnalyticsScreen(hwnd);
                    break;
                case ID_DASHBOARD_UNDO: {
                    Action a; if (!pop_undo(&a)) { MessageBox(hwnd,"No action to undo.","Undo",MB_OK|MB_ICONINFORMATION); break; }
                    switch (a.type) {
                        case ACTION_ADD: { Task *t = remove_task(&head, a.snapshot.id); if (t) free(t); break; }
                        case ACTION_DELETE: { int deps[MAX_DEP_PER_TASK]={0}; for (int i=0;i<a.snapshot.depCount && i<MAX_DEP_PER_TASK;i++) deps[i]=a.snapshot.dependencyIDs[i]; add_task_with_id(&head, a.snapshot.id, a.snapshot.title, a.snapshot.description, a.snapshot.category, a.snapshot.deadline, a.snapshot.priority, a.snapshot.status, a.snapshot.depCount, deps); break; }
                        case ACTION_UPDATE: { Task *t = find_task_by_id(head, a.snapshot.id); if (t) t->status = a.snapshot.status; break; }
                    }
                    push_redo(a);
                    save_tasks_to_file(head, current_user_id);
                    MessageBox(hwnd,"Undo successful.","Undo",MB_OK);
                    break;
                }
                case ID_DASHBOARD_REDO: {
                    Action a; if (!pop_redo(&a)) { MessageBox(hwnd,"No action to redo.","Redo",MB_OK|MB_ICONINFORMATION); break; }
                    switch (a.type) {
                        case ACTION_ADD: { int deps[MAX_DEP_PER_TASK]={0}; for (int i=0;i<a.snapshot.depCount && i<MAX_DEP_PER_TASK;i++) deps[i]=a.snapshot.dependencyIDs[i]; add_task_with_id(&head, a.snapshot.id, a.snapshot.title, a.snapshot.description, a.snapshot.category, a.snapshot.deadline, a.snapshot.priority, a.snapshot.status, a.snapshot.depCount, deps); break; }
                        case ACTION_DELETE: { Task *t = remove_task(&head, a.snapshot.id); if (t) free(t); break; }
                        case ACTION_UPDATE: { Task *t = find_task_by_id(head, a.snapshot.id); if (t) t->status = 1; break; }
                    }
                    push_undo(a);
                    save_tasks_to_file(head, current_user_id);
                    MessageBox(hwnd,"Redo successful.","Redo",MB_OK);
                    break;
                }
                case ID_DASHBOARD_EXIT: DestroyWindow(hwnd); break;

                // --- ADD TASK ACTIONS ---
                case ID_ADD_TASK_SUBMIT: {
                    char title[MAX_TITLE] = {0}, desc[MAX_DESC] = {0}, cat[MAX_CATEGORY] = {0}, deadline[20] = {0}, prio_str[8] = {0}, deps_str[512] = {0};
                    char deps_str_copy[512] = {0};
                    
                    GetWindowTextA(hTitle, title, MAX_TITLE); 
                    GetWindowTextA(hDesc, desc, MAX_DESC); 
                    GetWindowTextA(hCat, cat, MAX_CATEGORY); 
                    GetWindowTextA(hPriority, prio_str, 8); 
                    GetWindowTextA(hDeps, deps_str, 512);

                    SYSTEMTIME st = {0};
                    SendMessage(hDeadline, MCM_GETCURSEL, 0, (LPARAM)&st);
                    snprintf(deadline, 20, "%d-%02d-%02d", st.wYear, st.wMonth, st.wDay);

                    strncpy(deps_str_copy, deps_str, sizeof(deps_str_copy)-1);
                    int priority = atoi(prio_str); if (priority < 1 || priority > 3) priority = 2;
                    Task *t = add_task(&head, title, desc, cat, deadline, priority);
                    if (t) {
                        int cnt = 0;
                        char *tok = strtok(deps_str_copy, ", ");
                        int invalid_dep = 0;
                        char invalid_msg[1024] = "Invalid dependency IDs:\n";
                        while (tok && cnt < MAX_DEP_PER_TASK) {
                            int id = atoi(tok);
                            if (id > 0) {
                                Task *dep = find_task_by_id(head, id);
                                if (!dep) {
                                    invalid_dep = 1;
                                    char tmp[64];
                                    snprintf(tmp, sizeof(tmp), " - %d (not found)\n", id);
                                    safe_strncat(invalid_msg, tmp, sizeof(invalid_msg));
                                } else {
                                    if (id == t->id) {
                                        invalid_dep = 1;
                                        safe_strncat(invalid_msg, " - cannot depend on itself\n", sizeof(invalid_msg));
                                    } else {
                                        t->dependencyIDs[cnt++] = id;
                                    }
                                }
                            }
                            tok = strtok(NULL, ", ");
                        }
                        t->depCount = cnt;
                        if (invalid_dep) {
                            Task *deleted = remove_task(&head, t->id);
                            if (deleted) free(deleted);
                            MessageBox(hwnd, invalid_msg, "Error: Invalid Dependencies", MB_OK | MB_ICONERROR);
                            ShowDashboard(hwnd);
                            break;
                        }
                        if (detect_cycles(head)) {
                            Task *deleted = remove_task(&head, t->id);
                            if (deleted) free(deleted);
                            MessageBox(hwnd,"Dependency cycle would be created by this task. Task not added.","Error",MB_OK|MB_ICONERROR);
                        } else {
                            Action a = { ACTION_ADD, *t }; push_undo(a); clear_redo();
                            save_tasks_to_file(head, current_user_id);
                            export_deps_dot(head, "dependencies", current_user_id);
                            MessageBox(hwnd,"Task added successfully!","Success",MB_OK);
                        }
                        ShowDashboard(hwnd);
                    } else MessageBox(hwnd,"Failed to add task.","Error",MB_OK|MB_ICONERROR);
                    break;
                }

                case ID_ADD_TASK_BACK:
                    export_deps_dot(head, "dependencies", current_user_id);
                    ShowDashboard(hwnd);
                    break;

                // --- EDIT TASK ACTIONS ---
                case ID_EDIT_TASK_BACK:
                    editingTask = NULL;
                    ShowDashboard(hwnd);
                    break;

                case ID_EDIT_TASK_SUBMIT: {
                    if (!editingTask) { ShowDashboard(hwnd); break; }

                    char title[MAX_TITLE] = {0}, desc[MAX_DESC] = {0}, cat[MAX_CATEGORY] = {0}, deadline[20] = {0}, prio_str[8] = {0}, deps_str[512] = {0};
                    GetWindowTextA(hTitle, title, MAX_TITLE); 
                    GetWindowTextA(hDesc, desc, MAX_DESC); 
                    GetWindowTextA(hCat, cat, MAX_CATEGORY); 
                    GetWindowTextA(hPriority, prio_str, 8); 
                    GetWindowTextA(hDeps, deps_str, 512);

                    SYSTEMTIME st = {0};
                    SendMessage(hDeadline, MCM_GETCURSEL, 0, (LPARAM)&st);
                    snprintf(deadline, 20, "%d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
                    int priority = atoi(prio_str); if (priority < 1 || priority > 3) priority = 2;

                    Task backup = *editingTask; // Backup in case of error

                    strncpy(editingTask->title, title, MAX_TITLE-1);
                    strncpy(editingTask->description, desc, MAX_DESC-1);
                    strncpy(editingTask->category, cat, MAX_CATEGORY-1);
                    strncpy(editingTask->deadline, deadline, 19);
                    editingTask->priority = priority;

                    int cnt = 0;
                    int newDeps[MAX_DEP_PER_TASK] = {0};
                    char *tok = strtok(deps_str, ", ");
                    int invalid = 0;
                    
                    while (tok && cnt < MAX_DEP_PER_TASK) {
                        int did = atoi(tok);
                        if (did > 0) {
                            if (did == editingTask->id) invalid = 1; 
                            else if (!find_task_by_id(head, did)) invalid = 1; 
                            else newDeps[cnt++] = did;
                        }
                        tok = strtok(NULL, ", ");
                    }

                    if (invalid) {
                        MessageBox(hwnd, "Invalid dependencies (self-ref or non-existent).", "Error", MB_OK|MB_ICONERROR);
                        *editingTask = backup;
                        break;
                    }

                    editingTask->depCount = cnt;
                    for(int i=0; i<cnt; i++) editingTask->dependencyIDs[i] = newDeps[i];

                    if (detect_cycles(head)) {
                        MessageBox(hwnd, "Cycle detected! Dependencies reverted.", "Error", MB_OK|MB_ICONERROR);
                        *editingTask = backup;
                    } else {
                        save_tasks_to_file(head, current_user_id);
                        MessageBox(hwnd, "Task updated successfully!", "Success", MB_OK);
                        editingTask = NULL;
                        ShowDashboard(hwnd);
                    }
                    break;
                }

                // --- GET ID ACTIONS (For Delete, Complete, Edit) ---
                case ID_GETID_SUBMIT: {
                    char id_str[16] = {0}; GetWindowTextA(hGetIdInput, id_str, 16); int id = atoi(id_str);
                    if (id == 0) { MessageBox(hwnd,"Invalid ID entered.","Error",MB_OK|MB_ICONERROR); break; }
                    Task *t = find_task_by_id(head, id); if (!t) { MessageBox(hwnd,"Task not found.","Error",MB_OK|MB_ICONERROR); break; }
                    
                    if (get_id_next_action == ID_DASHBOARD_COMPLETE) {
                        int can_complete = 1;
                        char dep_buf[1024] = "Cannot complete task. It depends on the following pending tasks:\n";
                        for (int i = 0; i < t->depCount; i++) {
                            Task* dep = find_task_by_id(head, t->dependencyIDs[i]);
                            if (dep && dep->status == 0) {
                                can_complete = 0;
                                char tmp[128];
                                snprintf(tmp, sizeof(tmp), " - ID %d: %s\n", dep->id, dep->title);
                                safe_strncat(dep_buf, tmp, sizeof(dep_buf));
                            }
                        }
                        if (!can_complete) {
                            MessageBox(hwnd, dep_buf, "Dependency Conflict", MB_OK | MB_ICONWARNING);
                            break;
                        }
                        Action a = { ACTION_UPDATE, *t };
                        t->status = 1;
                        push_undo(a); clear_redo();
                        save_tasks_to_file(head, current_user_id);
                        award_points(POINTS_PER_TASK, current_user_id);
                        ensure_random();
                        int qi = rand() % LOGIN_QUOTES_N;
                        MessageBoxA(hwnd, LOGIN_QUOTES[qi], "Great! Here's a motivational quote", MB_OK);
                        show_badges_if_any(hwnd);
                        MessageBox(hwnd,"Task marked as complete and points awarded.","Success",MB_OK);
                        ShowDashboard(hwnd);
                    } 
                    else if (get_id_next_action == ID_DASHBOARD_DELETE) {
                        int is_dependency = 0;
                        char dep_buf[1024] = "Cannot delete task. The following tasks depend on it:\n";
                        for (Task* cursor = head; cursor; cursor = cursor->next) {
                            if (cursor->id == t->id) continue;
                            for (int i = 0; i < cursor->depCount; i++) {
                                if (cursor->dependencyIDs[i] == t->id) {
                                    is_dependency = 1;
                                    char tmp[128];
                                    snprintf(tmp, sizeof(tmp), " - ID %d: %s\n", cursor->id, cursor->title);
                                    safe_strncat(dep_buf, tmp, sizeof(dep_buf));
                                    break; 
                                }
                            }
                        }
                        if (is_dependency) {
                            MessageBox(hwnd, dep_buf, "Dependency Conflict", MB_OK | MB_ICONWARNING);
                            break;
                        }
                        Task *deleted_task = remove_task(&head, id);
                        if (deleted_task) {
                            Action a = { ACTION_DELETE, *deleted_task };
                            push_undo(a); clear_redo();
                            free(deleted_task); 
                            save_tasks_to_file(head, current_user_id);
                            export_deps_dot(head, "dependencies", current_user_id);
                            MessageBox(hwnd,"Task deleted successfully.","Success",MB_OK);
                        } else MessageBox(hwnd,"Task not found (already deleted?).","Error",MB_OK|MB_ICONERROR);
                        ShowDashboard(hwnd);
                    }
                    else if (get_id_next_action == ID_DASHBOARD_EDIT) {
                        editingTask = t;
                        ShowEditTaskForm(hwnd);
                    }
                    break;
                }

                case ID_GETID_BACK: ShowDashboard(hwnd); break;

                // --- VIEW TASKS ACTIONS ---
                case ID_VIEW_TASKS_BACK: 
                    ShowDashboard(hwnd); 
                    break;

            }
            break; // End WM_COMMAND
        }

        case WM_PAINT: {
            PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
            SetBkMode(hdc, TRANSPARENT);
            RECT rc; GetClientRect(hwnd, &rc);
            DrawGradientBackground(hdc, rc);

            if (currentState == STATE_ANALYTICS) {
                int total=0, completed=0, pending=0, overdue=0, ph=0, pm=0, pl=0, wait=0, cycles=0;
                int weekly_total[12][5], weekly_completed[12][5];
                int monthly_total[12], monthly_completed[12];
                ComputeAnalytics(head, &total, &completed, &pending, &overdue, &ph, &pm, &pl, &wait, &cycles, weekly_total, weekly_completed, monthly_total, monthly_completed);

                char buf[256];
                SetBkMode(hdc, TRANSPARENT);

                // ---------- 1. Dependency Graph ----------
                RECT graphArea = { 30, 40, 300, 220 };
                draw_dependency_graph(hdc, graphArea, head);

                // ---------- 2. Stats (Top-right) ----------
                int sx = 340, sy = 50;
                snprintf(buf,sizeof(buf),"Total Tasks: %d", total); TextOutA(hdc, sx, sy, buf, strlen(buf)); sy += 18;
                snprintf(buf,sizeof(buf),"Completed: %d", completed); TextOutA(hdc, sx, sy, buf, strlen(buf)); sy += 18;
                snprintf(buf,sizeof(buf),"Pending: %d", pending); TextOutA(hdc, sx, sy, buf, strlen(buf)); sy += 18;
                snprintf(buf,sizeof(buf),"Overdue: %d", overdue); TextOutA(hdc, sx, sy, buf, strlen(buf)); sy += 18;
                snprintf(buf,sizeof(buf),"Waiting on dependencies: %d", wait); TextOutA(hdc, sx, sy, buf, strlen(buf)); sy += 18;
                snprintf(buf,sizeof(buf),"Cycle detected: %s", cycles ? "YES" : "NO"); TextOutA(hdc, sx, sy, buf, strlen(buf));

                // ---------- 3. Priority Bars ----------
                int barX = 340, barY = 180; int barW = 180;
                int maxVal = max(ph, max(pm, pl)); if (maxVal == 0) maxVal = 1;

                Rectangle(hdc, barX, barY, barX + barW, barY + 18);
                HBRUSH hb = CreateSolidBrush(RGB(230,80,80));
                RECT r = { barX, barY, barX + (ph * barW) / maxVal, barY + 18 };
                FillRect(hdc, &r, hb); DeleteObject(hb);
                TextOutA(hdc, barX + barW + 8, barY + 2, "High", 4);

                barY += 28;
                Rectangle(hdc, barX, barY, barX + barW, barY + 18);
                hb = CreateSolidBrush(RGB(60,150,230));
                r.left = barX; r.right = barX + (pm * barW) / maxVal;
                r.top = barY; r.bottom = barY + 18;
                FillRect(hdc, &r, hb); DeleteObject(hb);
                TextOutA(hdc, barX + barW + 8, barY + 2, "Medium", 6);

                barY += 28;
                Rectangle(hdc, barX, barY, barX + barW, barY + 18);
                hb = CreateSolidBrush(RGB(100,200,100));
                r.left = barX; r.right = barX + (pl * barW) / maxVal;
                r.top = barY; r.bottom = barY + 18;
                FillRect(hdc, &r, hb); DeleteObject(hb);
                TextOutA(hdc, barX + barW + 8, barY + 2, "Low", 3);

                // ---------- 4. Weekly Progress ----------
                int wx = 30, wy = 280; 
                TextOutA(hdc, wx, wy - 18, "Weekly Progress:", 16); 
                int wbarW = 40;
                for (int w=0; w<5; w++) {
                    int tot = weekly_total[0][w], done = weekly_completed[0][w];
                    int fill = (tot > 0) ? (done * 40 / tot) : 0;
                    RECT wb = { wx + w*(wbarW+12), wy - fill, wx + w*(wbarW+12) + wbarW, wy };
                    HBRUSH bw = CreateSolidBrush(RGB(100,150,230));
                    FillRect(hdc, &wb, bw); DeleteObject(bw);
                    Rectangle(hdc, wb.left, wb.top, wb.right, wb.bottom);
                    char lbl[16]; snprintf(lbl,sizeof(lbl),"W%d", w+1);
                    TextOutA(hdc, wb.left+5, wy+5, lbl, strlen(lbl));
                }

                // ---------- 5. Monthly Progress ----------
                int mx = 340, my = 280;
                TextOutA(hdc, mx, my-16, "Monthly Progress:", 17);
                int mbarW = 160;
                for (int m=0; m<12; m++) {
                    int tot = monthly_total[m], done = monthly_completed[m];
                    int fill = (tot > 0) ? (done * mbarW / tot) : 0;
                    RECT mb = { mx, my, mx + mbarW, my + 10 }; 
                    RECT mc = { mx, my, mx + fill, my + 10 }; 
                    HBRUSH hm = CreateSolidBrush(RGB(200,200,200)); FillRect(hdc, &mb, hm); DeleteObject(hm);
                    HBRUSH hc = CreateSolidBrush(RGB(100,180,120)); FillRect(hdc, &mc, hc); DeleteObject(hc);
                    Rectangle(hdc, mb.left, mb.top, mb.right, mb.bottom);
                    char lbl[16]; snprintf(lbl,sizeof(lbl),"M%02d", m+1);
                    TextOutA(hdc, mb.right+8, my, lbl, strlen(lbl));
                    my += 12; 
                }

                // ---------- 6. Productivity Bar ----------
                int productivity = (total == 0) ? 0 : (completed * 100 / total);
                int px = 30, py = 400; 
                snprintf(buf, sizeof(buf), "Productivity: %d%%", productivity);
                TextOutA(hdc, px, py - 18, buf, strlen(buf));
                
                Rectangle(hdc, px, py, px + 200, py + 18);
                hb = CreateSolidBrush(RGB(80,180,80));
                RECT pb = { px, py, px + (productivity * 2), py + 18 };
                FillRect(hdc, &pb, hb); DeleteObject(hb);
            }

            EndPaint(hwnd, &ps);
            break;
        }

        case WM_DESTROY:
            save_tasks_to_file(head, current_user_id);
            export_deps_dot(head, "dependencies", current_user_id);
            save_points(current_user_id);
            free_all_tasks(head);
            head = NULL;
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}