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
        processes[i].arrival_time = rand() % 50; // 0~9 사이
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
// Gantt Chart 출력
// -----------------------------
void print_gantt_chart(Process processes[], int num_processes) {
    printf("\n[Gantt Chart]\n|");
    for (int i = 0; i < num_processes; i++) {
        printf("  P%d  |", processes[i].pid);
    }
    printf("\n");

    printf("%d", processes[0].start_time);
    for (int i = 0; i < num_processes; i++) {
        printf("     %2d", processes[i].end_time);
    }
    printf("\n");
}
void print_gantt_chart_with_idle(Process processes[], int num_processes) {
    printf("\n[Gantt Chart with Idle Time]\n|");

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

    // FCFS 스케줄링
    schedule_fcfs(processes, num_processes);
    print_gantt_chart(processes, num_processes);
    print_gantt_chart_with_idle(processes, num_processes);
    evaluation(processes, num_processes);
    return 0;
}
