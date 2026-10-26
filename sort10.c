/*
 * 과제-10: 정렬 알고리즘의 성능 및 안정성 비교
 *  - 선택 정렬, 버블 정렬, 버블 정렬(조기 종료), 삽입 정렬(> / >=)
 *  - 첫 번째 키(key1) 기준 오름차순
 *  - 비교 횟수: 두 데이터의 key1을 비교할 때마다 1회 (반복문 조건은 제외)
 *  - 이동 횟수: 데이터 1개 전체를 복사/대입할 때마다 1회 (교환 = 3회)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N          10000      /* 데이터 개수 */
#define TRIALS     100        /* 무작위 반복 횟수 */
#define MAXKEY     1000000    /* 키 최대값 */
#define PARTIAL_SWAPS 500     /* 부분 정렬 데이터: 무작위 교환 횟수 (N의 5%) */

typedef struct {
    int key1;     /* 정렬 기준 */
    int key2;     /* 정렬 기준 아님 */
    int order;    /* 원래 입력 순서 (안정성 검증용, 정렬 기준 아님) */
} Data;

/* 측정 카운터 */
static long long g_cmp = 0;
static long long g_mov = 0;

/* 두 데이터 교환: temp 사용 -> 이동 3회 */
static void swap_data(Data *a, Data *b)
{
    Data temp = *a;  g_mov++;
    *a = *b;         g_mov++;
    *b = temp;       g_mov++;
}

/* ---------------- 정렬 알고리즘 ---------------- */

static void selection_sort(Data a[], int n)
{
    for (int i = 0; i < n - 1; i++) {
        int min = i;
        for (int j = i + 1; j < n; j++) {
            g_cmp++;
            if (a[j].key1 < a[min].key1)
                min = j;
        }
        if (min != i)                 /* 같은 위치면 교환하지 않음 */
            swap_data(&a[i], &a[min]);
    }
}

/* 일반 버블 정렬: 정해진 n-1번 순회를 모두 수행 */
static void bubble_sort(Data a[], int n)
{
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            g_cmp++;
            if (a[j].key1 > a[j + 1].key1)
                swap_data(&a[j], &a[j + 1]);
        }
    }
}

/* 조기 종료 버블 정렬: 한 순회에서 교환이 없으면 종료 */
static void bubble_sort_early(Data a[], int n)
{
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - i; j++) {
            g_cmp++;
            if (a[j].key1 > a[j + 1].key1) {
                swap_data(&a[j], &a[j + 1]);
                swapped = 1;
            }
        }
        if (!swapped)
            break;
    }
}

/* 삽입 정렬: allow_equal == 0 이면 Case A (>), 1 이면 Case B (>=) */
static void insertion_sort(Data a[], int n, int allow_equal)
{
    for (int i = 1; i < n; i++) {
        Data current = a[i];  g_mov++;          /* 임시 변수에 저장 */
        int j = i - 1;
        while (j >= 0) {
            int shift;
            g_cmp++;
            if (allow_equal)
                shift = (a[j].key1 >= current.key1);
            else
                shift = (a[j].key1 >  current.key1);
            if (!shift)
                break;
            a[j + 1] = a[j];  g_mov++;           /* 한 칸 이동 */
            j--;
        }
        a[j + 1] = current;  g_mov++;            /* 최종 위치에 대입 */
    }
}

/* ---------------- 실험 도우미 ---------------- */

enum { SEL, BUB, BUBE, INS_GT, INS_GE, KINDS };
static const char *NAMES[KINDS] = {
    "Selection Sort",
    "Bubble Sort",
    "Bubble Sort (Early Termination)",
    "Insertion Sort",
    "Insertion Sort (>=)"
};

static void run_kind(int kind, Data a[], int n)
{
    switch (kind) {
    case SEL:    selection_sort(a, n);      break;
    case BUB:    bubble_sort(a, n);         break;
    case BUBE:   bubble_sort_early(a, n);   break;
    case INS_GT: insertion_sort(a, n, 0);   break;
    case INS_GE: insertion_sort(a, n, 1);   break;
    }
}

static int is_sorted_by_key1(const Data a[], int n)
{
    for (int i = 0; i < n - 1; i++)
        if (a[i].key1 > a[i + 1].key1)
            return 0;
    return 1;
}

/* 원본을 복사해서 정렬하고 비교/이동 횟수를 돌려준다. 복사는 횟수에 포함하지 않는다. */
static void run_sort(int kind, const Data src[], Data work[], int n,
                     long long *cmp, long long *mov, int *ok)
{
    memcpy(work, src, sizeof(Data) * n);   /* 복사: 카운트 안 함 */
    g_cmp = 0;
    g_mov = 0;
    run_kind(kind, work, n);
    *cmp = g_cmp;
    *mov = g_mov;
    *ok = is_sorted_by_key1(work, n);
}

static void gen_random(Data a[], int n)
{
    for (int i = 0; i < n; i++) {
        a[i].key1 = rand() % (MAXKEY + 1);
        a[i].key2 = rand() % (MAXKEY + 1);
        a[i].order = i;
    }
}

/* 사전 정렬용 비교 함수 (카운트 대상 아님) */
static int cmp_asc(const void *x, const void *y)
{
    const Data *p = x, *q = y;
    if (p->key1 != q->key1) return (p->key1 > q->key1) - (p->key1 < q->key1);
    return p->order - q->order;
}
static int cmp_desc(const void *x, const void *y)
{
    const Data *p = x, *q = y;
    if (p->key1 != q->key1) return (p->key1 < q->key1) - (p->key1 > q->key1);
    return p->order - q->order;
}

