#define _POSIX_C_SOURCE 200809L
/*
 * FTWEBRC v1.1.0 — God of Bug Bounty Enumeration
 * Made by BLOODRAVENIST · For Authorized Testing Only
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>

#define VERSION "1.1.0"
#define MAX_TOOLS 512
#define MAX_LINE 4096
#define MAX_PATH 2048
#define MAX_CMD 8192

#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[1;33m"
#define CYAN    "\033[0;36m"
#define NC      "\033[0m"

typedef struct {
    char name[64];
    char command[1024];
    int timeout;
    char phase[64];
} Tool;

static Tool g_tools[MAX_TOOLS];
static int g_tool_count = 0;

/* Safe string copy */
static void safe_copy(char *dst, const char *src, size_t size) {
    if (size == 0) return;
    size_t n = strlen(src);
    if (n >= size) n = size - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

/* Create directories recursively */
static int mkdir_p(const char *path) {
    char tmp[MAX_PATH];
    safe_copy(tmp, path, sizeof(tmp));
    size_t len = strlen(tmp);
    if (len == 0) return 0;
    if (tmp[len - 1] == '/') tmp[len - 1] = '\0';

    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) return -1;
            *p = '/';
        }
    }
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) return -1;
    return 0;
}

/* Check if tool is installed */
static int tool_exists(const char *cmd) {
    /* Extract first word */
    char binary[128];
    int i = 0;
    while (*cmd && *cmd != ' ' && *cmd != '|' && *cmd != '<' && i < 127) {
        binary[i++] = *cmd++;
    }
    binary[i] = '\0';

    if (i == 0) return 0;
    /* Skip known prefixes */
    if (strcmp(binary, "python3") == 0 ||
        strcmp(binary, "python") == 0 ||
        strcmp(binary, "cat") == 0 ||
        strcmp(binary, "echo") == 0 ||
        strcmp(binary, "curl") == 0 ||
        strcmp(binary, "wget") == 0) {
        return 1;
    }

    char check[256];
    snprintf(check, sizeof(check), "command -v %s >/dev/null 2>&1", binary);
    return system(check) == 0;
}

/* Run shell command, save output */
static int run_cmd(const char *cmd, const char *outfile, int timeout) {
    char full[MAX_CMD];
    int n = snprintf(full, sizeof(full),
        "timeout %ds bash -c %s > %s 2>&1 </dev/null",
        timeout, cmd, outfile);
    if (n < 0 || (size_t)n >= sizeof(full)) {
        return -2;
    }

    int status = system(full);
    if (status == -1) return -1;
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) return 0;
    if (WIFEXITED(status) && WEXITSTATUS(status) == 124) return 124;
    return WEXITSTATUS(status);
}

/* Load tools from config */
static int load_tools(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, RED "Cannot open %s\n" NC, path);
        return -1;
    }

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f) && g_tool_count < MAX_TOOLS) {
        if (line[0] == '#' || line[0] == '\n') continue;
        line[strcspn(line, "\r\n")] = '\0';

        char *save = NULL;
        char *name = strtok_r(line, "|", &save);
        char *command = strtok_r(NULL, "|", &save);
        char *timeout_str = strtok_r(NULL, "|", &save);
        char *phase = strtok_r(NULL, "|", &save);

        if (!name || !command || !timeout_str || !phase) continue;

        safe_copy(g_tools[g_tool_count].name, name,
                  sizeof(g_tools[g_tool_count].name));
        safe_copy(g_tools[g_tool_count].command, command,
                  sizeof(g_tools[g_tool_count].command));
        g_tools[g_tool_count].timeout = atoi(timeout_str);
        safe_copy(g_tools[g_tool_count].phase, phase,
                  sizeof(g_tools[g_tool_count].phase));
        g_tool_count++;
    }
    fclose(f);
    return 0;
}

/* Replace placeholder in command */
static void replace_all(char *str, size_t size, const char *find, const char *replace) {
    char tmp[MAX_CMD];
    char *p;
    size_t find_len = strlen(find);
    if (find_len == 0) return;

    while ((p = strstr(str, find)) != NULL) {
        size_t prefix_len = p - str;
        size_t replace_len = strlen(replace);
        size_t suffix_len = strlen(p + find_len);

        if (prefix_len + replace_len + suffix_len + 1 > size) break;

        memcpy(tmp, str, prefix_len);
        memcpy(tmp + prefix_len, replace, replace_len);
        memcpy(tmp + prefix_len + replace_len, p + find_len, suffix_len);
        tmp[prefix_len + replace_len + suffix_len] = '\0';
        safe_copy(str, tmp, size);
    }
}

static void build_command(char *out, size_t out_size, const char *template,
                          const char *url, const char *domain,
                          const char *subdomains_file, const char *urls_file,
                          const char *outdir) {
    safe_copy(out, template, out_size);
    replace_all(out, out_size, "<url>", url);
    replace_all(out, out_size, "<domain>", domain);
    replace_all(out, out_size, "<subdomains_file>", subdomains_file);
    replace_all(out, out_size, "<urls_file>", urls_file);
    replace_all(out, out_size, "<outdir>", outdir);
}

