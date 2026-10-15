#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define V 10
#define E 20
#define MAX_WEIGHT 100
#define INF 1000000

typedef struct {
    int u, v, w;
} Edge;

int weight[V][V];   /* 원래 간선 가중치 (0이면 간선 없음) */
int dist[V][V];     /* 최단거리 */
int nextV[V][V];    /* i에서 j로 갈 때 i 다음에 방문할 정점 */

/* ---------------- Union-Find (연결 확인용) ---------------- */
int findRoot(int parent[], int x) {
    while (parent[x] != x) x = parent[x];
    return x;
}

int isConnected(Edge edges[], int count) {
    int parent[V];
    for (int i = 0; i < V; i++) parent[i] = i;
    int components = V;
    for (int i = 0; i < count; i++) {
        int a = findRoot(parent, edges[i].u);
        int b = findRoot(parent, edges[i].v);
        if (a != b) {
            parent[a] = b;
            components--;
        }
    }
    return components == 1;
}

/* ---------------- 그래프 생성 ---------------- */
/* 1) 랜덤 신장 트리로 연결을 보장하고 2) 나머지 간선을 중복 없이 추가 */
void generateGraph(Edge edges[]) {
    int used[V][V];
    int perm[V];
    memset(used, 0, sizeof(used));

    for (int i = 0; i < V; i++) perm[i] = i;
    for (int i = V - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = perm[i]; perm[i] = perm[j]; perm[j] = t;
    }

    int made = 0;
    for (int i = 1; i < V; i++) {
        int a = perm[i];
        int b = perm[rand() % i];
        used[a][b] = used[b][a] = 1;
        edges[made].u = a < b ? a : b;     /* u < v 로 저장 */
        edges[made].v = a < b ? b : a;
        edges[made].w = rand() % MAX_WEIGHT + 1;
        made++;
    }

    while (made < E) {
        int a = rand() % V;
        int b = rand() % V;
        if (a == b) continue;
        if (used[a][b]) continue;
        used[a][b] = used[b][a] = 1;
        edges[made].u = a < b ? a : b;
        edges[made].v = a < b ? b : a;
        edges[made].w = rand() % MAX_WEIGHT + 1;
        made++;
    }
}

/* 출력 보기 좋게 (u, v) 오름차순 정렬 */
void sortEdges(Edge edges[]) {
    for (int i = 0; i < E - 1; i++) {
        for (int j = 0; j < E - 1 - i; j++) {
            int swap = 0;
            if (edges[j].u > edges[j + 1].u) swap = 1;
            else if (edges[j].u == edges[j + 1].u && edges[j].v > edges[j + 1].v) swap = 1;
            if (swap) {
                Edge t = edges[j];
                edges[j] = edges[j + 1];
                edges[j + 1] = t;
            }
        }
    }
}

/* ---------------- Floyd-Warshall (경로 복원 정보 포함) ---------------- */
void floydWarshall(void) {
    for (int i = 0; i < V; i++) {
        for (int j = 0; j < V; j++) {
            if (i == j) {
                dist[i][j] = 0;
                nextV[i][j] = j;
            } else if (weight[i][j] > 0) {
                dist[i][j] = weight[i][j];
                nextV[i][j] = j;
            } else {
                dist[i][j] = INF;
                nextV[i][j] = -1;
            }
        }
    }

    for (int k = 0; k < V; k++) {
        for (int i = 0; i < V; i++) {
            for (int j = 0; j < V; j++) {
                if (dist[i][k] == INF || dist[k][j] == INF) continue;
                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    nextV[i][j] = nextV[i][k];
                }
            }
        }
    }
}

/* 경로를 출력하면서, 경로에 포함된 간선 가중치의 합을 반환 */
int printPath(int u, int v) {
    int sum = 0;
    int cur = u;
    printf("%d", cur);
    while (cur != v) {
        int nxt = nextV[cur][v];
        sum += weight[cur][nxt];
        printf(" -> %d", nxt);
        cur = nxt;
    }
    printf("\n");
    return sum;
}

int main(void) {
    unsigned int seed = (unsigned int)time(NULL);
    srand(seed);

    Edge edges[E];
    do {
        generateGraph(edges);
    } while (!isConnected(edges, E));
    sortEdges(edges);

    memset(weight, 0, sizeof(weight));
    for (int i = 0; i < E; i++) {
        weight[edges[i].u][edges[i].v] = edges[i].w;
        weight[edges[i].v][edges[i].u] = edges[i].w;
    }

    /* ----- 그래프 출력 ----- */
    printf("Seed : %u\n\n", seed);
    printf("Graph\n");
    printf("Vertices : %d\n", V);
    printf("Edges    : %d\n\n", E);

    printf("Edges\n");
    for (int i = 0; i < E; i++) {
        printf("(%d, %d) Weight: %d\n", edges[i].u, edges[i].v, edges[i].w);
    }
    printf("\n");

    /* ----- 확인 사항 검증 ----- */
    int edgeCount = 0, selfLoop = 0, badWeight = 0;
    for (int i = 0; i < V; i++) {
        if (weight[i][i] != 0) selfLoop++;
        for (int j = i + 1; j < V; j++) {
            if (weight[i][j] > 0) edgeCount++;
            if (weight[i][j] != 0 && (weight[i][j] < 1 || weight[i][j] > MAX_WEIGHT)) badWeight++;
        }
    }
    printf("Check\n");
    printf("Vertex count        : %d\n", V);
    printf("Edge count          : %d\n", edgeCount);
    printf("Self-loops          : %s\n", selfLoop == 0 ? "None" : "Exist");
    printf("Duplicate edges     : %s\n", edgeCount == E ? "None" : "Exist");
    printf("Weights in 1..100   : %s\n", badWeight == 0 ? "Yes" : "No");
    printf("Connected graph     : %s\n\n", isConnected(edges, E) ? "Yes" : "No");

    /* ----- 최단경로 계산 ----- */
    floydWarshall();

    printf("Shortest Paths\n\n");
    int pairCount = 0;
    int verified = 0;

    for (int u = 0; u < V; u++) {
        for (int v = u + 1; v < V; v++) {
            printf("%d -> %d\n", u, v);
            printf("Path     : ");
            int sum = printPath(u, v);
            printf("Distance : %d\n\n", dist[u][v]);

            pairCount++;
            if (sum == dist[u][v]) verified++;
        }
    }

    printf("Total vertex pairs : %d\n", pairCount);
    printf("Distance == sum of path edge weights : %d / %d pairs\n", verified, pairCount);

    return 0;
}