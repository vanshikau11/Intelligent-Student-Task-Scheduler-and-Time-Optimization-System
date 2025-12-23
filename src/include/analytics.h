#ifndef ANALYTICS_H_
#define ANALYTICS_H_

#include "main.h"

/* Points & Badges */
void load_points(const char* userID);
void save_points(const char* userID);
void award_points(int pts, const char* userID);
void show_badges_if_any(HWND hwnd);

/* Analytics Computation */
void ComputeAnalytics(Task *head_ptr,
                      int *total, int *completed, int *pending, int *overdue,
                      int *prioHigh, int *prioMed, int *prioLow,
                      int *waitingOnDeps, int *cyclesDetected,
                      int weekly_total[12][5], int weekly_completed[12][5],
                      int monthly_total[12], int monthly_completed[12]);

/* Summary */
void save_weekly_summary(Task *head_ptr, const char* userID);

/* Drawing */
void draw_dependency_graph(HDC hdc, RECT area, Task *head_ptr);


#endif // ANALYTICS_H_