static void print_state(const char *title, const Data src[], Data work[], int n)
{
    printf("%s\n\n", title);
    for (int k = 0; k < KINDS; k++) {
        long long c, m;
        int ok;
        run_sort(k, src, work, n, &c, &m, &ok);
        printf("%s\n", NAMES[k]);
        printf("Comparisons : %lld\n", c);
        printf("Movements   : %lld\n", m);
        printf("Sorted OK   : %s\n\n", ok ? "Yes" : "NO!");
    }
}

/* ---------------- 안정성 검증 ---------------- */

#define STAB_N 1000

static int count_stability_violations(const Data a[], int n)
{
    int v = 0;
    for (int i = 0; i < n - 1; i++)
        if (a[i].key1 == a[i + 1].key1 && a[i].order > a[i + 1].order)
            v++;
    return v;
}

static void stability_verification(void)
{
    static Data base[STAB_N], work[STAB_N];

    /* (1) 작은 예시: 과제 본문의 4개 데이터 */
    Data small[4] = {
        {527, 100, 0}, {527, 200, 1}, {125, 300, 2}, {892, 400, 3}
    };
    Data sw[4];
    printf("Stability Verification\n\n");
    printf("[1] Small example (Index 0..3)\n");
    printf("Original            : (527,100) (527,200) (125,300) (892,400)\n");
    for (int k = 0; k < KINDS; k++) {
        memcpy(sw, small, sizeof(small));
        run_kind(k, sw, 4);
        printf("%-20s:", NAMES[k]);
        for (int i = 0; i < 4; i++)
            printf(" (%d,%d)", sw[i].key1, sw[i].key2);
        printf("\n");
    }

    /* (2) 중복 키가 많은 데이터 1000개: key1 은 0~49, order 에 원래 순서 기록 */
    for (int i = 0; i < STAB_N; i++) {
        base[i].key1 = rand() % 50;
        base[i].key2 = rand() % (MAXKEY + 1);
        base[i].order = i;
    }
    printf("\n[2] Duplicate-key data: %d items, key1 in 0..49, order = original index\n",
           STAB_N);
    for (int k = 0; k < KINDS; k++) {
        memcpy(work, base, sizeof(base));
        run_kind(k, work, STAB_N);
        int v = count_stability_violations(work, STAB_N);
        printf("%-20s: sorted=%s, order violations=%d -> %s\n",
               NAMES[k],
               is_sorted_by_key1(work, STAB_N) ? "Yes" : "NO!",
               v, v == 0 ? "Stable" : "Unstable");
    }
    printf("\n");
}

/* ---------------- main ---------------- */

int main(void)
{
    static Data orig[N], work[N];
    static Data base[N], sorted[N], reversed[N], partial[N];
    long long total_cmp[KINDS] = {0}, total_mov[KINDS] = {0};
    int fail = 0;

    srand(20261026);   /* 결과를 다시 재현할 수 있도록 시드 고정 */

    printf("Data Count : %d\n\n", N);

    /* 4. 무작위 데이터 100회 반복 (같은 복사본을 모든 정렬에 제공) */
    for (int t = 0; t < TRIALS; t++) {
        gen_random(orig, N);
        for (int k = 0; k < KINDS; k++) {
            long long c, m;
            int ok;
            run_sort(k, orig, work, N, &c, &m, &ok);
            total_cmp[k] += c;
            total_mov[k] += m;
            if (!ok) fail++;
        }
    }
    printf("Random Data\nNumber of Trials : %d\n", TRIALS);
    printf("Sorted check failures : %d\n\n", fail);
    for (int k = 0; k < KINDS; k++) {
        printf("%s\n", NAMES[k]);
        printf("Total Comparisons   : %lld\n", total_cmp[k]);
        printf("Average Comparisons : %.1f\n", (double)total_cmp[k] / TRIALS);
        printf("Total Movements     : %lld\n", total_mov[k]);
        printf("Average Movements   : %.1f\n\n", (double)total_mov[k] / TRIALS);
    }
    printf("------------------------------------\n\n");

    /* 7. 같은 원본에서 만든 세 가지 입력 상태 */
    gen_random(base, N);

    memcpy(sorted, base, sizeof(base));
    qsort(sorted, N, sizeof(Data), cmp_asc);          /* 사전 정렬: 카운트 제외 */

    memcpy(reversed, base, sizeof(base));
    qsort(reversed, N, sizeof(Data), cmp_desc);

    memcpy(partial, sorted, sizeof(sorted));          /* 정렬된 데이터에 무작위 교환 */
    for (int s = 0; s < PARTIAL_SWAPS; s++) {
        int i = rand() % N, j = rand() % N;
        Data tmp = partial[i];
        partial[i] = partial[j];
        partial[j] = tmp;
    }
    {
        int diff = 0;
        for (int i = 0; i < N; i++)
            if (partial[i].order != sorted[i].order) diff++;
        printf("(Partially sorted data: sorted data + %d random swaps, "
               "%d of %d positions differ from sorted)\n\n", PARTIAL_SWAPS, diff, N);
    }

    print_state("Sorted Data", sorted, work, N);
    printf("------------------------------------\n\n");
    print_state("Reverse-Sorted Data", reversed, work, N);
    printf("------------------------------------\n\n");
    print_state("Partially Sorted Data", partial, work, N);
    printf("------------------------------------\n\n");

    stability_verification();

    return 0;
}