static void normalize_url(const char *input, char *url, size_t url_size,
                          char *domain, size_t domain_size) {
    if (strncmp(input, "http://", 7) == 0 || strncmp(input, "https://", 8) == 0) {
        safe_copy(url, input, url_size);
    } else {
        snprintf(url, url_size, "https://%s", input);
    }

    char *start = strstr(url, "://");
    if (start) start += 3;
    else start = url;

    char *end = start;
    while (*end && *end != '/' && *end != ':') end++;

    size_t len = end - start;
    if (len >= domain_size) len = domain_size - 1;
    memcpy(domain, start, len);
    domain[len] = '\0';
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <URL or domain>\n", argv[0]);
        printf("Example: %s example.com\n", argv[0]);
        return 1;
    }

    printf(CYAN
        "\n"
        "╔═══════════════════════════════════════════════════════════════╗\n"
        "║          FTWEBRC — God of Bug Bounty Enumeration              ║\n"
        "║          Made by BLOODRAVENIST                                ║\n"
        "║          v%s                                           ║\n"
        "╚═══════════════════════════════════════════════════════════════╝\n"
        NC "\n", VERSION);

    char url[1024], domain[256];
    normalize_url(argv[1], url, sizeof(url), domain, sizeof(domain));

    printf(CYAN "Target:" NC " %s\n", url);
    printf(CYAN "Domain:" NC " %s\n\n", domain);

    char timestamp[64];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", tm_info);

    mkdir_p("results");
    char outdir[512];
    snprintf(outdir, sizeof(outdir), "results/%s_%s", domain, timestamp);
    mkdir_p(outdir);

    char rawdir[MAX_PATH], subdomains_file[MAX_PATH], urls_file[MAX_PATH];
    snprintf(rawdir, sizeof(rawdir), "%s/raw", outdir);
    snprintf(subdomains_file, sizeof(subdomains_file), "%s/subdomains.txt", outdir);
    snprintf(urls_file, sizeof(urls_file), "%s/urls.txt", outdir);
    mkdir_p(rawdir);

    FILE *sf = fopen(subdomains_file, "w");
    if (sf) { fprintf(sf, "%s\n", domain); fclose(sf); }
    FILE *uf = fopen(urls_file, "w");
    if (uf) { fprintf(uf, "%s\n", url); fclose(uf); }

    if (load_tools("config/tools.txt") != 0) {
        fprintf(stderr, RED "Failed to load tools\n" NC);
        return 1;
    }

    /* Check which tools are installed */
    int available = 0;
    int missing = 0;
    for (int i = 0; i < g_tool_count; i++) {
        if (tool_exists(g_tools[i].command)) available++;
        else missing++;
    }

    printf(GREEN "Loaded %d tools" NC " (%d available, %d missing)\n",
           g_tool_count, available, missing);
    printf(CYAN "Output: %s\n\n" NC, outdir);

    const char *phases[] = {
        "subdomain", "dns", "probe", "tech", "urls", "crawl",
        "content", "params", "screenshot", "ssl", "vuln",
        "xss", "sqli", "cmdi", "cors", "crlf", "redirect",
        "takeover", "exposure", "misconfig", "cves",
        "nuclei", "gf", NULL
    };

    int total = 0, success = 0, failed = 0, skipped = 0;

    for (int p = 0; phases[p]; p++) {
        int phase_has_tools = 0;
        for (int i = 0; i < g_tool_count; i++) {
            if (strcmp(g_tools[i].phase, phases[p]) == 0) {
                phase_has_tools = 1;
                break;
            }
        }
        if (!phase_has_tools) continue;

        printf(YELLOW "\n═══ %s ═══\n" NC, phases[p]);
        fflush(stdout);

        for (int i = 0; i < g_tool_count; i++) {
            if (strcmp(g_tools[i].phase, phases[p]) != 0) continue;

            /* Skip if not installed */
            if (!tool_exists(g_tools[i].command)) {
                printf(CYAN "  → %s" NC " " YELLOW "⊘ not installed\n" NC,
                       g_tools[i].name);
                skipped++;
                continue;
            }

            char cmd[MAX_CMD];
            build_command(cmd, sizeof(cmd), g_tools[i].command,
                          url, domain, subdomains_file, urls_file, outdir);

            char outfile[MAX_PATH];
            snprintf(outfile, sizeof(outfile), "%s/%s.txt", rawdir, g_tools[i].name);

            printf(CYAN "  → %s" NC " ", g_tools[i].name);
            fflush(stdout);

            int result = run_cmd(cmd, outfile, g_tools[i].timeout);
            total++;

            if (result == 0) {
                printf(GREEN "✓\n" NC);
                success++;
            } else if (result == 124) {
                printf(YELLOW "⏱ timeout\n" NC);
                failed++;
            } else if (result == -2) {
                printf(RED "✗ cmd too long\n" NC);
                failed++;
            } else {
                printf(RED "✗ (exit %d)\n" NC, result);
                failed++;
            }
            fflush(stdout);
            sleep(2);
        }
    }

    printf("\n" GREEN "═══════════════════════════════════════════════════════════════\n" NC);
    printf(GREEN "✓ Complete\n" NC);
    printf(GREEN "═══════════════════════════════════════════════════════════════\n" NC);
    printf("Tools:   %d run, %d succeeded, %d failed, %d skipped\n",
           total, success, failed, skipped);
    printf("Output:  %s\n", outdir);
    printf("\n");

    return 0;
}
