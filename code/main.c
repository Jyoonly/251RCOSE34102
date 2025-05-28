#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MAX_PROCESSES 100

// -----------------------------
// 구조체 정의
// -----------------------------
typedef struct { 
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int priority;
    int start_time;
    int end_time;
    int waiting_time;
    int turnaround_time;
    // I/O 관련은 추후 추가
} Process;


// -----------------------------
// 비교 함수 타입 정의
// -----------------------------
typedef int (*CompareFunc)(Process, Process);


// -----------------------------
// 정렬 함수 및 비교 함수들
// -----------------------------
void sort_processes(Process arr[], int n, CompareFunc cmp) {
    for (int i = 0; i < n-1; i++) {
        for (int j = i+1; j < n; j++) {
            if (cmp(arr[i], arr[j]) > 0) {
                Process tmp = arr[i];
                arr[i] = arr[j];
                arr[j] = tmp;
            }
        }
    }
}

int compare_by_arrival(Process a, Process b) {
    return a.arrival_time - b.arrival_time;
}

int compare_by_burst(Process a, Process b) {
    return a.burst_time - b.burst_time;
}

int compare_by_remaining(Process a, Process b) {
    return a.remaining_time - b.remaining_time;
}

int compare_by_priority(Process a, Process b) {
    return a.priority - b.priority;
}
// 함수 선언
void create_processes(Process processes[], int num_processes);
void config(Process processes[], int num_processes);
// void fcfs(Process processes[], int num_processes);
// void calculate_metrics(Process processes[], int num_processes);
// void print_gantt_chart(Process processes[], int num_processes);


// -----------------------------
// 프로세스 생성
// -----------------------------
void create_processes(Process processes[], int num_processes) {
    srand(time(NULL)); // 난수 초기화

    for (int i = 0; i < num_processes; i++) {
        processes[i].pid = i + 1;
        // 랜덤한 arrival_time,burst_time, priority 생성
        processes[i].arrival_time = rand() % 20; // 0~9 사이
        processes[i].burst_time = rand() % 10 + 1; // 1~10 사이
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].priority = rand() % num_processes + 1; // 1부터 num_processes 사이의 우선순위

        // 나머지는 초기화
        processes[i].start_time = -1;
        processes[i].end_time = -1;
        processes[i].waiting_time = 0;
        processes[i].turnaround_time = 0;
    }

    printf("\n[created processes information]\n");
    printf("PID | Arrival | Burst | Priority\n");
    for (int i = 0; i < num_processes; i++) {
        printf("%3d | %7d | %5d | %8d\n",
            processes[i].pid,
            processes[i].arrival_time,
            processes[i].burst_time,
            processes[i].priority);
    }
}

// -----------------------------
// 프로세스 생성 (테스트용. 직접입력)
// -----------------------------
void create_test_processes(Process processes[], int *num_processes) {
    *num_processes = 3;

    processes[0].pid = 1;
    processes[0].arrival_time = 0;
    processes[0].burst_time = 3;

    processes[1].pid = 2;
    processes[1].arrival_time = 5;
    processes[1].burst_time = 6;

    processes[2].pid = 3;
    processes[2].arrival_time = 6;
    processes[2].burst_time = 4;

    for (int i = 0; i < *num_processes; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].priority = i + 1; // 임의의 값
        processes[i].start_time = -1;
        processes[i].end_time = -1;
        processes[i].waiting_time = 0;
        processes[i].turnaround_time = 0;
    }
    printf("\n[created processes information]\n");
    printf("PID | Arrival | Burst | Priority\n");
    for (int i = 0; i < *num_processes; i++) {
        printf("%3d | %7d | %5d | %8d\n",
            processes[i].pid,
            processes[i].arrival_time,
            processes[i].burst_time,
            processes[i].priority);
    }
}


// config 함수
void config(Process processes[], int num_processes) {
    // 간단한 버블 정렬 (arrival_time 기준)
    for (int i = 0; i < num_processes - 1; i++) {
        for (int j = 0; j < num_processes - i - 1; j++) {
            if (processes[j].arrival_time > processes[j + 1].arrival_time) {
                // swap
                Process temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }
    // 출력부분. 디버깅용으로 둠. 추후 삭제
    printf("\n[Ready Queue Sorted by Arrival Time]\n");
    printf("PID | Arrival | Burst | Priority\n");
    for (int i = 0; i < num_processes; i++) {
        printf("%3d | %7d | %5d | %8d\n",
            processes[i].pid,
            processes[i].arrival_time,
            processes[i].burst_time,
            processes[i].priority);
    }
}

// -----------------------------
// Schedule 함수들
// -----------------------------

// -----------------------------
// 1. FCFS 
// -----------------------------
void schedule_fcfs(Process processes[], int num_processes) {
    // arrival_time 기준으로 정렬
    sort_processes(processes, num_processes, compare_by_arrival);
    
    int current_time = 0;
    for (int i = 0; i < num_processes; i++) {
        // idle 시간 처리
        if (current_time < processes[i].arrival_time) {
            current_time = processes[i].arrival_time;
        }
        processes[i].start_time = current_time; // 시작 시간
        processes[i].end_time = current_time + processes[i].burst_time; // 종료 시간
        processes[i].waiting_time = processes[i].start_time - processes[i].arrival_time; // 대기 시간
        processes[i].turnaround_time = processes[i].end_time - processes[i].arrival_time; // 반환 시간

        current_time = processes[i].end_time; // 현재 시간 업데이트
    }
}

// -----------------------------
// 2. SJF (non-preemptive)
// -----------------------------
void schedule_sjf(Process processes[], int num_processes) {
    int completed = 0;
    int current_time = 0;
    int is_completed[MAX_PROCESSES] = {0};

    while (completed < num_processes) {
        int idx = -1;
        int min_burst = 1e9;

        for (int i = 0; i < num_processes; i++) {
            if (processes[i].arrival_time <= current_time && !is_completed[i]) {
                if (processes[i].burst_time < min_burst) {
                    min_burst = processes[i].burst_time;
                    idx = i;
                } else if (processes[i].burst_time == min_burst) {
                    // 도착 시간이 빠른 쪽 우선
                    if (processes[i].arrival_time < processes[idx].arrival_time) {
                        idx = i;
                    }
                }
            }
        }

        if (idx != -1) {
            processes[idx].start_time = current_time;
            processes[idx].end_time = current_time + processes[idx].burst_time;
            processes[idx].waiting_time = current_time - processes[idx].arrival_time;
            processes[idx].turnaround_time = processes[idx].end_time - processes[idx].arrival_time;

            current_time = processes[idx].end_time;
            is_completed[idx] = 1;
            completed++;
        } else {
            current_time++; // idle
        }
    }
}
// -----------------------------
// 3. Priority (non-preemptive)
// -----------------------------
void schedule_priority(Process processes[], int num_processes) {
    int current_time = 0;
    int completed = 0;
    int is_completed[MAX_PROCESSES] = {0};

    // 최초 도착시간으로 초기화
    int earliest = 1e9;
    for (int i = 0; i < num_processes; i++) {
        if (processes[i].arrival_time < earliest)
            earliest = processes[i].arrival_time;
    }
    current_time = earliest;

    while (completed < num_processes) {
        int idx = -1;
        int best_priority = 1e9;

        for (int i = 0; i < num_processes; i++) {
            if (!is_completed[i] && processes[i].arrival_time <= current_time) {
                if (processes[i].priority < best_priority ||
                   (processes[i].priority == best_priority &&
                    processes[i].arrival_time < processes[idx].arrival_time)) {
                    best_priority = processes[i].priority;
                    idx = i;
                }
            }
        }

        if (idx != -1) {
            processes[idx].start_time = current_time;
            processes[idx].end_time = current_time + processes[idx].burst_time;
            processes[idx].waiting_time = processes[idx].start_time - processes[idx].arrival_time;
            processes[idx].turnaround_time = processes[idx].end_time - processes[idx].arrival_time;

            current_time = processes[idx].end_time;
            is_completed[idx] = 1;
            completed++;
        } else {
            current_time++; // idle
        }
    }
}


// -----------------------------
// 4. RR
// -----------------------------
void schedule_rr(Process processes[], int num_processes, int time_quantum) {
    int current_time = 0;
    int completed = 0;
    int queue[MAX_PROCESSES];
    int front = 0, rear = 0;
    int visited[MAX_PROCESSES] = {0};

    // 초기 도착 프로세스 삽입
    for (int i = 0; i < num_processes; i++) {
        if (processes[i].arrival_time == 0) {
            queue[rear++] = i;
            visited[i] = 1;
        }
    }

    while (completed < num_processes) {
        if (front == rear) {
            current_time++;
            for (int i = 0; i < num_processes; i++) {
                if (!visited[i] && processes[i].arrival_time <= current_time) {
                    queue[rear++] = i;
                    visited[i] = 1;
                }
            }
            continue;
        }

        int idx = queue[front++];

        if (processes[idx].start_time == -1) {
            processes[idx].start_time = current_time;
        }

        int exec_time = (processes[idx].remaining_time > time_quantum) ?
                        time_quantum : processes[idx].remaining_time;

        current_time += exec_time;
        processes[idx].remaining_time -= exec_time;

        // 새로 도착한 프로세스들 큐에 추가
        for (int i = 0; i < num_processes; i++) {
            if (!visited[i] && processes[i].arrival_time <= current_time) {
                queue[rear++] = i;
                visited[i] = 1;
            }
        }

        if (processes[idx].remaining_time > 0) {
            queue[rear++] = idx; // 다시 큐 뒤로
        } else {
            processes[idx].end_time = current_time;
            processes[idx].turnaround_time = processes[idx].end_time - processes[idx].arrival_time;
            processes[idx].waiting_time = processes[idx].turnaround_time - processes[idx].burst_time;
            completed++;
        }
    }
}

// -----------------------------
// Gantt Chart 출력
// -----------------------------
void print_gantt_chart(Process processes[], int num_processes) {
    printf("\n[Gantt Chart]\n|");

    int time_cursor = processes[0].start_time;

    for (int i = 0; i < num_processes; i++) {
        // idle 구간이 있다면 출력
        if (time_cursor < processes[i].start_time) {
            printf(" Idle |");
            time_cursor = processes[i].start_time;
        }

        printf("  P%d  |", processes[i].pid);
        time_cursor = processes[i].end_time;
    }

    // 시간 표시
    printf("\n%d", processes[0].start_time);
    time_cursor = processes[0].start_time;

    for (int i = 0; i < num_processes; i++) {
        if (time_cursor < processes[i].start_time) {
            printf("     %2d", processes[i].start_time); // idle 종료 시점
            time_cursor = processes[i].start_time;
        }

        printf("     %2d", processes[i].end_time);
        time_cursor = processes[i].end_time;
    }

    printf("\n");
}

void print_gantt_chart_with_idle_sorted(Process processes[], int num_processes) {
    Process sorted[MAX_PROCESSES];
    for (int i = 0; i < num_processes; i++) {
        sorted[i] = processes[i];
    }

    // start_time 기준 정렬
    for (int i = 0; i < num_processes - 1; i++) {
        for (int j = i + 1; j < num_processes; j++) {
            if (sorted[i].start_time > sorted[j].start_time) {
                Process temp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = temp;
            }
        }
    }

    printf("\n[Gantt Chart (start_time order)]\n|");
    int time_cursor = sorted[0].start_time;

    for (int i = 0; i < num_processes; i++) {
        if (time_cursor < sorted[i].start_time) {
            printf(" Idle |");
            time_cursor = sorted[i].start_time;
        }

        printf("  P%d  |", sorted[i].pid);
        time_cursor = sorted[i].end_time;
    }

    // 시간 출력
    printf("\n%d", sorted[0].start_time);
    time_cursor = sorted[0].start_time;
    for (int i = 0; i < num_processes; i++) {
        if (time_cursor < sorted[i].start_time) {
            printf("     %2d", sorted[i].start_time);
            time_cursor = sorted[i].start_time;
        }
        printf("     %2d", sorted[i].end_time);
        time_cursor = sorted[i].end_time;
    }
    printf("\n");
}


// -----------------------------
// Evaluate 함수
// -----------------------------
void evaluation(Process processes[], int num_processes) {
    int total_waiting_time = 0;
    int total_turnaround_time = 0;

    for (int i = 0; i < num_processes; i++) {
        total_waiting_time += processes[i].waiting_time;
        total_turnaround_time += processes[i].turnaround_time;
    }

    double avg_waiting_time = (double) total_waiting_time / num_processes;
    double avg_turnaround_time = (double) total_turnaround_time / num_processes;

    printf("\n[Evaluation Result]\n");
    printf("Average Waiting Time     : %.2f\n", avg_waiting_time);
    printf("Average Turnaround Time  : %.2f\n", avg_turnaround_time);
}

void print_process_table(Process processes[], int num_processes) {
    printf("\n[Process Status Table]\n");
    printf("PID | Arrival | Burst | Start | End  | Waiting | Turnaround | Priority\n");
    printf("----+---------+-------+-------+------+---------+------------+---------\n");

    for (int i = 0; i < num_processes; i++) {
        printf("%3d | %7d | %5d | %5d | %4d | %7d | %10d | %8d\n",
            processes[i].pid,
            processes[i].arrival_time,
            processes[i].burst_time,
            processes[i].start_time,
            processes[i].end_time,
            processes[i].waiting_time,
            processes[i].turnaround_time,
            processes[i].priority);
    }
}

// -----------------------------
// 메인 함수
// -----------------------------
int main() {
    Process processes[MAX_PROCESSES];
    int num_processes;

    printf("Number of processes?: ");
    scanf("%d", &num_processes);

    // Process 생성
    create_processes(processes, num_processes);
    //create_test_processes(processes, &num_processes);
    //config(processes, num_processes);

    // // FCFS 스케줄링
    // schedule_fcfs(processes, num_processes);
    // print_gantt_chart(processes, num_processes);
    // print_gantt_chart_with_idle_sorted(processes, num_processes);
    // print_process_table(processes, num_processes);
    // evaluation(processes, num_processes);

    // // SJF 스케줄링(non-preemptive)
    // schedule_sjf(processes, num_processes);
    // print_gantt_chart(processes, num_processes);
    // print_gantt_chart_with_idle_sorted(processes, num_processes);
    // print_process_table(processes, num_processes);
    // evaluation(processes, num_processes);

    // // Priority 스케줄링(non-preemptive)
    // schedule_priority(processes, num_processes);
    // print_gantt_chart(processes, num_processes);
    // print_gantt_chart_with_idle_sorted(processes, num_processes);
    // print_process_table(processes, num_processes);
    // evaluation(processes, num_processes);

    // RR 스케줄링
    schedule_rr(processes, num_processes, 2); // Time Quantum = 2
    print_gantt_chart(processes, num_processes);
    print_gantt_chart_with_idle_sorted(processes, num_processes);
    print_process_table(processes, num_processes);
    evaluation(processes, num_processes);
    return 0;
}


// 모든 알고리즘을 반복시행하고, 간트차트를 그리고, 한번에 evaluate 하는 함수를 하나 제작하는게 나으려나...? 흠